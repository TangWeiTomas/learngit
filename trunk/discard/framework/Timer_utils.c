/***********************************************************************************
 * 文 件 名   : Timer_utils.c
 * 负 责 人   : Edward
 * 创建日期   : 2016年7月19日
 * 文件描述   : 定时接口函数集
 * 版权说明   : Copyright (c) 2008-2016   xx xx xx xx 技术有限公司
 * 其    他   : 
 * 修改日志   : 
***********************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <sys/timerfd.h>
#include <stdio.h>
//#include <unistd.h>

#include "Timer_utils.h"
#include "logUtils.h"
#include "Polling.h"


#include <event2/event.h>



/******************************************************************************
 * Functions
 *****************************************************************************/
void tu_timer_handler(void * arg)
{
    tu_timer_t * timer = arg;
    uint64_t exp;
    log_debug("tu_timer_handler++\n");
    if (timer->continious)
    {
        if (read(polling_fds[timer->fd_index].fd, &exp, sizeof(uint64_t)) != sizeof(uint64_t))
        {
            log_err("%p ERROR timer read. Killing timer.\n", timer);
            tu_kill_timer(timer);
        }
    }
    else
    {
        tu_kill_timer(timer);
    }

    timer->timer_handler_cb(timer->timer_handler_arg);
    log_debug("tu_timer_handler--\n");
}

int tu_set_timer(tu_timer_t * timer, uint64_t milliseconds, bool continious, timer_handler_cb_t timer_handler_cb, void * timer_handler_arg)
{
    int fd;
    struct itimerspec its;
    tu_kill_timer(timer);
    
	log_debug("tu_set_timer\n");

    fd = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK);
    if (fd == -1)
    {
        log_err("Error creating timer");
        return -1;
    }

    its.it_value.tv_sec = (milliseconds * 1000000) / 1000000000;
    its.it_value.tv_nsec = (milliseconds * 1000000) % 1000000000;
    its.it_interval.tv_sec = continious ? its.it_value.tv_sec : 0;
    its.it_interval.tv_nsec = continious ? its.it_value.tv_nsec : 0;

    timer->timer_handler_cb = timer_handler_cb;
    timer->timer_handler_arg = timer_handler_arg;
    timer->continious = continious;

    if ((timerfd_settime(fd, 0, &its, NULL) == 0)
        && ((timer->fd_index = polling_define_poll_fd(fd, POLLIN, tu_timer_handler, timer)) != -1))
    {
    	log_err("tu_set_timer scessful\n");
        timer->in_use = true;
        return 0;
    }

    log_debug("Error setting timer\n");

    close(fd);
    return -1;
}

void tu_kill_timer(tu_timer_t * timer)
{
    //定义Linux定时器
    struct itimerspec its;
	log_debug("tu_kill_timer\n");
	
    if (timer->in_use)
    {
        its.it_value.tv_sec = 0;
        its.it_value.tv_nsec = 0;
        its.it_interval.tv_sec = 0;
        its.it_interval.tv_nsec = 0;

		if(polling_fds[timer->fd_index].fd != -1)
		{	
			log_debug("timerfd_settime\n");
	        if (timerfd_settime(polling_fds[timer->fd_index].fd, 0, &its, NULL) != 0)
	        {
	            log_err("ERROR killing timer\n");
	        }
			log_debug("close\n");
	        close(polling_fds[timer->fd_index].fd);
		}
		log_debug("polling_undefine_poll_fd\n");
        polling_undefine_poll_fd(timer->fd_index);

        timer->in_use = false;
    }
}

