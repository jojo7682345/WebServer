#pragma once
#include <stdint.h>
#include <stdio.h>

typedef unsigned char byte;
typedef size_t byte_index;
typedef uint32_t uint;




#define FILE_STRING_SIZE 255
typedef struct File {
	// file properties
	char filePath[FILE_STRING_SIZE+1];
	char fileName[FILE_STRING_SIZE+1];
	size_t fileSize;
	FILE* file;

	byte_index fileIndex;
	byte_index bufferIndex;

	byte_index allocatedSize;
	byte fileData[0];

} File;
typedef File* FileHandle;

#define FULL_FILE 0

int attachFile(const char* path, FileHandle* file, size_t maxBufferSize);
void releaseFile(FileHandle file);

size_t loadFile(FileHandle file, size_t amount);
size_t readFile(FileHandle file, size_t amount, void* buffer);



