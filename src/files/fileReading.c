#include "fileReading.h"
#include <stdio.h>
#include <string.h>
#include <memory.h>
#include <stdlib.h>

char* reverse_memchr(const char* str, char chr){
    size_t len = strlen(str);
	for (size_t i = len - 1; i >= 0; i--) {
		if (str[i] == chr) {
			return (char*)str + i + 1;
		}
		if (i == 0) {
			return (char*)str;
		}
	}
}

const char* getFileName(const char* filePath) {
	return reverse_memchr(filePath, '/');
}

const char* getFileExtension(const char* fileName){
	return reverse_memchr(fileName, '.');
}

bool populateFileNameData(const char* file, FileHandle handle){
    size_t fileLen = strlen(file);
    if(fileLen > MAX_FILE_FULL_PATH_LENGTH){
        return 0;
    }
    memcpy(handle->fileLocation, file, fileLen);

    const char* fileName = getFileName(file);
    size_t fileNameLen = strlen(fileName);
    if(fileNameLen > MAX_FILE_NAME_LENGTH){
        return 0;
    }
    memcpy(handle->fileName, fileName, fileNameLen);

    const char* extension = getFileExtension(file);
    size_t extensionLen = strlen(extension);
    if(extensionLen > MAX_EXTENSION_LENGTH){
        return 0;
    }
    memcpy(handle->fileExtension, extension, extensionLen);

    return 1;
}

bool populateFileData(const char* file, FileHandle handle){
    handle->filePtr = fopen(file, "rb");

    if(!handle->filePtr){
        return 0;
    }

    if(!fseek(handle->filePtr, 0, SEEK_END)){
        printf("failed to get file size");
        fclose(handle->filePtr);
        return 0;
    }

    handle->fileSize = ftell(handle->filePtr);

    if(!fseek(handle->filePtr, 0, SEEK_SET)){
        printf("failed to get file size");
        fclose(handle->filePtr);
        return 0;
    }

    return 1;
}

bool openFile(const char* file, FileHandle* handle){
    *handle = malloc(sizeof(File));
    if(*handle){
        printf("out of memory\n");
        exit(-1);
    }
    memset(*handle, 0, sizeof(File));
    
    if(!populateFileNameData(file, *handle)){
        printf("invalid file name/path\n");
        return 0;
    }

    if(!populateFileData(file, *handle)){
        return 0;
    }

    return 1;
}

size_t readRange(Range* range, byte* data, FileHandle handle){

    if(range->end == 0){
        range->end = handle->fileSize -1;
    }

    if(range->start > range->end){
        range->start = 0;
        range->end = 0; 
        return 0;
    }

    if(range->start >= handle->fileSize){
        range->start = 0;
        range->end = 0; 
        return 0;
    }

    if(range->end >= handle->fileSize){
        range->end = handle->fileSize-1;
    }

    if(!fseek(handle->filePtr, range->start, SEEK_SET)){
        printf("failed to get range");
        fclose(handle->filePtr);
        range->start = 0;
        range->end = 0; 
        return 0;
    }

    size_t readSize = fread(data, range->end - range->start + 1, 1, handle->filePtr);

    range->end = range->start + readSize - 1;

    return readSize;
}

void closeFile(FileHandle handle){
    if(handle->filePtr){
        fclose(handle->filePtr);
    }
    free(handle);
}