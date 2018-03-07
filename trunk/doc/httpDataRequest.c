/**************************************************************************
 * Filename:       httpDataRequest.c
 * Author:         edward
 * E-Mail:         ouxiangping@feixuekj.cn
 * Description:    libevent evhttp data request
 *
 *  Copyright (C) 2016 FeiXue Company - http://www.feixuekj.cn/
 *
 * Version:         1.00  (2014-11-23,13:00)    :   Create the file.
 *************************************************************************/


/*********************************************************************
* INCLUDES
*/
#include "httpDataRequest.h"
#include "logUtils.h"

/*********************************************************************
* MACROS
*/

/*********************************************************************
* CONSTANTS
*/

/*********************************************************************
* TYPEDEFS
*/

/*********************************************************************
* GLOBAL VARIABLES
*/

       
/*********************************************************************
* LOCAL VARIABLES
*/


/*********************************************************************
* LOCAL FUNCTIONS
*/

/*********************************************************************
* GLOBAL FUNCTIONS
*/
static inline void print_request_head_info(struct evkeyvalq *header)
{
	struct evkeyval *first_node = header->tqh_first;
	while (first_node) {
		log_debug("key:%s  value:%s", first_node->key, first_node->value);
		first_node = first_node->next.tqe_next;
	}
}

static inline void print_uri_parts_info(const struct evhttp_uri * http_uri)
{
	log_debug("scheme:%s\n", evhttp_uri_get_scheme(http_uri));
	log_debug("host:%s\n", evhttp_uri_get_host(http_uri));
	log_debug("path:%s\n", evhttp_uri_get_path(http_uri));
	log_debug("port:%d\n", evhttp_uri_get_port(http_uri));
	log_debug("query:%s\n", evhttp_uri_get_query(http_uri));
	log_debug("userinfo:%s\n", evhttp_uri_get_userinfo(http_uri));
	log_debug("fragment:%s\n", evhttp_uri_get_fragment(http_uri));
}

void http_requset_post_cb(struct evhttp_request *req, void *arg)
{
	log_debug("http_requset_post_cb++\n");
    struct http_request_post *http_req_post = (struct http_request_post *)arg;
	http_request_post_free((struct http_request_get *)http_req_post);
	//evhttp_request_free(req);
	//log_debug("http_requset_post_cb++ %s\n",req->response_code);
	/*
    switch(req->response_code)
    {
        case HTTP_OK:
        {
			log_debug("HTTP_OK++\n");
			/*
            struct evbuffer* buf = evhttp_request_get_input_buffer(req);
            size_t len = evbuffer_get_length(buf);
            log_debug( "print the head info:");
            print_request_head_info(req->output_headers);
            
            log_debug("len:%zu  body size:%zu", len, req->body_size);       
            char *tmp = malloc(len+1);
            memcpy(tmp, evbuffer_pullup(buf, -1), len);
            tmp[len] = '\0';
            log_debug( "print the body:");
            log_debug("HTML BODY:%s", tmp);
            free(tmp);
            http_request_post_free((struct http_request_get *)http_req_post);
            //event_base_loopexit(http_req_post->base, 0);	
		
            break;
        }
        case HTTP_MOVEPERM:
           log_debug("the uri moved permanently\n");
            break;
        case HTTP_MOVETEMP:
        {
			log_debug("HTTP_MOVETEMP++\n");
			/*
            const char *new_location = evhttp_find_header(req->input_headers, "Location");
            struct evhttp_uri *new_uri = evhttp_uri_parse(new_location);
            evhttp_uri_free(http_req_post->uri);
            http_req_post->uri = new_uri;
            start_url_request((struct http_request_get *)http_req_post, REQUEST_POST_FLAG);
			
            return;
        }
            
        default:
        	log_debug("default++\n");
         	//http_request_post_free((struct http_request_get *)http_req_post);
            //event_base_loopexit(http_req_post->base, 0);
            return;
    }
    */
	log_debug("http_requset_post_cb--\n");
}

void http_requset_get_cb(struct evhttp_request *req, void *arg)
{
    struct http_request_get *http_req_get = (struct http_request_get *)arg;
    switch(req->response_code)
    {
        case HTTP_OK:
        {
            struct evbuffer* buf = evhttp_request_get_input_buffer(req);
            size_t len = evbuffer_get_length(buf);
            log_debug( "print the head info:");
            print_request_head_info(req->output_headers);
            
            log_debug("len:%zu  body size:%zu", len, req->body_size);
            char *tmp = malloc(len+1);
            memcpy(tmp, evbuffer_pullup(buf, -1), len);
            tmp[len] = '\0';
            log_debug( "print the body:");
            log_debug("HTML BODY:%s", tmp);
            free(tmp);

			http_request_get_free(http_req_get);
            //event_base_loopexit(http_req_get->base, 0);
            break;
        }
        case HTTP_MOVEPERM:
           log_debug( "%s", "the uri moved permanently");
           break;
        case HTTP_MOVETEMP:
        {
            const char *new_location = evhttp_find_header(req->input_headers, "Location");
            struct evhttp_uri *new_uri = evhttp_uri_parse(new_location);
            evhttp_uri_free(http_req_get->uri);
            http_req_get->uri = new_uri;
            start_url_request(http_req_get, REQUEST_GET_FLAG);
            return;
        }
            
        default:
        	http_request_get_free(http_req_get);
            //event_base_loopexit(http_req_get->base, 0);
            return;
    }
}

