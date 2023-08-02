/**
 * @file usr_can.c
 * @author Leo-jiahao (leo884823525@gmail.com)
 * @brief 
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
#include "usr_can.h"

#include <string.h>
#include "canfestival.h"
#include "main.h"
#include "objdict.h"
#include "server_ftp.h"
#include "stm32f405_demo1_flash.h"



/** @defgroup  Demo_Canopen  DEV_Modules can设备模块 Private FunctionPrototypes 私有函数
  * @{
  */

static void state_change(CO_Data* d)
{
    if(d->nodeState == Initialisation)
        DEMO_LOG( "Node state is now  : Initialisation\n");
    else if(d->nodeState == Disconnected)
        DEMO_LOG( "Node state is now  : Disconnected\n");
    else if(d->nodeState == Connecting)
        DEMO_LOG( "Node state is now  : Connecting\n");
    else if(d->nodeState == Preparing)
        DEMO_LOG( "Node state is now  : Preparing\n");
    else if(d->nodeState == Stopped)
        DEMO_LOG( "Node state is now  : Stopped\n");
    else if(d->nodeState == Operational)
        DEMO_LOG( "Node state is now  : Operational\n");
    else if(d->nodeState == Pre_operational)
        DEMO_LOG( "Node state is now  : Pre_operational\n");
    else if(d->nodeState == Unknown_state)
        DEMO_LOG( "Node state is now  : Unknown_state\n");
    else
        DEMO_LOG( "Error : unexpected node state\n");
}

static void TestSlave_post_emcy(CO_Data* d, UNS8 nodeID, UNS16 errCode, UNS8 errReg, const UNS8 errSpec[5])
{
	DEMO_LOG( "Slave received EMCY message. Node: %2.2x  ErrorCode: %4.4x  ErrorRegister: %2.2x\n", nodeID, errCode, errReg);
}

static void TestSlave_post_sync(CO_Data* d)
{
	DEMO_LOG("Slave received sync\n");
}

static void TestSlave_post_TPDO(CO_Data* d)
{
	DEMO_LOG("Slave received TPDO\n");
}

static UNS32 OD0x6000Callback_t(CO_Data* d, UNS16 wIndex, UNS8 bSubindex)
{
  DEMO_LOG("\r\nSlave received wIndex:0X%04X,subidnex:0X%02X callback:%s\n",wIndex,bSubindex,TestSlave_obj6000);
  return NULL;
}

 /**
  * @}
  */

/**
 * @brief 接收到一块数据后的回调函数
 * 
 * @param filename 
 * @param buffer 
 * @param buffer_size 
 * @param ftell 
 * @param file_size 
 * @return UNS32 
 */
UNS32 file_callback(char *filename, const char * buffer, size_t buffer_size, int ftell, size_t file_size)
{
  bool ret = false;
  if(ftell == 0){
      //擦除扇区
      ret = BSP_EraseSector(FLASH_SECTOR_11);
      if (ret == false){
        return FTP_FILE_ABORT;
      }

      ret = BSP_WriteByteFlash(ADDR_FLASH_SECTOR_11, (uint8_t *)buffer, buffer_size);
        /* code */
      if(ret == false){
        return FTP_FILE_ABORT;
      }

      return FTP_SUCCESSFUL;
  }else{
    if(ftell + buffer_size > ADDR_FLASH_SECTOR_END - ADDR_FLASH_SECTOR_11){
      return FTP_FILE_ABORT;
    }
    ret = BSP_WriteByteFlash(ADDR_FLASH_SECTOR_11+ftell, (uint8_t *)buffer, buffer_size);

    /*文件接收完成*/
    if(ftell + buffer_size == file_size){
		
      DEMO_LOG("File Received successfully: %d bytes\r\n",file_size);
    
	}
  }

  return FTP_SUCCESSFUL;
    
}
/** @defgroup  Demo_Canopen DEV_Modules can设备模块 Public Functions 共有函数
  * @{
  */

void canfestival_ftp_demo(void)
{
    s_BOARD board = {
      .baudrate = "1M",
      .busname = "CAN1",
    };
    /* 当前节点进入到初始化状态的回调函数,可以writeLocalDict配置本地的PDO等 */
    TestSlave_Data.initialisation = state_change;

    /* 节点进入preoperation的回调函数*/
    TestSlave_Data.preOperational = state_change;

    /* 节点进入operation的回调函数*/
    TestSlave_Data.operational = state_change;
    TestSlave_Data.stopped = state_change;

    /*节点接收到同步包后的回调,之后协议栈内部再处理*/
    TestSlave_Data.post_sync = TestSlave_post_sync;

    /*节点接收到同步包后post_sync先处理，在协议栈处理，最后调用。通常可以在一段PDO接收后进行某些判断*/
    TestSlave_Data.post_TPDO = TestSlave_post_TPDO;
    
    /*接收到紧急事件包之后回调*/
    TestSlave_Data.post_emcy = TestSlave_post_emcy;
    
	
    TimerInit();
    
    setNodeId(&TestSlave_Data,2);
    
    canOpen(&board, &TestSlave_Data);

    RegisterSetODentryCallBack(&TestSlave_Data, 0x6000, 0,OD0x6000Callback_t);

    setState(&TestSlave_Data, Initialisation);

    //初始化文件接收的回调函数
    server_tfp_init(file_callback);
    while (1)
    {
		HAL_Delay(100);
      /* code */
    }
    
}


