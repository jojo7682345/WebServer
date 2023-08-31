#ifndef WEB_SERVER_H_
#define WEB_SERVER_H_

#include <microhttpd.h>
typedef struct MHD_Connection* Connection;
typedef struct MHD_Response* Response;
#define nullptr ((void*)0)

#define LIST_OF_FUNCIONS \
    FUNC(handleGetRequest, int, const char*, size_t*, const char*, Connection) \
    FUNC(handlePostRequest, int, const char*, size_t*, const char*, Connection) \
    FUNC(handleDeleteRequest, int, const char*, size_t*, const char*, Connection)

#ifndef HOT_RELOADING

#define FUNC(name, ret, ...) ret name(__VA_ARGS__);
LIST_OF_FUNCIONS
#undef FUNC

#else

#define FUNC(name, ret, ...) typedef ret (*name##_t)(__VA_ARGS__);
LIST_OF_FUNCIONS
#undef FUNC



#endif

#endif // WEB_SERVER_H_