int tu_set_timer_realtime(tu_timer_t * timer, uint64_t milliseconds, bool continious, timer_handler_cb_t timer_handler_cb, void * timer_handler_arg)
{
    int fd;
    struct itimerspec its;
    struct timespec now;
    time_t tv_sec;
    long tv_nsec;
    tu_kill_timer(timer);
	log_debug("tu_set_timer_realtime\n");
    if(clock_gettime(CLOCK_REALTIME,&now) == -1)
    {
        log_err("Error clock_gettime timer\n");
        return -1;
    }

    fd = timerfd_create(CLOCK_REALTIME, TFD_NONBLOCK);
    if (fd == -1)
    {
        log_err("Error creating timer\n");
        return -1;
    }

    tv_sec = (milliseconds * 1000000) / 1000000000;
    tv_nsec = (milliseconds * 1000000) % 1000000000;

    //设置到期时间
    its.it_value.tv_sec  = now.tv_sec + tv_sec;
    its.it_value.tv_nsec = now.tv_nsec + tv_nsec;

    //如果使用循环模式，设置循环间隔
    its.it_interval.tv_sec = continious ? tv_sec : 0;
    its.it_interval.tv_nsec = continious ? tv_nsec : 0;

    timer->timer_handler_cb = timer_handler_cb;
    timer->timer_handler_arg = timer_handler_arg;
    timer->continious = continious;

    if ((timerfd_settime(fd,TFD_TIMER_ABSTIME, &its, NULL) == 0)
        && ((timer->fd_index = polling_define_poll_fd(fd, POLLIN, tu_timer_handler, timer)) != -1))
    {
    	log_err("tu_set_timer_realtime successful\n");
        timer->in_use = true;
        return 0;
    }

    log_debug("Error setting timer\n");

    close(fd);
    return -1;
}

//重新计时
int tu_reset_timer(tu_timer_t * timer,uint64_t milliseconds,bool continious)
{

    struct itimerspec its;
    struct timespec now;
    time_t tv_sec;
    long tv_nsec;
    
	log_debug("tu_reset_timer\n");
    if(timer == NULL)
        return -1;
    if(timer->in_use != true)
        return -1;

    if(clock_gettime(CLOCK_REALTIME,&now) == -1)
    {
        log_err("Error clock_gettime timer\n");
        return -1;
    }

    tv_sec = (milliseconds * 1000000) / 1000000000;
    tv_nsec = (milliseconds * 1000000) % 1000000000;

    //设置到期时间
    its.it_value.tv_sec  = now.tv_sec + tv_sec;
    its.it_value.tv_nsec = now.tv_nsec + tv_nsec;

    //如果使用循环模式，设置循环间隔
    its.it_interval.tv_sec = continious ? tv_sec : 0;
    its.it_interval.tv_nsec = continious ? tv_nsec : 0;

	if(polling_fds[timer->fd_index].fd != -1)
	{
	    if (timerfd_settime(polling_fds[timer->fd_index].fd,TFD_TIMER_ABSTIME, &its, NULL) == 0)
	    {
	    	log_err("timerfd_settime successful\n");
	        timer->in_use = true;
	        return 0;
	    }
	}
	
    log_debug("Error setting timer\n");

    return -1;
}

///////////////////////////////////////////////////////////////////////////////////////////////
//							基于libevent实现定时器任务  									 //
///////////////////////////////////////////////////////////////////////////////////////////////

void tu_evtimer_handler(evutil_socket_t fd, short event, void *args)
{
    tu_evtimer_t * timer = args;
    uint64_t exp;
    log_debug("tu_timer_handler++\n");
    if (timer->continious)
    {
        if (read(timer->fd, &exp, sizeof(uint64_t)) != sizeof(uint64_t))
        {
            log_err("%p ERROR timer read. Killing timer.\n", timer);
            tu_kill_evtimer(timer);
        }
    }
    else
    {
        tu_kill_evtimer(timer);
    }

    timer->timer_handler_cb(timer->timer_handler_arg);
    log_debug("tu_timer_handler--\n");
}


