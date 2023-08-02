/**
 * @file client_ftp.c
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

#include <stdlib.h>

#include "client_ftp.h"
#include "canfestival.h"



/**
 * @brief 日志输出接口
 * 
 */
#if FTP_LOG_ENABLE == 1

#    define FTP_LOG(step, str, val)            \
          FTP_MSG("FTP LOG:%s,%d : Step:%d, %s 0X%x \n",__FILE__, __LINE__,step, str, val)

#else

#    define FTP_LOG(step, str, val)            

#endif


#define FTP_CO_DATA_OBJ_NAME_SUBINDEX       1
#define FTP_CO_DATA_OBJ_FSIZE_SUBINDEX      2
#define FTP_CO_DATA_OBJ_MBSIZE_SUBINDEX     3
#define FTP_CO_DATA_OBJ_BINDEX_SUBINDEX     4
#define FTP_CO_DATA_OBJ_CBSIZE_SUBINDEX     5
#define FTP_CO_DATA_OBJ_BUFFER_SUBINDEX     6

/**
 * @brief 私有函数声明
 * 
 * @param 
 */
static void SDOSendFile(CO_Data* d, UNS8 nodeId);

/**
 * @brief 定义发送文件的状态枚举
 * 
 */
typedef enum _CLIENT_FTP_TYPE
{
    CFTP_NONE = 0,
    CFTP_WRITE_FILE_NAME,
    CFTP_READ_BLOCK_MAX_SIZE,
    CFTP_WRITE_FILE_SIZE,
    CFTP_READ_BLOCK_INDEX,
    CFTP_WRITE_BLOCK_SIZE,
    CFTP_WRITING_BLOCK_BUFFER,
    CFTP_WRITE_FILE_FINISHED,
    CFTP_ERROR,

}c_ftp_status_t;


/**
 * @brief 定义发送文件的数据结构
 * 
 */
typedef struct _client_ftp
{
    /* data */

    uint32_t block_max_size;
    uint32_t block_index;
    c_ftp_status_t status;
    ftp_tell_callback ftell_callback;

    uint32_t block_num;//0到end，block index == block_max 时，应该写入remain_bytes
    uint32_t remain_bytes;

    char filename[FTP_FILE_NAME_MAX_SIZE];
    char *buffer;
    UNS32 length; 
    
}c_ftp_t, * p_c_ftp_t;


static c_ftp_t c_ftp = 
{

    .block_max_size = 0,
    .block_index = 0,
    .status = CFTP_NONE,
    .ftell_callback = NULL,
    
    .block_num = 0,
    .remain_bytes = 0,
    
    .filename = {0, },
    .buffer = NULL,
    .length = 0,
};

/* Callback function that check the write SDO demand */
static void CheckSDOAndContinue(CO_Data* d, UNS8 nodeId)
{
	UNS32 abortCode;
    UNS32 size;
    UNS32 data;
    switch (c_ftp.status)
    {
    case CFTP_NONE:
        /* code */
        break;
    case CFTP_WRITE_FILE_NAME:
    case CFTP_WRITE_FILE_SIZE:
    case CFTP_WRITE_BLOCK_SIZE:
        if(getWriteResultNetworkDict (d, nodeId, &abortCode) != SDO_FINISHED){
                FTP_LOG(c_ftp.status, "Failed getWriteResultNetworkDict. abortCode:",abortCode);
                c_ftp.status = CFTP_ERROR;
        }else{
            c_ftp.status++;
        }
        break;
    case CFTP_WRITING_BLOCK_BUFFER:
        /* code */
        if(getWriteResultNetworkDict (d, nodeId, &abortCode) != SDO_FINISHED){
            FTP_LOG(c_ftp.status, "Failed getWriteResultNetworkDict. abortCode:",abortCode);
            c_ftp.status = CFTP_ERROR;
        }else{
            if(c_ftp.block_index == c_ftp.block_num){
                c_ftp.status++;
            }else{
                c_ftp.status -= 2;
            }
        }

        break;
    case CFTP_READ_BLOCK_MAX_SIZE:
        if(getReadResultNetworkDict (d, nodeId, &data, &size, &abortCode) != SDO_FINISHED){
            FTP_LOG(c_ftp.status, "Failed getReadResultNetworkDict. abortCode:",abortCode);
            c_ftp.status = CFTP_ERROR;
        }else{
            c_ftp.block_max_size = data;
            c_ftp.status++;
        }
        break;
    case CFTP_READ_BLOCK_INDEX:
        if(getReadResultNetworkDict (d, nodeId, &data, &size, &abortCode) != SDO_FINISHED){
            FTP_LOG(c_ftp.status, "Failed getReadResultNetworkDict. abortCode:",abortCode);
            c_ftp.status = CFTP_ERROR;
        }else{
            c_ftp.block_index = data;
            if(c_ftp.block_index == 0xFFFFFFFF){
                c_ftp.status = CFTP_ERROR;
            }else{
                c_ftp.status++;
            }

            if(c_ftp.ftell_callback){
                c_ftp.ftell_callback((c_ftp.block_index + 1) * c_ftp.block_max_size > c_ftp.length ? c_ftp.length : (c_ftp.block_index + 1) * c_ftp.block_max_size);
            }
            
        }
        break;
    
    case CFTP_WRITE_FILE_FINISHED:
        
        break;
    case CFTP_ERROR:

        break;
    default:
        break;
    }
	
	/* Finalise last SDO transfer with this node */
	closeSDOtransfer(d, nodeId, SDO_CLIENT);

	SDOSendFile(d, nodeId);
}

