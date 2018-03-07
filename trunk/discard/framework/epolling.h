#ifndef __EPOLLING_H__
#define __EPOLLING_H__
#if 0
/*******************************************************************************
 * Includes
 ******************************************************************************/
#include <stdbool.h>
#include <sys/epoll.h> 


/*******************************************************************************
 * Types
 ******************************************************************************/
typedef void (* epoll_event_handler_cb_t)(void * arg);

typedef struct
{
    epoll_event_handler_cb_t epoll_event_handler_cb;
    void * epoll_event_handler_arg;
    bool in_use;
} epoll_fds_sidestruct_t;

/*******************************************************************************
 * Global Variables
 ******************************************************************************/
extern struct pollfd * polling_fds;
extern int epolling_fds_count;
extern bool epolling_quit;

/*******************************************************************************
 * Function Prototypes
 ******************************************************************************/
int epolling_define_poll_fd(int fd, short events, epoll_event_handler_cb_t epoll_event_handler_cb, void * event_handler_arg);
void epolling_undefine_poll_fd(int fd_index);
bool epolling_process_activity(void);
#endif
#endif /* POLLING_H */

