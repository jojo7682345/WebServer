#include "cache.h"
#include <stdlib.h>
#include <memory.h>


bool createFileCache(const char* filePath, FileCache* cache){
    FileHandle file = {0};
    if(!openFile(filePath, &file)){
        return 0;
    }
    *cache = malloc(sizeof(FileCache_T));
    if(*cache == NULL){
        printf("out of memory\n");
        exit(-1);
        return 0;
    }
    memset(*cache, 0, sizeof(FileCache_T));

    void* data = malloc(CACHE_SIZE);
    void* cacheData = malloc(CACHE_SIZE);
    if(data == NULL || cacheData==NULL){
        printf("out of memory\n");
        exit(-1);
        return 0;
    }

    (*cache)->file = file;
    (*cache)->data = data;
    (*cache)->cachedData = cacheData;
    (*cache)->dataRange.start = 0;
    (*cache)->dataRange.end = 0;
    (*cache)->cachedSize = 0;

    return 1;
}

bool isRequestedInRange(Range range, Range* request){

    if(request->start > range.end){
        return 0;
    }

    if(request->start < range.start){
        return 0;
    }

    if(request->end < range.start){
        return 0;
    }

    if(request->end > range.end){
        request->end = range.end;
    }

    return 1;

}

bool readFileCache(Range* range, FileCache cache){

    Range cacheRange = {
        .start = cache->dataRange.end+1,
        .end = cache->dataRange.end + cache->cachedSize
    };
    if(isRequestedInRange(cacheRange, range)){
        memcpy(cache->data, cache->cachedData, cacheRange.end - cacheRange.start + 1);
        cache->dataSize = cache->cachedSize;
        return 1;
    }

    cache->dataSize = readRange(range, cache->data, cache->file);
    if(cache->dataSize == 0){
        return 0;
    }
    return 1;
}

bool cacheFile(FileCache cache){

    Range range = {
        .start = cache->dataRange.end + 1,
        .end = cache->dataRange.end + 1 + CACHE_SIZE,
    };

    cache->cachedSize = readRange(&range, cache->cachedData, cache->file);

}

void destroyFileCache(FileCache cache){
    free(cache->data);
    free(cache->cachedData);
    closeFile(cache->file);
    free(cache);

}