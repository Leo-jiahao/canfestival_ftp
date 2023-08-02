/**
 * @file driver_timer.c
 * @author Leo-jiahao (leo884823525@gmail.com)
 * @brief 创建一个定时器，通过互斥信号量管理定时器触发后的执行任务。
 * 该执行任务是CanFestival的协议接口，所以必须包括协议的timer.h，并调用
 * @version 0.1
 * @date 2023-08-01
 * 
 * @copyright 
 * <one line to give the program's name and a brief idea of what it does.>
 * Copyright (C) <2023>  <Leo-jiahao> 
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 * 
 */
#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 200809L
#endif
#include <stdlib.h>

#include <sys/time.h>
#include <pthread.h> 
#include <signal.h>

#include "applicfg.h"
#include "timer.h"

static pthread_mutex_t CanFestival_mutex = PTHREAD_MUTEX_INITIALIZER;

static struct timeval last_sig;

static timer_t timer;

void TimerCleanup(void)
{
	/* only used in realtime apps */
}

void EnterMutex(void)
{
	if(pthread_mutex_lock(&CanFestival_mutex)) {
		fprintf(stderr, "pthread_mutex_lock() failed\n");
	}
}

void LeaveMutex(void)
{
	if(pthread_mutex_unlock(&CanFestival_mutex)) {
		fprintf(stderr, "pthread_mutex_unlock() failed\n");
	}
}

void timer_notify(sigval_t val)
{
	if(gettimeofday(&last_sig,NULL)) {
		perror("gettimeofday()");
	}
	EnterMutex();
	TimeDispatch();
	LeaveMutex();

}

void TimerInit(void)
{
	struct sigevent sigev;

	// Take first absolute time ref.
	if(gettimeofday(&last_sig,NULL)){
		perror("gettimeofday()");
	}

#if defined(__UCLIBC__)
	int ret;
	ret = timer_create(CLOCK_PROCESS_CPUTIME_ID, NULL, &timer);
	signal(SIGALRM, timer_notify);
#else
	memset (&sigev, 0, sizeof (struct sigevent));
	sigev.sigev_value.sival_int = 0;
	sigev.sigev_notify = SIGEV_THREAD;
	sigev.sigev_notify_attributes = NULL;
	sigev.sigev_notify_function = timer_notify;

	if(timer_create (CLOCK_REALTIME, &sigev, &timer)) {
		perror("timer_create()");
	}
#endif
}

void StopTimerLoop(TimerCallback_t exitfunction)
{
	EnterMutex();
	if(timer_delete (timer)) {
		perror("timer_delete()");
	}
	exitfunction(NULL,0);
	LeaveMutex();
}

void StartTimerLoop(TimerCallback_t init_callback)
{
	EnterMutex();
	// At first, TimeDispatch will call init_callback.
	SetAlarm(NULL, 0, init_callback, 0, 0);
	LeaveMutex();
}

void WaitReceiveTaskEnd(TASK_HANDLE *Thread)
{
	if(pthread_cancel(*Thread)) {
		perror("pthread_cancel()");
	}
	if(pthread_join(*Thread, NULL)) {
		perror("pthread_join()");
	}
}

#define maxval(a,b) ((a>b)?a:b)
void setTimer(TIMEVAL value)
{
//	printf("setTimer(TIMEVAL value=%d)\n", value);
	// TIMEVAL is us whereas setitimer wants ns...
	long tv_nsec = 1000 * (maxval(value,1)%1000000);
	time_t tv_sec = value/1000000;
	struct itimerspec timerValues;
	timerValues.it_value.tv_sec = tv_sec;
	timerValues.it_value.tv_nsec = tv_nsec;
	timerValues.it_interval.tv_sec = 0;
	timerValues.it_interval.tv_nsec = 0;

 	timer_settime (timer, 0, &timerValues, NULL);
}

TIMEVAL getElapsedTime(void)
{
	struct timeval p;
	if(gettimeofday(&p,NULL)) {
		perror("gettimeofday()");
	}
//	printf("getCurrentTime() return=%u\n", p.tv_usec);
	return (p.tv_sec - last_sig.tv_sec)* 1000000 + p.tv_usec - last_sig.tv_usec;
}

