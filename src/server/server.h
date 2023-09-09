#pragma once
#include <stdint.h>
#include <stdint.h>
#include <cache/cache.h>

#define LIST_OF_HANDLERS \
    HANDLER(GetRequestHandler)\
    HANDLER(PostRequestHandler)

#define HANDLER(name) typedef struct name##_T* name; 
LIST_OF_HANDLERS;
#undef HANDLER;

typedef void* RequestHandler;

typedef struct ConnectionHandlerCreateInfo {
    GetRequestHandler getHandler;
} ConnectionHandlerCreateInfo;

typedef struct GetRequestHandlerCreateInfo {
    void;
} GetRequestHandlerCreateInfo;

typedef struct PostRequestHandlerCreateInfo{

}PostRequestHandlerCreateInfo;

#define HANDLER(name)\
void create##name(name* handle); \
void destroy##name(name handle);
LIST_OF_HANDLERS;
#undef HANDLER

typedef struct ConnectionHandler_T* ConnectionHandler;
void createConnectionHandler(ConnectionHandlerCreateInfo info, ConnectionHandler* handle); 
void destroyConnectionHandler(ConnectionHandler handle);

typedef struct OptionList_T* OptionList;
bool optionListContains(const char* option);
const char* optionListGetValue(const char* option);


typedef uint (*FunctionPathCallback)(OptionList options);
#define FUNC_PATH_CALLBACK(name) uint name(OptionList options)

bool _requestHandlerAddFunctionPath(RequestHandler handler, const char* path, FunctionPathCallback callback, ...);
#define requestHandlerAddFunctionPath(handler, path, callback, ...) _requestHandlerAddFunctionPath(handler, path, callback, __VA_ARGS__, NULL);


int startServer(uint16_t port, ConnectionHandler handler);
