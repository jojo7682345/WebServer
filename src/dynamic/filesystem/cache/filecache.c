#include "filecache.h"
#include <memory.h>

size_t selectCacheSize(size_t cacheSize, size_t fileSize){
    if(fileSize <= cacheSize){
        return fileSize;
    }

    return cacheSize;
}

size_t selectDataSize(size_t cacheSize, size_t fileSize){

    if(fileSize == cacheSize){
        return cacheSize;
    }

    return cacheSize / 2 + 1;

}

bool createFileCache(const char* filePath, size_t cacheSize, uint numCaches, FileCache* fileCache) {
    FILE* file = fopen(filePath, "rb");
    if(!file){
        return 0;
    }
    fseek(file, 0, SEEK_END);
    size_t fileSize = ftell(file);
    fseek(file, 0, SEEK_SET);

    cacheSize = selectCacheSize(cacheSize, fileSize);

    void* cacheSizes = malloc(cacheSize * sizeof(size_t));
    if(cacheSizes==NULL){
        printf("out of memory\n");
        abort();
        return 0;
    }

    void* caches = malloc(cacheSize * numCaches * sizeof(byte));
    if(caches==NULL){
        printf("out of memory\n");
        abort();
        return 0;
    }

    FileCache filecache = {
        .fd=file,
        .cacheSize=cacheSize, 
        .fileSize=fileSize,
        .cacheSize=cacheSize,
        .caches = caches,
        .cacheIndex = 0,
        .cache = caches,
        .pCacheSize = cacheSizes,
        .cacheSizes = cacheSizes,
        .numCaches = numCaches,
    };

    memcpy(fileCache, &filecache, sizeof(FileCache));

    return 1;
}

size_t readFileCache(Range* range, FileCache* fileCache, byte** data) {
    if(range->end == 0){
        range->end = fileCache->fileSize;
    }
    if(range->start >= range->end){
        return 0;
    }
    if(range->start >= fileCache->fileSize){
        return 0;
    }
    if(range->end > fileCache->fileSize){
        range->end = fileCache->fileSize;
    }
    if(range->end - range->start > fileCache->cacheSize){
        range->end = range->start + fileCache->cacheSize;
    }
    
    byte* cache = fileCache->cache;
    size_t cachedSize = *fileCache->pCacheSize;
    fileCache->cacheIndex++;
    fileCache->cacheIndex %= fileCache->numCaches;
    fileCache->cache =(byte*) fileCache->caches + fileCache->cacheIndex * fileCache->cacheSize;
    fileCache->pCacheSize =(size_t*) fileCache->cacheSizes + fileCache->cacheIndex * sizeof(uint);


    *fileCache->pCacheSize = fread(fileCache->cache, range->end - range->end, 1, fileCache->fd);
    if(data){
        *data = cache;
    }
    return cachedSize;
}

void destroyFileCache(FileCache fileCache) {
    fclose(fileCache.fd);
    free((byte*)fileCache.caches);
    free((size_t*)fileCache.cacheSizes);
}

GlobalCache globalCache = {0};

uint findFile(const char* fileName){
    for(uint i = 0; i < GLOBAL_CACHE_SIZE; i++){
        if(strcmp(fileName, globalCache.nameTable[i])==0){
            return i;
        };
    }
    return (uint)-1;
}

uint findFileCacheSpot(){
    for(uint i = 0; i < GLOBAL_CACHE_SIZE; i++){
        if(globalCache.nameTable[i][0]=='\0'){
            return i;
        }
    }
    return (uint)-1;
}

uint findLeastUsedSpot(){
    uint min_time = -1;
    uint min_index = -1;
    for(uint i = 0; i < GLOBAL_CACHE_SIZE; i++){
        uint currentTime = globalCache.timeoutCounters[i];
        if(currentTime <= min_time){
            min_time = currentTime;
            min_index = i;
        }
    }
    return min_index;
}

uint clearLeastUsedSpot(){
    uint index = findLeastUsedSpot();

    FileCache* cache = &globalCache.caches[index];

    destroyFileCache(*cache);
    memset(cache, 0, sizeof(FileCache));

    globalCache.nameTable[index][0] = '\0';
    globalCache.timeoutCounters[index] = 0;

    return index;
}

