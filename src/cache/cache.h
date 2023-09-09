#pragma once
#include "../files/fileReading.h"

#define CACHE_SIZE 4096

typedef struct FileCache_T{
    FileHandle file;
    
    void* data;
    size_t dataSize;
    Range dataRange;

    void* cachedData;
    size_t cachedSize;
}FileCache_T;
typedef FileCache_T* FileCache;

bool createFileCache(const char* filePath, FileCache* cache);

bool readFileCache(Range* range, FileCache cache);

void destroyFileCache(FileCache cache);