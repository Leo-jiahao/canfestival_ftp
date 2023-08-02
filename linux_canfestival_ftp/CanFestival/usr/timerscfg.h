/**
 * @file timerscfg.h
 * @author Leo-jiahao (leo884823525@gmail.com)
 * @brief ²Î¿¼Ô´ÂëÂ·¾¶\CanFestival-master\include\cm4\timerscfg.h
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
#ifndef __TIMERSCFG_H__
#define __TIMERSCFG_H__


#include <pthread.h>

/* Time unit : us */
/* Time resolution : 64bit (~584942 years) */
#define TIMEVAL unsigned long long
#define TIMEVAL_MAX ~(TIMEVAL)0
#define MS_TO_TIMEVAL(ms) ms*1000L
#define US_TO_TIMEVAL(us) us

#define TASK_HANDLE pthread_t


#endif /* __TIMERSCFG_H__ */


