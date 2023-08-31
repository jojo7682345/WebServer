#include "fileLoading.h"
#include <memory.h>
#include <string.h>
#include <stdlib.h>

#define nullptr ((void*)0)

const char* findFileNameStart(const char* filePath) {
	size_t len = strlen(filePath);
	for (size_t i = len - 1; i >= 0; i--) {
		if (filePath[i] == '/') {
			return filePath + i + 1;
		}
		if (i == 0) {
			return filePath;
		}
	}
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

	memmove(file->fileData, file->fileData + amount, amount);

	file->bufferIndex -= amount;
	return amount;
}