/**
 * @brief SDO 发送文件
 * 
 * @param d 
 * @param nodeId 
 */
static void SDOSendFile(CO_Data* d, UNS8 nodeId)
{
	UNS8 res;
	switch(c_ftp.status){
		case CFTP_NONE: 
            break;
        case CFTP_WRITE_FILE_NAME: 
		{	/*写文件名*/
			FTP_LOG(c_ftp.status,"Write file name", 0);
			res = writeNetworkDictCallBack (d, /*CO_Data* d*/
					nodeId, /*UNS8 nodeId*/
					FTP_CO_DATA_OBJ_INDEX, /*UNS16 index*/
					FTP_CO_DATA_OBJ_NAME_SUBINDEX, /*UNS8 subindex*/
					strlen(c_ftp.filename)+1, /*UNS8 count*/
					visible_string, /*UNS8 dataType*/
					c_ftp.filename,/*void *data*/
					CheckSDOAndContinue, /*SDOCallback_t Callback*/
                    1); /* use block mode */
		}			
		break;

		case CFTP_READ_BLOCK_MAX_SIZE: 
		{	/*读块最大size*/
			FTP_LOG(c_ftp.status,"Read Block Max size", 0);
			res = readNetworkDictCallback (d, /*CO_Data* d*/
					nodeId, /*UNS8 nodeId*/
					FTP_CO_DATA_OBJ_INDEX, /*UNS16 index*/
					FTP_CO_DATA_OBJ_MBSIZE_SUBINDEX, /*UNS8 subindex*/
					0, /*UNS8 dataType*/
					CheckSDOAndContinue, /*SDOCallback_t Callback*/
                    0); /* use block mode */
		}
		break;
					
		case CFTP_WRITE_FILE_SIZE: 
		{	
			/*写文件大小*/
			FTP_LOG(c_ftp.status,"Write File size:", c_ftp.length);
			res = writeNetworkDictCallBack (d, /*CO_Data* d*/
					nodeId, /*UNS8 nodeId*/
					FTP_CO_DATA_OBJ_INDEX, /*UNS16 index*/
					FTP_CO_DATA_OBJ_FSIZE_SUBINDEX, /*UNS8 subindex*/
					4, /*UNS8 count*/
					0, /*UNS8 dataType*/
					&c_ftp.length,/*void *data*/
					CheckSDOAndContinue, /*SDOCallback_t Callback*/
                    0); /* use block mode */
            c_ftp.block_num = c_ftp.length / c_ftp.block_max_size;
            c_ftp.remain_bytes = c_ftp.length % c_ftp.block_max_size;
		}
		break;

        case CFTP_READ_BLOCK_INDEX:
        {
            /*读目标块INDEX*/
			FTP_LOG(c_ftp.status,"Read Next Block Index", 0);
			res = readNetworkDictCallback (d, /*CO_Data* d*/
					nodeId, /*UNS8 nodeId*/
					FTP_CO_DATA_OBJ_INDEX, /*UNS16 index*/
					FTP_CO_DATA_OBJ_BINDEX_SUBINDEX, /*UNS8 subindex*/
					0, /*UNS8 dataType*/
					CheckSDOAndContinue, /*SDOCallback_t Callback*/
                    0); /* use block mode */
        }
        break;
        case CFTP_WRITE_BLOCK_SIZE:
        {
            UNS32 count;
            if(c_ftp.block_index < c_ftp.block_num){
                count = c_ftp.block_max_size;
                
            }else{
                count = c_ftp.remain_bytes;

            }
            FTP_LOG(c_ftp.status,"Write Next Block Size", count);
            res = writeNetworkDictCallBack (d, /*CO_Data* d*/
					nodeId, /*UNS8 nodeId*/
					FTP_CO_DATA_OBJ_INDEX, /*UNS16 index*/
					FTP_CO_DATA_OBJ_CBSIZE_SUBINDEX, /*UNS8 subindex*/
					4, /*UNS32 count*/
					domain, /*UNS8 dataType*/
					&count,/*void *data*/
					CheckSDOAndContinue, /*SDOCallback_t Callback*/
                    0); /* use block mode */
        }
        break;

				
		case CFTP_WRITING_BLOCK_BUFFER:
		{	
			/*写目标块INDEX的文件数据*/
            void *data;
            UNS32 count;
			FTP_LOG(c_ftp.status,"Write Next Block Data", c_ftp.block_index);
			if(c_ftp.block_index < c_ftp.block_num){
                count = c_ftp.block_max_size;
                
            }else{
                count = c_ftp.remain_bytes;

            }

            data = (void *)c_ftp.buffer + c_ftp.block_index * c_ftp.block_max_size;
			res = writeNetworkDictCallBack (d, /*CO_Data* d*/
					nodeId, /*UNS8 nodeId*/
					FTP_CO_DATA_OBJ_INDEX, /*UNS16 index*/
					FTP_CO_DATA_OBJ_BUFFER_SUBINDEX, /*UNS8 subindex*/
					count, /*UNS32 count*/
					domain, /*UNS8 dataType*/
					data,/*void *data*/
					CheckSDOAndContinue, /*SDOCallback_t Callback*/
                    1); /* use block mode */
		}	
		

		case CFTP_WRITE_FILE_FINISHED: 

            break;
        case CFTP_ERROR: 

            break;
		default :

            break;
			
	}

    if(res){
        FTP_LOG(c_ftp.status,"SDO Write or Read Err", 0);
        c_ftp.status = CFTP_ERROR;  
    }

}


