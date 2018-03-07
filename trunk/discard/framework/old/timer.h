#ifndef _CTIMER_H_
#define _CTIMER_H_
#include "queue.h"
#include <sys/time.h>

typedef enum{
		TIMER_IDLE=0,
		TIMER_ALIVE,
		TIMER_TIMEOUT
}TimerState;

typedef enum{
		TIMER_ONCE=0,
		TIMER_CIRCLE
}TimerType;

typedef struct CTimer{

	unsigned int id_;
	unsigned long m_interval;
	unsigned int m_counter;
	struct timeval m_endtime;
	TimerState m_state;
	TimerType m_type;
	void (*m_func)(struct CTimer *,void *);
	void * m_data;
	TAILQ_ENTRY(CTimer) entry_;
}CTimer;


void Init_Timer(CTimer * Timer,unsigned int vinterval,void (*vfunc)(CTimer *,void *),void *vdata,TimerType vtype);
void start_Timer(CTimer * Timer);
void stop_Timer(CTimer * Timer);
void reset_Timer(CTimer * Timer,unsigned int vinterval);
void Release_Timer(CTimer * Timer);
#endif
