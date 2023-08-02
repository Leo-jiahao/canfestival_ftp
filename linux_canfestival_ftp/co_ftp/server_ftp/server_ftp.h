/**
 * @file server_ftp.h
 * @author Leo-jiahao (leo884823525@gmail.com)
 * @brief 
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
#ifndef __FTP_H
#define __FTP_H

#include "data.h"


#define CO_FTP_LOG    0




#if CO_FTP_LOG == 1
#include "SEGGER_RTT.h"
#define FTP_MSG(...) SEGGER_RTT_printf(0, __VA_ARGS__)
#else
#define FTP_MSG(...) 
#endif


#define FTP_BLOCK_MAX_SIZE          1024
#define FTP_FILE_NAME_MAX_SIZE      32
#define FTP_CO_DATA_OBJ_INDEX       0x3000

#define FTP_LOG_ENABLE   1

#define FTP_SUCCESSFUL 	             0x00000000
#define FTP_BLOCK_RETRY              0x00000001
#define FTP_FILE_RETRY               0x00000002
#define FTP_FILE_ABORT               0x00000003

/**
 * @brief 文件数据块的回调函数，每接收到一块成功后执行一次
 * @return 
 * FTP_SUCCESSFUL
 * FTP_BLOCK_RETRY
 * FTP_FILE_RETRY
 * FTP_FILE_ABORT
 */
typedef UNS32 (*file_callback_t)(char *filename, const char * buffer, size_t buffer_size, int ftell, size_t file_size);

void server_tfp_init(file_callback_t callback);


extern ODCallback_t CO_Data_FTP_callbacks[7];

extern subindex CO_Data_FTP[7];

#endif  /* __FTP_H */