int tu_set_evtimer(tu_evtimer_t * timer, uint64_t milliseconds, bool continious, timer_handler_cb_t timer_handler_cb, void * timer_handler_arg)
{
    int fd = -1;
    struct itimerspec its;
    
    tu_kill_evtimer(timer);
    
	log_debug("tu_set_evtimer++\n");

    fd = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK);
    if (fd == -1)
    {
        log_err("Error creating timer");
        goto error;
    }
	
    its.it_value.tv_sec = (milliseconds * 1000000) / 1000000000;
    its.it_value.tv_nsec = (milliseconds * 1000000) % 1000000000;
    its.it_interval.tv_sec = continious ? its.it_value.tv_sec : 0;
    its.it_interval.tv_nsec = continious ? its.it_value.tv_nsec : 0;

    timer->timer_handler_cb = timer_handler_cb;
    timer->timer_handler_arg = timer_handler_arg;
    timer->continious = continious;
	timer->fd = fd;

	if(timer->base == NULL)
    {
		log_err("tu_set_evtimer base failed\n");
		goto error;
    }

    if (timerfd_settime(fd, 0, &its, NULL) != 0)
    {
    	log_err("timerfd_settime failed\n");
        goto error;
    }

    timer->evtimer= event_new(timer->base,fd,EV_READ|EV_PERSIST,tu_evtimer_handler,timer);
    if(timer->evtimer == NULL)
	{
		log_err("evtimer_new failed\n");
		goto error;
	}
	
	if(event_add(timer->evtimer,NULL) == -1)
	{
		log_err("evtimer_add failed\n");
		goto error;
	}

	timer->in_use = true;
	
	log_debug("tu_set_evtimer--\n");

  	return 0;
    
error:
	log_err("Error setting timer\n");
	
	if(timer->evtimer)
	{
		event_free(timer->evtimer);
		timer->evtimer = NULL;
		timer->in_use = false;
		timer->fd = -1;
	}
	
	if(fd != -1)
    	close(fd);
    	
    return -1;
}

void tu_kill_evtimer(tu_evtimer_t * timer)
{
    //定义Linux定时器
    struct itimerspec its;

	log_debug("tu_kill_timer++\n");

	if(timer == NULL)
    	return ;
    	
    if (timer->in_use)
    {
        its.it_value.tv_sec = 0;
        its.it_value.tv_nsec = 0;
        its.it_interval.tv_sec = 0;
        its.it_interval.tv_nsec = 0;

		if(timer->fd != -1)
		{	
	        if (timerfd_settime(timer->fd, 0, &its, NULL) != 0)
	        {
	            log_err("ERROR killing timer\n");
	        }
	        close(timer->fd);
	        timer->fd = -1;
		}
		
		if(timer->evtimer!=NULL)
	    {
	    	event_free(timer->evtimer);
	    	timer->evtimer = NULL;
	    	timer->in_use = false;
	    }
    }

    log_debug("tu_kill_timer--\n");
}

int tu_set_evtimer_realtime(tu_evtimer_t * timer, uint64_t milliseconds, bool continious, timer_handler_cb_t timer_handler_cb, void * timer_handler_arg)
{
    int fd = -1;
    struct itimerspec its;
    struct timespec now;
    time_t tv_sec;
    long tv_nsec;
    
    tu_kill_evtimer(timer);
    
	log_debug("tu_set_timer_realtime++\n");

    if(clock_gettime(CLOCK_REALTIME,&now) == -1)
    {
        log_err("Error clock_gettime timer\n");
        return -1;
    }

    fd = timerfd_create(CLOCK_REALTIME, TFD_NONBLOCK);
    if (fd == -1)
    {
        log_err("Error creating timer\n");
        return -1;
    }

    tv_sec = (milliseconds * 1000000) / 1000000000;
    tv_nsec = (milliseconds * 1000000) % 1000000000;

    //设置到期时间
    its.it_value.tv_sec  = now.tv_sec + tv_sec;
    its.it_value.tv_nsec = now.tv_nsec + tv_nsec;

    //如果使用循环模式，设置循环间隔
    its.it_interval.tv_sec = continious ? tv_sec : 0;
    its.it_interval.tv_nsec = continious ? tv_nsec : 0;

    timer->timer_handler_cb = timer_handler_cb;
    timer->timer_handler_arg = timer_handler_arg;
    timer->continious = continious;
	timer->fd = fd;


	if(timer->base == NULL)
    {
		log_err("tu_set_evtimer base failed\n");
		goto error;
    }
	
    if (timerfd_settime(fd,TFD_TIMER_ABSTIME, &its, NULL) != 0)
    {
    	log_err("event_new falied\n");
        goto error;
    }

    timer->evtimer= event_new(timer->base,fd,EV_READ|EV_PERSIST,tu_evtimer_handler,timer);
    if(timer->evtimer == NULL)
	{
		log_err("evtimer_new failed\n");
		goto error;
	}

	if(evtimer_add(timer->evtimer,NULL) == -1)
	{
		log_err("evtimer_add failed\n");
		goto error;
	}

	timer->in_use = true;
	log_debug("tu_set_timer_realtime--\n");	
    return 0 ;
    
error:
	log_err("Error setting timer\n");
    if(timer->evtimer)
	{
		event_free(timer->evtimer);
		timer->evtimer = NULL;
		timer->in_use = false;
		timer->fd = -1;
	}
	
	if(fd != -1)
    	close(fd);
    	
    return -1;
}

