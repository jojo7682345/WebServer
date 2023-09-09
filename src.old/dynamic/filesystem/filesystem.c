#include "filesystem.h"
#include <memory.h>
#include <string.h>
#include <stdlib.h>
#undef __USE_MISC
#define __USE_MISC 1
#include <dirent.h>
#include <stdarg.h>
#include <ctype.h>

#define nullptr ((void*)0)

const char* findLastOccuranceOf(const char* str, char chr){
	size_t len = strlen(str);
	for (size_t i = len - 1; i >= 0; i--) {
		if (str[i] == chr) {
			return str + i + 1;
		}
		if (i == 0) {
			return str;
		}
	}

}

const char* findFileNameStart(const char* filePath) {
	return findLastOccuranceOf(filePath, '/');
}

const char* getFileExtension(const char* fileName){
	return findLastOccuranceOf(fileName, '.');
	
}

int attachFile(const char* filePath, FileHandle* file, size_t maxBufferSize) {
	File fileHandle = { 0 };
	// set name fields
	size_t filePathLength = strlen(filePath);
	if (filePathLength >= FILE_STRING_SIZE*2) {
		printf("filepath too long\n");
		return 0;
	}
	const char* fileName = findFileNameStart(filePath);
	size_t pathLength = fileName - filePath;
	size_t nameLength = strlen(fileName);
	if (pathLength >= FILE_STRING_SIZE) {
		printf("path too long\n");
		return 0;
	}
	if (nameLength >= FILE_STRING_SIZE) {
		printf("name too long\n");
		return 0;
	}
	memcpy(fileHandle.fileName, fileName, nameLength);
	memcpy(fileHandle.filePath, filePath, pathLength);

	// open file
	fileHandle.file = fopen(filePath, "rb");
	if (!fileHandle.file) {
		printf("unable to open file\n");
		return 0;
	}

	// get file size
	fseek(fileHandle.file, 0, SEEK_END);
	fileHandle.fileSize = ftell(fileHandle.file);
	fseek(fileHandle.file, 0, SEEK_SET);

	if (maxBufferSize == 0) {
		maxBufferSize = fileHandle.fileSize;
	}

	// allocate buffer
	if (maxBufferSize > fileHandle.fileSize) {
		maxBufferSize = fileHandle.fileSize;
	}
	fileHandle.allocatedSize = maxBufferSize;

	(*file) = malloc(sizeof(File) + maxBufferSize);
	if (*file == nullptr) {
		printf("out of memory\n");
		return 0;
	}
	memset(*file, 0, sizeof(File) + maxBufferSize);
	memcpy(*file, &fileHandle, sizeof(File));
	return 1;
}

void releaseFile(FileHandle file) {
	
	if (!file->file) {
		printf("file not open\n");
		return;
	}

	// close file
	fclose(file->file);
	file->file = nullptr;

	free(file);
}

size_t loadFile(FileHandle file, size_t amount) {
	if (amount == 0) {
		amount = file->allocatedSize;
	}

	if (file->bufferIndex + amount > file->allocatedSize) {
		amount = file->allocatedSize - file->bufferIndex;
	}

	if (file->fileIndex + amount > file->fileSize) {
		amount = file->fileSize - file->fileIndex;
	}

	if (amount == 0) {
		return 0;
	}

	amount = fread(file->fileData + file->bufferIndex, 1, amount, file->file);
	
	file->bufferIndex+= amount;
	file->fileIndex += amount;
	return amount;
}

size_t readFile(FileHandle file, size_t amount, void* buffer) {
	
	memset(buffer, 0, amount);
	if (file->bufferIndex == 0) {
		return 0;
	}

	if (amount > file->bufferIndex) {
		amount = file->bufferIndex;
	}

	if (amount == 0) {
		amount = file->bufferIndex;
	}
	
	if (buffer) {
		memcpy(buffer, file->fileData, amount);
	}
	if(amount == file->allocatedSize){
		file->bufferIndex -= amount;
		return amount;
	}
	memmove(file->fileData, file->fileData + amount, file->allocatedSize - amount);

	file->bufferIndex -= amount;
	return amount;
}

#define _FILETYPE(type, str, ...) FILETYPE(type, str, __VA_ARGS__,nullptr)
#define LIST_OF_FILE_TYPES \
	_FILETYPE(TEXT, "text",		"txt")\
	_FILETYPE(IMAGE, "image", 	"png", "jpg", "jpeg", "gif")\
	_FILETYPE(VIDEO, "video", 	"mp4", "avi", "mkv")\
	_FILETYPE(AUDIO, "audio", 	"mp3", "wav", "flac")\
	_FILETYPE(PDF, "pdf", 		"pdf")\
	_FILETYPE(URL, "url", 		"url")

#define FILETYPE(type,...) type,
typedef enum FileTypes{
	LIST_OF_FILE_TYPES
} FileTypes;
#undef FILETYPE

int strcicmp(char const *a, char const *b)
{
    for (;; a++, b++) {
        int d = tolower((unsigned char)*a) - tolower((unsigned char)*b);
        if (d != 0 || !*a)
            return d;
    }
}

int checkContains(const char* str, ...){
	va_list args;
	
	int valid = 0;

	va_start(args, str);

	const char* check;
	while((check = va_arg(args, const char*))!=nullptr){
		if(strcicmp(str,check)==0){
			valid = 1;
			break;
		}
	}

	va_end(args);
	return valid;
}


#define FILETYPE(type, str, ...) if(checkContains(extension,__VA_ARGS__)){return str;}
const char* getFileTypeStr(const char* fileName){
		const char* extension = getFileExtension(fileName);
		LIST_OF_FILE_TYPES;
		return "text";
}
#undef FILETYPE

void listFilesInDirectory(const char* path, char** str, size_t* len){
	struct json_object* listedFiles = json_object_new_array();
	
	DIR* d;
	struct dirent* dir;
	d = opendir(path);

	if(d){
		while((dir = readdir(d)) != NULL){
			if(strcmp(dir->d_name,".")==0 || strcmp(dir->d_name,"..")==0){
				continue;
			}

			struct json_object* fileEntry = json_object_new_object();
			json_object_object_add(fileEntry, "name", json_object_new_string(dir->d_name));
			json_object_object_add(fileEntry, "isDirectory", json_object_new_boolean(dir->d_type == DT_DIR));
			json_object_object_add(fileEntry, "fileType", json_object_new_string(dir->d_type == DT_DIR ? "folder" : getFileTypeStr(dir->d_name)));
			json_object_array_add(listedFiles, fileEntry);
		}
		closedir(d);
	}
	const char* data = json_object_to_json_string_length(listedFiles, JSON_C_TO_STRING_PRETTY, len);
	
	*str = malloc(*len);
	if(!*str){
		printf("buy more ram\n");
		return;
	}
	memcpy(*str, data, *len);

	json_object_put(listedFiles);

}