int start_url_request(struct http_request_get *http_req, int req_get_flag)
{
    if (http_req->cn)
        evhttp_connection_free(http_req->cn);
    
    int port = evhttp_uri_get_port(http_req->uri);
    http_req->cn = evhttp_connection_base_new(http_req->base,
                                                   NULL,
                                                   evhttp_uri_get_host(http_req->uri),
                                                   (port == -1 ? 80 : port));
    
    /**
     * Request will be released by evhttp connection
     * See info of evhttp_make_request()
     */
    if (req_get_flag == REQUEST_POST_FLAG) {
        http_req->req = evhttp_request_new(http_requset_post_cb, http_req);
    } else if (req_get_flag ==  REQUEST_GET_FLAG) {
        http_req->req = evhttp_request_new(http_requset_get_cb, http_req);
    }
    
    if (req_get_flag == REQUEST_POST_FLAG) {
        const char *path = evhttp_uri_get_path(http_req->uri);
        evhttp_make_request(http_req->cn, http_req->req, EVHTTP_REQ_POST,
                            path ? path:"/");
		log_debug("PATH:%s\n",path);
        /** Set the post data */
        struct http_request_post *http_req_post = (struct http_request_post *)http_req;
        evbuffer_add(http_req_post->req->output_buffer, http_req_post->post_data, strlen(http_req_post->post_data));
        evhttp_add_header(http_req_post->req->output_headers, "Content-Type", http_req_post->content_type);
    } else if (req_get_flag == REQUEST_GET_FLAG) {
        const char *query = evhttp_uri_get_query(http_req->uri);
        const char *path = evhttp_uri_get_path(http_req->uri);
        size_t len = (query ? strlen(query) : 0) + (path ? strlen(path) : 0) + 1;
        char *path_query = NULL;
        if (len > 1) {
            path_query = calloc(len, sizeof(char));
            sprintf(path_query, "%s?%s", path, query);
        }        
        evhttp_make_request(http_req->cn, http_req->req, EVHTTP_REQ_GET,
                             path_query ? path_query: "/");
    }
	
    /** Set the header properties */
    evhttp_add_header(http_req->req->output_headers, "Host", evhttp_uri_get_host(http_req->uri));
    
    return 0;
}


void *http_request_new(struct event_base* base, const char *url, int req_get_flag, \
                       const char *content_type, const char* data)
{
    int len = 0;
    if (req_get_flag == REQUEST_GET_FLAG) {
        len = sizeof(struct http_request_get);
    } else if(req_get_flag == REQUEST_POST_FLAG) {
        len = sizeof(struct http_request_post);
    }
    
    struct http_request_get *http_req_get = calloc(1, len);
    http_req_get->uri = evhttp_uri_parse(url);
    //print_uri_parts_info(http_req_get->uri);
    
    http_req_get->base = base;
    
    if (req_get_flag == REQUEST_POST_FLAG) {
        struct http_request_post *http_req_post = (struct http_request_post *)http_req_get;
        if (content_type == NULL) {
            content_type = HTTP_CONTENT_TYPE_URL_ENCODED;
        }
        http_req_post->content_type = strdup(content_type);
        
        if (data == NULL) {
            http_req_post->post_data = NULL;
        } else {
            http_req_post->post_data = strdup(data);
        }
    }
    
    return http_req_get;
}

void http_request_free(struct http_request_get *http_req_get, int req_get_flag)
{
    evhttp_connection_free(http_req_get->cn);
    evhttp_uri_free(http_req_get->uri);
	//evhttp_request_free(http_req_get->req);
    if (req_get_flag == REQUEST_GET_FLAG) {
        free(http_req_get);
    } else if(req_get_flag == REQUEST_POST_FLAG) {
        struct http_request_post *http_req_post = (struct http_request_post*)http_req_get;
        if (http_req_post->content_type) {
            free(http_req_post->content_type);
        }
        if (http_req_post->post_data) {
            free(http_req_post->post_data);
        }
        free(http_req_post);
    }
    http_req_get = NULL;
}

void http_request_post_free(struct http_request_get *http_req_get)
{
	http_request_free(http_req_get,REQUEST_POST_FLAG);
}

void http_request_get_free(struct http_request_get *http_req_get)
{
	http_request_free(http_req_get,REQUEST_GET_FLAG);
}

/*********************************************************************
* @fn          start_http_requset
*
* @brief       启动http数据请求
*
* @param       base - struct event_base objects.
			   url  - url
*
* @return      struct http_request_get
*/
void *start_http_requset(struct event_base* base, const char *url, int req_get_flag, \
												const char *content_type, const char* data)
{
	struct http_request_get *http_req_get = http_request_new(base, url, req_get_flag, content_type, data);
	start_url_request(http_req_get, req_get_flag);
	
	return http_req_get;
}

void *http_request_post_send(struct event_base* base, const char *url,const char *content_type, const char* data)
{
	return start_http_requset(base,url,REQUEST_POST_FLAG,content_type,data);
}

void *http_request_get_send(struct event_base* base, const char *url,const char *content_type, const char* data)
{
	return start_http_requset(base,url,REQUEST_GET_FLAG,content_type,data);
}

/*********************************************************************/

