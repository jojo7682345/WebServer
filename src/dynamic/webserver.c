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

#define MIN(A,B) ((A) < (B) ? (A) : (B))

#define checkOption(str) (memcmp(url+1, str, MIN(len,sizeof(str)))==0)


int isOption(const char* url, GetRequest* getRequest) {

	size_t len = strlen(url);
	if(len <= 1){
		return 0;
	}
#define METHOD(name) if(checkOption(#name)) {\
	getRequest->type=name;\
	return 1;\
}
	LIST_OF_METHODS
#undef METHOD

		if (getRequest->type == none) {
			return 0;
		}

	return 0;
}


Response createEmptyResponse() {
	return MHD_create_response_from_buffer(0, nullptr, MHD_RESPMEM_PERSISTENT);
}

#define SEND_RESPONSE(CODE, resp, callback) int ret = MHD_queue_response(connection, CODE, resp);\
MHD_destroy_response(resp);\
callback;\
return ret;

EXPORT int handleGetRequest(const char* url, size_t* uploadDataSize, const char* uploadData, Connection connection) {
	GetRequest request = { 0 };

	if (isOption(url, &request)) {

		if(request.type == list){

			const char* path = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "path");

			const char pageBase[] = "./site";
	 		char filePathBuffer[FILE_STRING_SIZE + sizeof(pageBase) + 1] = { 0 };
			size_t urlLen = strlen(path);
			if (urlLen > FILE_STRING_SIZE) {
				SEND_RESPONSE(MHD_HTTP_BAD_REQUEST, createEmptyResponse(),);
			}
			memcpy(filePathBuffer, pageBase, sizeof(pageBase));
			memcpy(filePathBuffer + sizeof(pageBase)-1, path, urlLen);

			char* listedFiles;
			size_t listedFilesLength = 0;
			listFilesInDirectory(filePathBuffer, &listedFiles, &listedFilesLength);
		
			Response response = MHD_create_response_from_buffer(
				listedFilesLength,
				(void*) listedFiles, 
				MHD_RESPMEM_MUST_COPY
			);

			MHD_add_response_header(response, MHD_HTTP_HEADER_CONTENT_TYPE, "text/html");

			SEND_RESPONSE(
					MHD_HTTP_OK, 
					response,
					free(listedFiles);
			);

		}

		if(request.type == open){

			const char* path = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "path");
			const char* fileName = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "file");

			const char pageBase[] = "./site";
	 		char filePathBuffer[FILE_STRING_SIZE + sizeof(pageBase) + 1] = { 0 };
			size_t urlLen = strlen(path);
			size_t fileNameLen = strlen(fileName);
			if (urlLen+fileNameLen > FILE_STRING_SIZE) {
				SEND_RESPONSE(MHD_HTTP_BAD_REQUEST, createEmptyResponse(),);
			}
			memcpy(filePathBuffer, pageBase, sizeof(pageBase));
			memcpy(filePathBuffer + sizeof(pageBase)-1, path, urlLen);
			memcpy(filePathBuffer + urlLen + sizeof(pageBase)-1, fileName, fileNameLen);
			
			
			FileHandle file;
			if (!attachFile(filePathBuffer, &file, 0)) {
				printf("unable to open file %s\n", filePathBuffer);
				SEND_RESPONSE(MHD_HTTP_NOT_FOUND, createEmptyResponse(),);
			}
	
			size_t sendSize = loadFile(file, 0);
			void* fileData = malloc(sendSize);
			if (!fileData) {
				SEND_RESPONSE(MHD_HTTP_INTERNAL_SERVER_ERROR, createEmptyResponse(),);
				return 0;
			}
			size_t readSize = readFile(file, 0, fileData);
			releaseFile(file);

			Response response = MHD_create_response_from_buffer(readSize, fileData, MHD_RESPMEM_MUST_COPY);

			char* contentTypeBuffer[512] = {0};
			const char* fileTypeStr = getFileTypeStr(filePathBuffer);
			size_t fileTypeLen = strlen(fileTypeStr);
			strcpy(contentTypeBuffer, fileTypeStr);
			strcat(contentTypeBuffer, "/");
			strcat(contentTypeBuffer, getFileExtension(filePathBuffer));

			MHD_add_response_header(response, MHD_HTTP_HEADER_CONTENT_TYPE, contentTypeBuffer );
			
			SEND_RESPONSE(
					MHD_HTTP_OK, 
					response,
					free(fileData);
			);

		}

	} else {
		// url is a page file
		if (strcmp(url, "/") == 0) {
			url = "/index.html";
		}
		const char pageBase[] = "./site";
		char filePathBuffer[FILE_STRING_SIZE + sizeof(pageBase) + 1] = { 0 };
		size_t urlLen = strlen(url);
		if (urlLen > FILE_STRING_SIZE) {
			SEND_RESPONSE(MHD_HTTP_BAD_REQUEST, createEmptyResponse(),);
		}
		memcpy(filePathBuffer, pageBase, sizeof(pageBase));
		memcpy(filePathBuffer + sizeof(pageBase)-1, url, urlLen);


		FileHandle file;
		if (!attachFile(filePathBuffer, &file, 0)) {
			printf("unable to open file %s\n", filePathBuffer);
			SEND_RESPONSE(MHD_HTTP_NOT_FOUND, createEmptyResponse(),);
		}

		size_t sendSize = loadFile(file, 0);
		void* fileData = malloc(sendSize);
		if (!fileData) {
			SEND_RESPONSE(MHD_HTTP_INTERNAL_SERVER_ERROR, createEmptyResponse(),);
			return 0;
		}
		size_t readSize = readFile(file, 0, fileData);
		releaseFile(file);

		SEND_RESPONSE(
				MHD_HTTP_OK, 
				MHD_create_response_from_buffer(
					readSize, 
					fileData, 
					MHD_RESPMEM_MUST_COPY
				),
				free(fileData);
		);
	}

	SEND_RESPONSE(MHD_HTTP_NOT_IMPLEMENTED, createEmptyResponse(),);
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

