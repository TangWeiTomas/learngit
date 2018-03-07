/*******************************************************************************
 * Includes
 ******************************************************************************/
 #if 0
#include <stdlib.h>
#include <poll.h>
#include <string.h>
#include <stdio.h>

#include "epolling.h"
#include "logUtils.h"
/*******************************************************************************
 * Constants
 ******************************************************************************/
#define POLL_FDS_TO_ADD_EACH_TIME 3

#define EPOLL_FDS_MAX_SIZE	64

/*******************************************************************************
 * Global Variables
 ******************************************************************************/
//struct pollfd * polling_fds = NULL;
struct epoll_event  *epolling_fds = NULL;

int epolling_fds_count = 0;
bool epolling_quit = false;

/*******************************************************************************
 * Internal Variables
 ******************************************************************************/
epoll_fds_sidestruct_t * epoll_fds_sidestruct = NULL;

/*******************************************************************************
 * Functions
 ******************************************************************************/

//分配新的fd存储空间
int add_epoll_fds(int additional_count)
{
    struct pollfd * temp_fds;
    poll_fds_sidestruct_t * temp_poll_fds_sidestruct;
    int i;

    temp_fds = malloc(sizeof(struct pollfd) * (polling_fds_count + additional_count));
    if (temp_fds == NULL)
    {
        return -1;
    }

    temp_poll_fds_sidestruct = malloc(sizeof(poll_fds_sidestruct_t) * (polling_fds_count + additional_count));
    if (temp_poll_fds_sidestruct == NULL)
    {
        free(temp_fds);
        return -1;
    }
    //将原来的内容拷贝到新的地址空间
    memcpy(temp_fds, polling_fds, sizeof(struct pollfd) * polling_fds_count);
    memcpy(temp_poll_fds_sidestruct, poll_fds_sidestruct, sizeof(poll_fds_sidestruct_t) * polling_fds_count);

    //初始化新分配的空间
    for (i = polling_fds_count; i < (polling_fds_count + additional_count); i++)
    {
        temp_fds[i].fd = -1;
        temp_poll_fds_sidestruct[i].in_use = false;
    }
    //是否原来的旧空间
    free(polling_fds);
    free(poll_fds_sidestruct);

    //将新的空间地址信息保存到全局变量中
    polling_fds = temp_fds;
    poll_fds_sidestruct = temp_poll_fds_sidestruct;
    polling_fds_count += additional_count;

    return 0;
}

//将FD添加到poll_fds_sidestruct中
int epolling_define_poll_fd(int fd, short events, event_handler_cb_t event_handler_cb, void * event_handler_arg)
{
    int i;

    for (i = 0; i < polling_fds_count; i++)
    {
        if (poll_fds_sidestruct[i].in_use == false)
        {
            break;
        }
    }
    //如果全局变量poll_fds_sidestruct没有空余的位置，则分配新的存储空间，
    //如果有空余位置，则将新的加入到当前空闲的位置
    if (i == polling_fds_count)
    {
        if (add_poll_fds(POLL_FDS_TO_ADD_EACH_TIME) != 0)
        {
            return -1;
        }
    }

    poll_fds_sidestruct[i].in_use = true;
    poll_fds_sidestruct[i].event_handler_cb = event_handler_cb;
    poll_fds_sidestruct[i].event_handler_arg = event_handler_arg;

    polling_fds[i].fd = fd;
    polling_fds[i].events = events;

    return i;
}

void epolling_undefine_poll_fd(int fd_index)
{
    poll_fds_sidestruct[fd_index].in_use = false;
    polling_fds[fd_index].fd = -1;
}

bool epolling_process_activity(void)
{
    int i;
    if (poll(polling_fds, polling_fds_count, -1) > 0)
    {
        for (i = 0; i < polling_fds_count; i++)
        {
            if (poll_fds_sidestruct[i].in_use && (polling_fds[i].revents & polling_fds[i].events))
            {
                poll_fds_sidestruct[i].event_handler_cb(poll_fds_sidestruct[i].event_handler_arg);
            }
        }
    }
    else
    {
        log_debug("ERROR with poll()");
        polling_quit = true;
    }

    return (!polling_quit);
}
#endif 
