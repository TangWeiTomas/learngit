#include "timer_manager.h"
#include <stdio.h>
#include <sys/select.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

pthread_mutex_t m_mutex= PTHREAD_MUTEX_INITIALIZER;
CTimerManager * m_instance=NULL;
unsigned int mark=0;

void Init_CTimerManager()
{
    m_instance->m_state = TIMER_MANAGER_STOP;
    TAILQ_INIT(&(m_instance->list_));
}

CTimerManager * instance()
{
    if(m_instance==NULL)
    {
        pthread_mutex_lock(&m_mutex);
        if(m_instance==NULL)
        {
            m_instance= malloc(sizeof(CTimerManager));
            memset(m_instance,0,sizeof(CTimerManager));
        }
        pthread_mutex_unlock(&m_mutex);
    }
    return m_instance;
}

void add_timer_(CTimer * vtimer)
{	
    if(vtimer->m_state==TIMER_ALIVE) return;

    CTimer *item;
    struct timeval now,interval;

    vtimer->m_state=TIMER_ALIVE;

    interval.tv_sec=vtimer->m_interval/1000;
    interval.tv_usec=(vtimer->m_interval%1000)*1000;
    gettimeofday(&now,0);

    timeradd(&now,&interval,&vtimer->m_endtime);
    TAILQ_FOREACH(item, &(m_instance->list_), entry_)
    {
        if(timercmp(&item->m_endtime,&vtimer->m_endtime,>=))	
        {
            TAILQ_INSERT_BEFORE(item, vtimer, entry_);
            break;
        }
    }
    
    if(item==TAILQ_END(&(m_instance->list_)))
    {
        TAILQ_INSERT_TAIL(&(m_instance->list_), vtimer,entry_);
    }
}

void remove_timer_(CTimer * vtimer)
{
    if(vtimer->m_state!=TIMER_ALIVE) return;
    TAILQ_REMOVE(&(m_instance->list_),vtimer,entry_);
    vtimer->m_state=TIMER_IDLE;
}
void add_timer(CTimer * vtimer)
{
    pthread_mutex_lock(&m_mutex);
    add_timer_(vtimer);
    vtimer->id_=++mark;
    pthread_mutex_unlock(&m_mutex);
}
void remove_timer(CTimer * vtimer)
{
    pthread_mutex_lock(&m_mutex);
    remove_timer_(vtimer);
    vtimer->m_state=TIMER_IDLE;
    pthread_mutex_unlock(&m_mutex);
}
void  start_CTimerManager(unsigned long interval,unsigned long repair)
{
    instance();
    Init_CTimerManager();
    m_instance->m_interval.tv_sec=interval/1000;
    m_instance->m_interval.tv_usec=(interval%1000)*1000;
    m_instance->m_repair.tv_sec=repair/1000;
    m_instance->m_repair.tv_usec=(repair%1000)*1000;
    if(m_instance->m_state==TIMER_MANAGER_STOP)
    {
        m_instance->m_state=TIMER_MANAGER_START;
        pthread_t pid;
        pthread_create(&pid,0,process,m_instance);
    }
}

void  stop_CTimerManager()
{
    m_instance->m_state=TIMER_MANAGER_STOP;
}
void dump_CTimerManager()
{
    CTimer *item;
    struct timeval now;
    pthread_mutex_lock(&m_mutex);
    gettimeofday(&now,0);
    struct timeval subTimer;
    unsigned int nTimeToEnd;
    printf("----id--interval--endtime--type--state----\n");
    TAILQ_FOREACH(item, &(m_instance->list_), entry_)
    {
        timersub(&item->m_endtime,&now,&subTimer);
        nTimeToEnd=subTimer.tv_sec*1000+subTimer.tv_usec/1000;
        printf("      %d    %ld    %d    %d    %d\n",item->id_,item->m_interval,nTimeToEnd,item->m_type,item->m_state);
    }
    pthread_mutex_unlock(&m_mutex);
}



void *  process(void * arg)
{
    printf("process++ \n");

    pthread_detach(pthread_self());

    CTimerManager *manager=(CTimerManager *)arg;

    CTimer *item;
    struct timeval now,stand;
    unsigned int delay;
    struct timeval tm;
    CTimer tmpTimer;

    while(manager->m_state==TIMER_MANAGER_START)
    {

        tm.tv_sec=manager->m_interval.tv_sec;
        tm.tv_usec=manager->m_interval.tv_usec;
        while(select(0,0,0,0,&tm)<0&&errno==EINTR);
        gettimeofday(&now,0);
        timeradd(&now,&manager->m_repair,&stand);
        pthread_mutex_lock(&manager->m_mutex);
        TAILQ_FOREACH(item, &(manager->list_), entry_)
        {
            if(timercmp(&item->m_endtime,&stand,<))	
            {

                if(item->m_func)
                {
                    item->m_func(item,item->m_data);
                }

                if(item->m_type==TIMER_ONCE)
                {
                    remove_timer_(item);
                    item->m_state=TIMER_TIMEOUT;
                }
                else if(item->m_type==TIMER_CIRCLE)
                {
                    tmpTimer.entry_=item->entry_;
                    remove_timer_(item);
                    add_timer_(item);
                    item=&tmpTimer;
                }
            }
            else
            {
                break;
            }
        }
        pthread_mutex_unlock(&manager->m_mutex);
    }
}

