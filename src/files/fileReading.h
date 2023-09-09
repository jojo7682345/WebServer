#pragma once
#include <stdio.h>
#include <stdint.h>

#define MAX_FILE_FULL_PATH_LENGTH 1023
#define MAX_FILE_NAME_LENGTH 511
#define MAX_EXTENSION_LENGTH 31
#define MAX_FILE_PATH_LENGTH 255

typedef struct File {
    FILE* filePtr;
    size_t fileSize;
    char fileName[MAX_FILE_NAME_LENGTH+1];
    char fileExtension[MAX_EXTENSION_LENGTH+1];
    char filePath[MAX_FILE_PATH_LENGTH+1];
    char fileLocation[MAX_FILE_FULL_PATH_LENGTH+1];
} File;

typedef struct Range {
    size_t start;
    size_t end;
} Range;

typedef File* FileHandle;
typedef unsigned char byte;
typedef uint_fast8_t bool;
typedef uint32_t uint;

bool openFile(const char* file, FileHandle* handle);

size_t readRange(Range* range, byte* data, FileHandle handle);

void closeFile(FileHandle handle);
