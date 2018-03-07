/**************************************************************************
 * Filename:       httpDataRequest.h
 * Author:         edward
 * E-Mail:         ouxiangping@feixuekj.cn
 * Description:    libevent evhttp data request
 *
 *  Copyright (C) 2016 FeiXue Company - http://www.feixuekj.cn/
 *
 * Version:         1.00  (2014-11-23,13:00)    :   Create the file.
 *************************************************************************/

#ifndef __TEMPLATE_H__
#define __TEMPLATE_H__

#ifdef __cplusplus
extern "C"
{
#endif

/*********************************************************************
 * INCLUDES
 */
#include <errno.h>
#include <stdlib.h> 
#include <string.h>
	
#include <event2/event.h>
#include <event2/buffer.h>
#include <event2/http.h>
#include <event2/http_struct.h>
#include <event2/keyvalq_struct.h>

/*********************************************************************
 * CONSTANTS
 */
// (default)
#define HTTP_CONTENT_TYPE_URL_ENCODED   "application/x-www-form-urlencoded"   
#define HTTP_CONTENT_TYPE_URL_JSON      "Content-Type:application/json;charset=UTF-8"
// (use for files: picture, mp3, tar-file etc.) 									   
#define HTTP_CONTENT_TYPE_FORM_DATA     "multipart/form-data"                 
// (use for plain text)
#define HTTP_CONTENT_TYPE_TEXT_PLAIN    "text/plain"

#define REQUEST_POST_FLAG               2
#define REQUEST_GET_FLAG                3
/*********************************************************************
 * MACROS
 */

/*********************************************************************
 * TYPEDEFS
 */
struct http_request_get {
	struct evhttp_uri *uri;
	struct event_base *base;
	struct evhttp_connection *cn;
	struct evhttp_request *req;
};

struct http_request_post {
	struct evhttp_uri *uri;
	struct event_base *base;
	struct evhttp_connection *cn;
	struct evhttp_request *req;
	char *content_type;
	char *post_data;
};

/*********************************************************************
 * VARIABLES
 */
 
    
/*********************************************************************
 * FUNCTIONS
 */
 
void *start_http_requset(struct event_base* base, const char *url, int req_get_flag, \
												const char *content_type, const char* data);
void *http_request_post_send(struct event_base* base, const char *url,const char *content_type, const char* data);
void *http_request_get_send(struct event_base* base, const char *url,const char *content_type, const char* data);
void http_request_get_free(struct http_request_get *http_req_get);
void http_request_post_free(struct http_request_get *http_req_get);

/*********************************************************************/

#ifdef __cplusplus
}
#endif

#endif /* __ENCRYPT_H__ */

