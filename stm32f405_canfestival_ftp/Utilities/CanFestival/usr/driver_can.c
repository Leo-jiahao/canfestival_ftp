/**
 * @file driver_can.c
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
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stddef.h>		/* for NULL */
#include <errno.h>

#include "config.h"

#include "stm32f405_demo1_can.h"


#ifndef __USE_MISC
#define __USE_MISC
#endif



#include "data.h"
#include "dcf.h"



#include "timers_driver.h"





#include "can_driver.h"


int canClose(CO_Data * d);

typedef struct
{
  int num;
  void* d;
  char used;
} CANPort; /* taken from drivers/unix.c */

static CANPort port[2] = {
  {
    .d = NULL,
    .num = 1,
    .used = NULL,
  },
  {
    .d = NULL,
    .num = 2,
    .used = NULL,
  }
};
/**
 * @brief 获取波特率
 * 
 * @param optarg 
 * @return CANBRate_TypeDef 
 */
static CANBRate_TypeDef TranslateBaudeRate(char* optarg){

	if(!strcmp( optarg, "1M")) return CAN_1M;
	if(!strcmp( optarg, "500K")) return CAN_500K;
	if(!strcmp( optarg, "250K")) return CAN_250K;
	if(!strcmp( optarg, "100K")) return CAN_100K;
	if(!strcmp( optarg, "50K")) return CAN_50K;
	if(!strcmp( optarg, "20K")) return CAN_20K;
	if(!strcmp( optarg, "none")) return CAN_ERR;
	return CAN_ERR;
}

static void CAN1_CallBack(CANRxMSG_TypeDef *rmsg)
{
    int i;

    Message rxm = {0};

    // Drop extended frames
    if(rmsg->IDE == CAN_ID_EXT)
      return;
    rxm.cob_id = rmsg->cob_id;
    if(rmsg->RTR == CAN_RTR_REMOTE)
      rxm.rtr = 1;

    rxm.len = rmsg->length;

    for(i=0 ; i<rxm.len ; i++)
        rxm.data[i] = rmsg->data[i];

    if(port[0].used){
       EnterMutex();
       canDispatch(port[0].d, &rxm);
       LeaveMutex();
    }
  
}

static void CAN2_CallBack(CANRxMSG_TypeDef *rmsg)
{
    int i;

    Message rxm = {0};

    // Drop extended frames
    if(rmsg->IDE == CAN_ID_EXT)
      return;
    rxm.cob_id = rmsg->cob_id;
    if(rmsg->RTR == CAN_RTR_REMOTE)
      rxm.rtr = 1;

    rxm.len = rmsg->length;

    for(i=0 ; i<rxm.len ; i++)
        rxm.data[i] = rmsg->data[i];

    if(port[1].used){
       EnterMutex();
       canDispatch(port[1].d, &rxm);
       LeaveMutex();
    }
}
/**
 * CAN open routine
 * @param board device name and baudrate
 * 
 * struct struct_s_BOARD {
 * char * busname;  
 * char * baudrate; 
 * };
 * @param d CAN object data
 * @return valid CAN_PORT pointer or NULL
 */
CAN_PORT canOpen(s_BOARD *board, CO_Data * d)
{
  CANPort *p_port = NULL;
  CANBRate_TypeDef baudeRate;
  bool ret = false;

    baudeRate  = TranslateBaudeRate(board->baudrate);
    if(baudeRate == CAN_ERR){
      MSG("Unknown baudrate: %s",board->baudrate);
      return NULL;
    }

    if(strcmp(board->busname,"CAN1") == 0){

      p_port = &port[0];

      ret = BSP_CAN1_Init(baudeRate, 0);

      BSP_CAN1_SetRxCallBack(CAN1_CallBack);

    }else if(strcmp(board->busname,"CAN2") == 0){

      p_port = &port[1];

      ret = BSP_CAN2_Init(baudeRate, 0);

      BSP_CAN2_SetRxCallBack(CAN2_CallBack);

    }else{

      MSG("Unknown bus name: %s",board->busname);

      return NULL;

    }
    if(ret == false){

      MSG("%s initialize baud rate %s error", board->busname, board->baudrate);

      canClose(d);

      return NULL;
    }

    d->canHandle = p_port;
    
    p_port->used = 1;
    
    p_port->d = d;

    return p_port;
}


/**
 * CAN send routine
 * @param port CAN port
 * @param m CAN message
 * @return success or error
 */
UNS8 canSend(CAN_PORT port, Message *m)
{

    CANPort *p_port = port;
    if(!p_port){
        return 1;
    }
    bool res;

    CAN_TxHeaderTypeDef TxMessage = {0};

    TxMessage.StdId = m->cob_id;

    TxMessage.IDE = CAN_ID_STD;

    if(m->rtr)
        TxMessage.RTR = CAN_RTR_REMOTE;

    else

        TxMessage.RTR = CAN_RTR_DATA;

    TxMessage.DLC = m->len;

    res = false;
    
    if(p_port->num == 1){

      res = BSP_CAN1_Transmit_frame(&TxMessage, m->data, m->len);

      
    }else if(p_port->num == 2){

      res = BSP_CAN2_Transmit_frame(&TxMessage, m->data, m->len);


    }

    if(res == false){
      
      return 0;
    }else 
      return 1;		// succesful
}

/**
 * @brief 
 * 
 * CAN Receiver Task
 * @param port CAN port
 */
void canReceiveLoop(CO_Data * d)
{
//    Message m;
//    int res;
//    struct can_frame frame;
//    while (((CANPort*)d->canHandle)->used) {
//        res = CAN_RECV(((CANPort*)d->canHandle)->fd, &frame, sizeof(frame), MSG_WAITALL);
//        if (res < 0)
//        {
//            fprintf (stderr, "Recv failed: %s\n", strerror (CAN_ERRNO (res)));
//            continue;
//        }

//        m.cob_id = frame.can_id & CAN_EFF_MASK;
//        m.len = frame.can_dlc;
//        if (frame.can_id & CAN_RTR_FLAG)
//            m.rtr = 1;
//        else
//            m.rtr = 0;
//        memcpy (m.data, frame.data, 8);   

//        EnterMutex();
//        canDispatch(d, &m);
//        LeaveMutex();
//    }
}



/**
 * CAN close routine
 * @param d CAN object data
 * @return success or error
 */
int canClose(CO_Data * d)
{
    if(d == NULL){
      return 0;
    }
    CANPort *p_port = d->canHandle;
    if(!p_port ){
        return 0;
    }

    p_port->used = 0;

    if(p_port->num == 1){

      BSP_CAN1_DeInit();

      BSP_CAN1_SetRxCallBack(NULL);

    }else if(p_port->num == 2){

      BSP_CAN2_DeInit();

      BSP_CAN2_SetRxCallBack(NULL);

    }
    d->canHandle = NULL;

    return 1;

}


/**
 * CAN change baudrate routine
 * @param port CAN port
 * @param baud baudrate
 * @return success or error
 */
UNS8 canChangeBaudRate(CAN_PORT port, char* baud)
{
	printf("canChangeBaudRate not yet supported by this driver\n");
	return 0;
}
