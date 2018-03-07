/*******************************************************************************
 Filename:       timer_utils.h
 Revised:        $Date: 2014-05-19 16:36:17 -0700 (Mon, 19 May 2014) $
 Revision:       $Revision: 38594 $

 Description:   Timer utilities


 Copyright 2013 Texas Instruments Incorporated. All rights reserved.

 IMPORTANT: Your use of this Software is limited to those specific rights
 granted under the terms of a software license agreement between the user
 who downloaded the software, his/her employer (which must be your employer)
 and Texas Instruments Incorporated (the "License").  You may not use this
 Software unless you agree to abide by the terms of the License. The License
 limits your use, and you acknowledge, that the Software may not be modified,
 copied or distributed unless used solely and exclusively in conjunction with
 a Texas Instruments radio frequency device, which is integrated into
 your product.  Other than for the foregoing purpose, you may not use,
 reproduce, copy, prepare derivative works of, modify, distribute, perform,
 display or sell this Software and/or its documentation for any purpose.

 YOU FURTHER ACKNOWLEDGE AND AGREE THAT THE SOFTWARE AND DOCUMENTATION ARE
 PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED,l
 INCLUDING WITHOUT LIMITATION, ANY WARRANTY OF MERCHANTABILITY, TITLE,
 NON-INFRINGEMENT AND FITNESS FOR A PARTICULAR PURPOSE. IN NO EVENT SHALL
 TEXAS INSTRUMENTS OR ITS LICENSORS BE LIABLE OR OBLIGATED UNDER CONTRACT,
 NEGLIGENCE, STRICT LIABILITY, CONTRIBUTION, BREACH OF WARRANTY, OR OTHER
 LEGAL EQUITABLE THEORY ANY DIRECT OR INDIRECT DAMAGES OR EXPENSES
 INCLUDING BUT NOT LIMITED TO ANY INCIDENTAL, SPECIAL, INDIRECT, PUNITIVE
 OR CONSEQUENTIAL DAMAGES, LOST PROFITS OR LOST DATA, COST OF PROCUREMENT
 OF SUBSTITUTE GOODS, TECHNOLOGY, SERVICES, OR ANY CLAIMS BY THIRD PARTIES
 (INCLUDING BUT NOT LIMITED TO ANY DEFENSE THEREOF), OR OTHER SIMILAR COSTS.

 Should you have any questions regarding your right to use this Software,
 contact Texas Instruments Incorporated at www.TI.com.
*******************************************************************************/
#ifndef TIMER_UTILS_H
#define TIMER_UTILS_H

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <stdint.h>
#include <stdbool.h>
#include <event2/util.h>
#include "globalVal.h"

/******************************************************************************
 * Constants
 *****************************************************************************/
#define TIMER_RESET_VALUE {NULL, NULL, -1, false, false}

/******************************************************************************
 * Types
 *****************************************************************************/
typedef void (* timer_handler_cb_t)(void * arg);
typedef void (* evtimer_handler_cb_t)(evutil_socket_t fd, short event, void *args);

typedef struct tu_timer_s
{
    timer_handler_cb_t timer_handler_cb;
    void * timer_handler_arg;
    int fd_index;
    bool continious;
    bool in_use;
} tu_timer_t;


typedef struct tu_evtimer_s
{
    struct event_base *base;
    struct event *evtimer;
    timer_handler_cb_t timer_handler_cb;
    void * timer_handler_arg;
    int fd;
    bool continious;
    bool in_use;
} tu_evtimer_t;



/******************************************************************************
 * Function Prototypes
 *****************************************************************************/
int tu_set_timer(tu_timer_t * timer, uint64_t milliseconds, bool continious, timer_handler_cb_t timer_handler_cb, void * timer_handler_arg);
void tu_kill_timer(tu_timer_t * timer);
int tu_set_timer_realtime(tu_timer_t * timer, uint64_t milliseconds, bool continious, timer_handler_cb_t timer_handler_cb, void * timer_handler_arg);
int tu_reset_timer(tu_timer_t * timer,uint64_t milliseconds,bool continious);

//int tu_set_evtimer(tu_evtimer_t *timer, uint64_t milliseconds, bool continious, evtimer_handler_cb_t cb , void *arg);
//void tu_kill_evtimer(tu_evtimer_t *timer);
//int tu_reset_evtimer(tu_evtimer_t *timer,uint64_t milliseconds);
int tu_set_evtimer(tu_evtimer_t * timer, uint64_t milliseconds, bool continious, timer_handler_cb_t timer_handler_cb, void * timer_handler_arg);
void tu_kill_evtimer(tu_evtimer_t * timer);
int tu_set_evtimer_realtime(tu_evtimer_t * timer, uint64_t milliseconds, bool continious, timer_handler_cb_t timer_handler_cb, void * timer_handler_arg);
int tu_reset_evtimer(tu_evtimer_t * timer,uint64_t milliseconds,bool continious);
tu_evtimer_t * tu_evtimer_new(struct event_base *base);
void tu_evtimer_free(tu_evtimer_t * evtimer);




#endif