//重新计时
int tu_reset_evtimer(tu_evtimer_t * timer,uint64_t milliseconds,bool continious)
{
    struct itimerspec its;
    struct timespec now;
    time_t tv_sec;
    long tv_nsec;
    
	log_debug("tu_reset_timer++\n");
	
    if(timer == NULL)
        return -1;
      
    if(timer->in_use != true)
        return -1;

    if(clock_gettime(CLOCK_REALTIME,&now) == -1)
    {
        log_err("Error clock_gettime timer\n");
        return -1;
    }

    tv_sec = (milliseconds * 1000000) / 1000000000;
    tv_nsec = (milliseconds * 1000000) % 1000000000;
    
    //设置到期时间
    its.it_value.tv_sec  = now.tv_sec + tv_sec;
    its.it_value.tv_nsec = now.tv_nsec + tv_nsec;
	
    //如果使用循环模式，设置循环间隔
    its.it_interval.tv_sec = continious ? tv_sec : 0;
    its.it_interval.tv_nsec = continious ? tv_nsec : 0;

	if(timer->fd == -1)
	{
		log_err("timer->fd falied\n");
		goto error;
	}
		
    if (timerfd_settime(timer->fd,TFD_TIMER_ABSTIME, &its, NULL) != 0)
    {
    	log_err("event_new falied\n");
	    goto error;
    }
    
	/*
    if(event_add(timer->evtimer,NULL)==-1)
    {
		log_err("evtimer_add failed\n");
		goto error;
    }
	*/
	
	timer->in_use = true;

	log_debug("tu_reset_timer--\n");

	return 0;
error:
	log_err("Error setting timer\n");
	
	if(timer->evtimer)
	{
		event_free(timer->evtimer);
		timer->evtimer = NULL;
		timer->in_use = false;
		timer->fd = -1;
	}
	
	if(timer->fd != -1)
		close(timer->fd);
		
	return -1;
}

tu_evtimer_t * tu_evtimer_new(struct event_base *base)
{
	tu_evtimer_t *evtimer = NULL;
	ASSERT(base != NULL);

	evtimer = (tu_evtimer_t *)malloc(sizeof(tu_evtimer_t));
	if(evtimer == NULL)
		return NULL;

	memset(evtimer,0,sizeof(tu_evtimer_t));
	
	evtimer->base = base;
	evtimer->evtimer = NULL;
	evtimer->continious = false;
	evtimer->fd = -1;
	evtimer->in_use = false;
	evtimer->timer_handler_arg = NULL;
	evtimer->timer_handler_cb = NULL;

	return evtimer;
}

void tu_evtimer_free(tu_evtimer_t * evtimer)
{
	ASSERT(evtimer != NULL);
	if(evtimer->evtimer != NULL)
		event_free(evtimer->evtimer);
	free(evtimer);
}

