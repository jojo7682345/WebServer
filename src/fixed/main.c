#include <stdio.h>
//#include <conio.h>

#include "../dynamic/webserver.h"

#define PORT 8888

int answer_to_connection(void* cls, struct MHD_Connection* connection,
						 const char* url,
						 const char* method, const char* version,
						 const char* upload_data,
						 size_t* upload_data_size, void** con_cls) {
	if (strcmp(method, "GET") == 0) {
		return handleGetRequest(url, upload_data_size, upload_data, connection);
	}
	if (strcmp(method, "POST") == 0) {
		return handlePostRequest(url, upload_data_size, upload_data, connection);
	}
	if (strcmp(method, "DELETE") == 0) {
		return handleDeleteRequest(url, upload_data_size, upload_data, connection);
	}
	printf("wrong method\n");
	return MHD_NO;
}



int main() {

	struct MHD_Daemon* daemon;
	daemon = MHD_start_daemon(MHD_USE_INTERNAL_POLLING_THREAD, PORT, NULL, NULL,
							  (MHD_AccessHandlerCallback)&answer_to_connection, NULL, MHD_OPTION_END);
	if (NULL == daemon) return 1;
	getchar();


	MHD_stop_daemon(daemon);

	printf("returned succesfully\n");
	
	return 0;
}
