#include "webserver.h"
#include "filesystem/cache/filecache.h"

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
if(CODE != MHD_HTTP_OK){\
printf("sending empty response %s\n", #CODE);\
}\
return ret;

int prefixPath(const char buffer[], const char* prefix, size_t prefixSize, const char* str){
	
	size_t len = strlen(str);
	if (len > FILE_STRING_SIZE) {
		return 0;
	}
	memcpy((char*)buffer, prefix, prefixSize);
	memcpy((char*)buffer + prefixSize-1, str, len);
	return 1;
}

EXPORT int handleGetRequest(const char* url, size_t* uploadDataSize, const char* uploadData, Connection connection) {
	GetRequest request = { 0 };

	if (isOption(url, &request)) {

		if(request.type == list){

			const char* path = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "path");

			const char pageBase[] = "./site";
	 		char filePathBuffer[FILE_STRING_SIZE + sizeof(pageBase) + 1] = { 0 };
			if(!prefixPath(filePathBuffer,pageBase, sizeof(pageBase), path)){
				SEND_RESPONSE(MHD_HTTP_BAD_REQUEST, createEmptyResponse(),);
			}
			printf("filePathBuffer: %s\n", filePathBuffer);

			char* listedFiles;
			size_t listedFilesLength = 0;
			listFilesInDirectory(filePathBuffer, &listedFiles, &listedFilesLength);
		
			Response response = MHD_create_response_from_buffer(
				listedFilesLength,
				(void*) listedFiles, 
				MHD_RESPMEM_MUST_COPY
			);

			MHD_add_response_header(response, MHD_HTTP_HEADER_CONTENT_TYPE, "text/json");

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
			
			void* data;
			size_t size;
			size_t offset;
			size_t fileSize;
			if(!fetchFile(filePathBuffer, &data, &offset, &size, &fileSize)){
				SEND_RESPONSE(MHD_HTTP_INTERNAL_SERVER_ERROR, createEmptyResponse(),);
			}

			Response response = MHD_create_response_from_buffer(size, data, MHD_RESPMEM_PERSISTENT);

			char contentTypeBuffer[512] = {0};
			const char* fileTypeStr = getFileTypeStr(filePathBuffer);
			//size_t fileTypeLen = strlen(fileTypeStr);
			strcpy(contentTypeBuffer, fileTypeStr);
			strcat(contentTypeBuffer, "/");
			strcat(contentTypeBuffer, getFileExtension(filePathBuffer));

			MHD_add_response_header(response, MHD_HTTP_HEADER_CONTENT_TYPE, contentTypeBuffer );
			MHD_add_response_header(response,MHD_HTTP_HEADER_ACCEPT_RANGES, "bytes");

			char range[512] = {0};
			sprintf(range, "bytes=%zi-%zi/%zi",offset,offset+size-1,fileSize);

			MHD_add_response_header(response,MHD_HTTP_HEADER_RANGE, range);

			SEND_RESPONSE(
					MHD_HTTP_PARTIAL_CONTENT, 
					response,
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

