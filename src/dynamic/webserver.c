#include "webserver.h"
#include "filesystem/fileLoading.h"

#ifdef _WIN32
//#define EXPORT __declspec(dllexport)
#define EXPORT
#else
#define EXPORT
#endif


#define LIST_OF_METHODS \
	METHOD(none)\
	METHOD(list)\
	METHOD(open)\
	METHOD(options)\
	METHOD(download)\
	METHOD(stream)

#define METHOD(name) name,
typedef enum MethodType {
	LIST_OF_METHODS
}MethodType;
#undef METHOD

typedef struct GetRequest {
	MethodType type;
	char path[FILE_STRING_SIZE + 1];
	char file[FILE_STRING_SIZE + 1];
} GetRequest;

#define checkOption(str) (memcmp(url, str, __min(typeLen,sizeof(str)))==0)


int findOptions(const char* url, GetRequest* getRequest) {

	size_t len = strlen(url);

	char* typeEnd = memchr(url, '?', len);

	if (!typeEnd) {
		return 0;
	}

	size_t typeLen = typeEnd - url;

#define METHOD(name) if(checkOption(#name)) {\
	getRequest->type=name;\
}
	LIST_OF_METHODS
#undef METHOD

		if (getRequest->type == none) {
			return 0;
		}

	printf("not supported yet\n");

	return 1;
}


Response createEmptyResponse() {
	return MHD_create_response_from_buffer(0, nullptr, MHD_RESPMEM_PERSISTENT);
}

#define SEND_RESPONSE(CODE, resp) int ret = MHD_queue_response(connection, CODE, resp);\
MHD_destroy_response(resp);\
return ret;

EXPORT int handleGetRequest(const char* url, size_t* uploadDataSize, const char* uploadData, Connection connection) {
	GetRequest request = { 0 };
	if (findOptions(url, &request)) {



	} else {
		// url is a page file
		if (strcmp(url, "/") == 0) {
			url = "/index.html";
		}
		const char pageBase[] = "./site";
		char filePathBuffer[FILE_STRING_SIZE + sizeof(pageBase) + 1] = { 0 };
		size_t urlLen = strlen(url);
		if (urlLen > FILE_STRING_SIZE) {
			SEND_RESPONSE(MHD_HTTP_BAD_REQUEST, createEmptyResponse());
		}
		memcpy(filePathBuffer, pageBase, sizeof(pageBase));
		memcpy(filePathBuffer + sizeof(pageBase)-1, url, urlLen);

		printf("%s\n", filePathBuffer);

		FileHandle file;
		if (!attachFile(filePathBuffer, &file, 0)) {
			printf("unable to open file %s\n", filePathBuffer);
			SEND_RESPONSE(MHD_HTTP_NOT_FOUND, createEmptyResponse());
		}

		size_t sendSize = loadFile(file, 0);
		void* fileData = malloc(sendSize);
		if (!fileData) {
			SEND_RESPONSE(MHD_HTTP_INTERNAL_SERVER_ERROR, createEmptyResponse());
			return 0;
		}
		size_t readSize = readFile(file, 0, fileData);
		releaseFile(file);

		SEND_RESPONSE(MHD_HTTP_OK, MHD_create_response_from_buffer(readSize, fileData, MHD_RESPMEM_MUST_FREE));
	}

	SEND_RESPONSE(MHD_HTTP_NOT_IMPLEMENTED, createEmptyResponse());
}

EXPORT int handlePostRequest(const char* url, size_t* uploadDataSize, const char* uploadData, Connection connection) {

	Response response = MHD_create_response_from_buffer(0, nullptr, MHD_RESPMEM_PERSISTENT);
	int ret = MHD_queue_response(connection, MHD_HTTP_NOT_IMPLEMENTED, response);
	MHD_destroy_response(response);

	return ret;
}

EXPORT int handleDeleteRequest(const char* url, size_t* uploadDataSize, const char* uploadData, Connection connection) {
	Response response = MHD_create_response_from_buffer(0, nullptr, MHD_RESPMEM_PERSISTENT);
	int ret = MHD_queue_response(connection, MHD_HTTP_NOT_IMPLEMENTED, response);
	MHD_destroy_response(response);
	return ret;
}

