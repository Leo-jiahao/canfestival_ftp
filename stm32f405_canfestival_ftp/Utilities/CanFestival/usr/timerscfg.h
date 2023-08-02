/**
 * @file timerscfg.h
 * @author Leo-jiahao (leo884823525@gmail.com)
 * @brief 参考源码路径\CanFestival-master\include\cm4\timerscfg.h
 * @version 0.1
 * @date 2023-08-02
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


// Whatever your microcontroller, the timer wont work if
// TIMEVAL is not at least on 32 bits
#define TIMEVAL UNS32

// using 16 bits timer
#define TIMEVAL_MAX 0xFFFF

// The timer is incrementing every 10 us.
#define MS_TO_TIMEVAL(ms) ((ms) * 100)
#define US_TO_TIMEVAL(us) ((us) / 10)

#define TASK_HANDLE void *

#endif /* __TIMERSCFG_H__ */