/**
 * @brief 客户端向服务端发送文件,不可重入,再上一次完成之前，再次被调用会导致无效
 * 
 * @param nodeId 服务端ID
 * @param filename 文件名,字符串常量，不应该被外界释放
 * @param buffer 文件buffer
 * @param length 文件字节大小
 * @param callback tell追踪发送成功的数据量,每次文件数据发送成功一次会被执行一次
 * @return int 
 * 0 ok
 * 
 */
int client_ftp(CO_Data *d, uint8_t nodeId, char *filename, char *buffer, int length, ftp_tell_callback callback)
{

    if(c_ftp.status == CFTP_NONE ||
    c_ftp.status == CFTP_WRITE_FILE_FINISHED ||
    c_ftp.status == CFTP_ERROR){

    }else{

        FTP_LOG(CFTP_ERROR, "FTP Status Failed",0);
        return 1;

    }

    if(d == NULL || filename == NULL || buffer == NULL){

        FTP_LOG(CFTP_ERROR, "FTP Parameters Failed",0);
        return 1;

    }

    memset(&c_ftp, 0, sizeof(c_ftp));

    c_ftp.buffer = buffer;

    strncpy(c_ftp.filename, filename, FTP_FILE_NAME_MAX_SIZE-1);

    c_ftp.length = length;
    c_ftp.ftell_callback = callback;

    c_ftp.status = CFTP_WRITE_FILE_NAME;
    
    SDOSendFile(d, nodeId);

    if(c_ftp.status == CFTP_WRITE_FILE_FINISHED){
        return 0;
    }else{
        return 1;
    }
}


