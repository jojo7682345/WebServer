#include "../filesystem.h"

typedef int bool;
#define true 1
#define false 0

typedef struct Range {
    size_t start;
    size_t end;
} Range;

typedef struct FileCache {
    FILE* fd;
    const size_t fileSize;
    const size_t cacheSize;

    byte* cache;
    size_t* pCacheSize;

    const size_t* cacheSizes;
    const byte* caches;
    const uint numCaches;
    uint cacheIndex;
} FileCache;

bool createFileCache(const char* filePath, size_t cacheSize, uint numCaches, FileCache* fileCache);

size_t readFileCache(Range* range, FileCache* fileCache, byte** data);

void destroyFileCache(FileCache fileCache);

#define FILE_CACHE_NAME_LENGTH 1024
#define GLOBAL_CACHE_SIZE 10
#define LOCAL_CACHE_SIZE 1024

typedef struct GlobalCache {
    char nameTable[GLOBAL_CACHE_SIZE][FILE_CACHE_NAME_LENGTH];
    uint timeoutCounters[GLOBAL_CACHE_SIZE];
    FileCache caches[GLOBAL_CACHE_SIZE];
    byte* temporaryFileData;
} GlobalCache;

extern GlobalCache globalCache;

int fetchFile(const char* file, void** data, size_t* offset, size_t* size, size_t* fileSize);
