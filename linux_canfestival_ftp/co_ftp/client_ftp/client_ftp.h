/**
 * @file client_ftp.h
 * @author Leo-jiahao (leo884823525@gmail.com)
 * @brief 作为客户端，发送文件
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
#ifndef __CLIENT_FTP_H__
#define __CLIENT_FTP_H__

#include <stdint.h>
#include <stdio.h>

#include "data.h"


#define FTP_LOG_ENABLE   1

#define FTP_MSG(...) printf(__VA_ARGS__)

#define FTP_FILE_NAME_MAX_SIZE      32
#define FTP_CO_DATA_OBJ_INDEX       (0x3000)

typedef void (*ftp_tell_callback)(int tell);

/**/
int client_ftp(CO_Data *d, uint8_t nodeId, char *filename, char *buffer, int length, ftp_tell_callback callback);

#endif // __CLIENT_FTP_H__