uint getFileCacheSpot(){
    uint index = findFileCacheSpot();
    if(index != -1){
        return index;
    }
    
    return clearLeastUsedSpot();
}

bool cacheNewFile(const char* fileName, uint fileIndex){
    if(fileIndex >= GLOBAL_CACHE_SIZE){
        return 0;
    }
    FileCache* cache = &globalCache.caches[fileIndex];
    bool createdFileCache = createFileCache(fileName, LOCAL_CACHE_SIZE, 2, cache);
    if(!createdFileCache){
        return 0;
    }
    Range fileRange = {0,0};
    size_t readSize = readFileCache(&fileRange, cache, NULL);
    if(readSize!=0){
        printf("error in code\n");
        abort();
    }

    strcpy(globalCache.nameTable[fileIndex],fileName);
    globalCache.timeoutCounters[fileIndex] = 0;

    return 1;
}

FILE* openFile(const char* fileName){
    return fopen(fileName, "rb");
}

size_t getFileSize(const char* fileName){
    FILE* file = openFile(fileName);
    if(!file){
        return 0;
    }
    fseek(file, 0, SEEK_END);
    size_t fileSize = ftell(file);
    fseek(file, 0, SEEK_SET);
    fclose(file);
    return fileSize;
}

size_t storeFileIntoTemporaryStorage(const char* fileName, size_t fileSize){
    FILE* file = openFile(fileName);
    if(!file){
        return 0;
    }
    if(!globalCache.temporaryFileData){
        globalCache.temporaryFileData = malloc(LOCAL_CACHE_SIZE);
        if(globalCache.temporaryFileData==NULL){
            printf("out of mem\n");
            abort();
        }
    }
    size_t readSize = fread(globalCache.temporaryFileData, fileSize, 1, file);
    fclose(file);
    return readSize;
}

void copyTemporaryStoredFile(size_t storageSize, void** data, size_t* offset, size_t* size){
    *data = malloc(storageSize);
    if(*data==NULL){
        printf("out of mem\n");
        abort();
    }
    memcpy(*data, globalCache.temporaryFileData, storageSize);
    *offset = 0;
    *size = storageSize; 
}

bool fetchSmallFile(const char* fileName, size_t fileSize, void** data, size_t* offset, size_t* size){
    size_t storageSize = storeFileIntoTemporaryStorage(fileName, fileSize);
    if(storageSize == 0){
        return 0;
    }
    copyTemporaryStoredFile(storageSize, data, offset, size);
    return 1;
}

bool readCachedFile(uint index, void** data, size_t* offset, size_t* size, size_t* fileSize){

    FileCache* cache = &globalCache.caches[index];
    *fileSize = cache->fileSize;

    globalCache.timeoutCounters[index]++;

    Range range = {*offset, *offset+*size -1};
    *size = readFileCache(&range, cache, (byte**)data);
    if(*size == 0){
        return 0;
    }
    *offset = range.start;
    return 1;
}

bool handleNewFileCache(const char* fileName, void** data, size_t* offset, size_t* size, size_t* fileSize){
    // cache file
    uint index = getFileCacheSpot();

    bool fileCached = cacheNewFile(fileName, index);
    if(fileCached == 0){
        return 0;
    }

    return readCachedFile(index, data, offset, size, fileSize);
}

bool handleUncachedFile(const char* fileName, void** data, size_t* offset, size_t* size, size_t* pFileSize){
    size_t fileSize = getFileSize(fileName);

    if(fileSize <= LOCAL_CACHE_SIZE){
        *pFileSize = fileSize;
        return fetchSmallFile(fileName, fileSize, data, offset, size);
    }

    return handleNewFileCache(fileName, data, offset, size, pFileSize);    
}

bool fetchFile(const char* fileName, void** data, size_t* offset, size_t* size, size_t* fileSize) {
    if(strlen(fileName) > FILE_CACHE_NAME_LENGTH){
        printf("FilePath %s is to long for the cache\n", fileName);
        return 0;
    }

    uint fileIndex = findFile(fileName);
    
    if(fileIndex!=(uint)-1){
        return readCachedFile(fileIndex, data, offset, size, fileSize);
    }

    return handleUncachedFile(fileName,data,offset,size, fileSize);
}


