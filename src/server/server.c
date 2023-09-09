#include "server.h"
#include "../files/fileReading.h"
#include <microhttpd.h>
#include <stdlib.h>
#include <string.h>

typedef struct MHD_Connection* Connection;
typedef struct MHD_Response* Response;
#define nullptr ((void*)0)

int handleGetRequest(const char* url, size_t* uploadDataSize, const char* uploadData, Connection connection) {
	
    const char* html = "<head></head><body>Hello World</body>";

	const char* path = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "path");

    Response response = MHD_create_response_from_buffer(strlen(html), html, MHD_RESPMEM_PERSISTENT);
	int ret = MHD_queue_response(connection, MHD_HTTP_OK, response);
	MHD_destroy_response(response);

	return ret;
}

int answer_to_connection(void* cls, struct MHD_Connection* connection,
						 const char* url,
						 const char* method, const char* version,
						 const char* upload_data,
						 size_t* upload_data_size, void** con_cls) {

	if (strcmp(method, "GET") == 0) {
		return handleGetRequest(url, upload_data_size, upload_data, connection);
	}

	Response response = MHD_create_response_from_buffer(0, nullptr, MHD_RESPMEM_PERSISTENT);
	int ret = MHD_queue_response(connection, MHD_HTTP_NOT_IMPLEMENTED, response);
	MHD_destroy_response(response);
	return ret;
	return MHD_NO;
}


int startServer(uint16_t port, ConnectionHandler handler){

    struct MHD_Daemon* daemon;
	daemon = MHD_start_daemon(MHD_USE_INTERNAL_POLLING_THREAD, port, NULL, NULL,
							  (MHD_AccessHandlerCallback)&answer_to_connection, NULL, MHD_OPTION_END);
	if (NULL == daemon) return 1;
	getchar();


	MHD_stop_daemon(daemon);

    return 0;
}