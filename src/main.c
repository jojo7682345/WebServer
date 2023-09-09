#include <stdio.h>
#include <microhttpd.h>
#include "server/server.h"

#define PORT 8888
 
FUNC_PATH_CALLBACK(listFiles){

};

int main(){

    GetRequestHandler getHandler;
    createGetRequestHandler(&getHandler);
    requestHandlerAddFunctionPath(getHandler, "files", &listFiles, "path");

    ConnectionHandler connectionHandler;
    ConnectionHandlerCreateInfo connectionHandlerInfo = {
        .getHandler = getHandler,
    };
    createConnectionHandler(connectionHandlerInfo, &connectionHandler);


    startServer(PORT, connectionHandler);

    destroyConnectionHandler(connectionHandler);

    return 0;
}