/**
 * @file server_ftp.c
 * @author Leo-jiahao (leo884823525@gmail.com)
 * @brief 基于canopen定义一个文件传输协议
 * 0x6001h 0x00h subindex num：3
 * 0x6001h 0x01h file name(32字节长度的文件名)
 * 0x6001h 0x02h file size（4字节长度的文件大小）
 * 0x6001h 0x03h file buffer（文件存储地址）
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
#include <stdlib.h>

#include "server_ftp.h"

#include "canfestival.h"



#if FTP_LOG_ENABLE == 1

#    define FTP_LOG(step, str, val)            \
          FTP_MSG("FTP LOG:%s,%d : Step:%d, %s 0X%x \n",__FILE__, __LINE__,step, str, val);

#else

#    define FTP_LOG(step, str, val)            

#endif



typedef struct _FTP_Data_Def
{
    UNS8 CO_Data_highestSubIndex_obj;
    UNS8 CO_Data_FileName_obj[FTP_FILE_NAME_MAX_SIZE];
    UNS32 CO_Data_FileSize_obj;
    UNS32 CO_Data_BlockSize_obj;
    UNS32 CO_Data_BlockIndex_obj;
    UNS32 CO_Data_CurrentBlockSize_obj;
    UNS8 CO_Data_FileBuffer_obj[FTP_BLOCK_MAX_SIZE];

    uint32_t ftell;
    file_callback_t callback;
}ftp_t, *p_ftp;


static UNS32 __file_callback(char *filename, const char * buffer, size_t buffer_size, int ftell, size_t file_size);

static ftp_t ftp_s={
    .CO_Data_highestSubIndex_obj = 6,
    .CO_Data_FileName_obj = {0,},
    .CO_Data_FileSize_obj = NULL,
    .CO_Data_BlockIndex_obj = NULL,
    .CO_Data_BlockSize_obj = FTP_BLOCK_MAX_SIZE,
    .CO_Data_CurrentBlockSize_obj = NULL,
    .CO_Data_FileBuffer_obj = {0,},
    
    .callback = __file_callback,
};

static UNS32 FileNameCallback(CO_Data* d, UNS16 wIndex, UNS8 bSubindex);

static UNS32 FileSizeCallback(CO_Data* d, UNS16 wIndex, UNS8 bSubindex);

static UNS32 FileBufferCallback(CO_Data* d, UNS16 wIndex, UNS8 bSubindex);

ODCallback_t CO_Data_FTP_callbacks[7] = 
{
    NULL,
    FileNameCallback,
    FileSizeCallback,
    NULL,
    NULL,
    NULL,
    FileBufferCallback,
};
subindex CO_Data_FTP[7] = 
{
    { RO, uint8, sizeof (UNS8), (void*)&ftp_s.CO_Data_highestSubIndex_obj},
    { RW, visible_string, FTP_FILE_NAME_MAX_SIZE, (void*)ftp_s.CO_Data_FileName_obj },
    { RW, uint32, sizeof(UNS32), (void*)&ftp_s.CO_Data_FileSize_obj },
    { RO, uint32, sizeof(UNS32), (void*)&ftp_s.CO_Data_BlockSize_obj },
    { RO, uint32, sizeof(UNS32), (void*)&ftp_s.CO_Data_BlockIndex_obj },
    { RW, uint32, sizeof(UNS32), (void*)&ftp_s.CO_Data_CurrentBlockSize_obj },
    { RW, domain, FTP_BLOCK_MAX_SIZE, (void*)ftp_s.CO_Data_FileBuffer_obj },
};


static UNS32 __file_callback(char *filename, const char * buffer, size_t buffer_size, int ftell, size_t file_size)
{

    return FTP_SUCCESSFUL;
}
/**
 * @brief 收到文件名后的回调函数
 * 清空状态，block_index_obj,current_block_size_obj
 * 
 * @param d 
 * @param wIndex 
 * @param bSubindex 
 * @return UNS32 
 */
static UNS32 FileNameCallback(CO_Data* d, UNS16 wIndex, UNS8 bSubindex)
{
    ftp_s.CO_Data_BlockIndex_obj = 0;
    ftp_s.CO_Data_CurrentBlockSize_obj = 0;
    ftp_s.ftell = 0;
    FTP_LOG(1,ftp_s.CO_Data_FileName_obj, 0);
    return OD_SUCCESSFUL;
}
/**
 * @brief 收到文件大小后的回调函数
 * 
 * @param d 
 * @param wIndex 
 * @param bSubindex 
 * @return UNS32 
 */
static UNS32 FileSizeCallback(CO_Data* d, UNS16 wIndex, UNS8 bSubindex)
{
    FTP_LOG(2,"FILE SIZE", ftp_s.CO_Data_FileSize_obj);
    return OD_SUCCESSFUL;
}

/**
 * @brief 收到文件buff后的回调函数
 * 
 * @param d 
 * @param wIndex 
 * @param bSubindex 
 * @return UNS32 
 */
static UNS32 FileBufferCallback(CO_Data* d, UNS16 wIndex, UNS8 bSubindex)
{
    UNS32 ret;
    
    FTP_LOG(3,"Received Block Index", ftp_s.CO_Data_BlockIndex_obj);
    FTP_LOG(3,"Received Block Size", ftp_s.CO_Data_CurrentBlockSize_obj);
    FTP_LOG(3,"Ftell", ftp_s.ftell);

    ret = ftp_s.callback(
        (char *)ftp_s.CO_Data_FileName_obj, 
        (const char * )ftp_s.CO_Data_FileBuffer_obj, 
        ftp_s.CO_Data_CurrentBlockSize_obj,
        ftp_s.ftell,
        ftp_s.CO_Data_FileSize_obj);
    
    switch (ret)
    {
    case FTP_SUCCESSFUL:
        ftp_s.ftell += ftp_s.CO_Data_CurrentBlockSize_obj;
        ftp_s.CO_Data_BlockIndex_obj++;
        FTP_LOG(4, "User callback returned Success", ftp_s.ftell);
        if(ftp_s.ftell == ftp_s.CO_Data_FileSize_obj){
            FTP_LOG(5,"Received File OK, File size:", ftp_s.ftell);
        }
        break;
    case FTP_BLOCK_RETRY:
        FTP_LOG(4, "User callback returned Failure, BLOCK_RETRY", ftp_s.ftell);
        break;
    case FTP_FILE_RETRY:
        FTP_LOG(4, "User callback returned Failure, FILE_RETRY", ftp_s.ftell);
        ftp_s.CO_Data_BlockIndex_obj = 0;
        break;
    case FTP_FILE_ABORT:
        FTP_LOG(3, "User callback returned Failure, FILE_ABORT", ftp_s.ftell);
        //块index 0xFFFFFFFF作为文件依次中断标记
        ftp_s.CO_Data_BlockIndex_obj = 0XFFFFFFFF;
        break;
    default:
        break;
    }

    return OD_SUCCESSFUL;
}


void server_tfp_init(file_callback_t callback)
{
    if(callback)
        ftp_s.callback = callback;
}

