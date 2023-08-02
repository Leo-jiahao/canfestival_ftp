/**
 * @file stm32f405_demo1_can.c
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
#include "stm32f405_demo1_can.h"

#include <string.h>
typedef struct 
{
  uint32_t   SJW;
  uint32_t   BS1;
  uint32_t   BS2;
  uint32_t	 PreScale;
} CAN_BaudRate_TypeDef;

CAN_HandleTypeDef hcan1;
CAN_HandleTypeDef hcan2;

/**
 * @brief CAN波特率配置数组
 * 
 */
static const CAN_BaudRate_TypeDef  CAN_BaudRateInitTab[]= 
{
  {CAN_SJW_1TQ, CAN_BS1_6TQ, CAN_BS2_7TQ, 150},   // 20K
  {CAN_SJW_1TQ, CAN_BS1_6TQ, CAN_BS2_7TQ, 60},    // 50K
  {CAN_SJW_1TQ, CAN_BS1_6TQ, CAN_BS2_7TQ, 30},    // 100K
  {CAN_SJW_1TQ, CAN_BS1_9TQ, CAN_BS2_4TQ, 12},    // 250K
  {CAN_SJW_1TQ, CAN_BS1_9TQ, CAN_BS2_4TQ, 6},     // 500K
  {CAN_SJW_1TQ, CAN_BS1_9TQ, CAN_BS2_4TQ, 3},     // 1M
    
};

static uint32_t HAL_RCC_CAN1_CLK_ENABLED=0;
static BSP_CANx_CallBack  BSP_CAN1_Callback = NULL;
static BSP_CANx_CallBack  BSP_CAN2_Callback = NULL;

//CAN1中断服务函数
void CAN1_RX0_IRQHandler(void)
{
    HAL_CAN_IRQHandler(&hcan1);
}
//CAN2中断服务函数
void CAN2_RX0_IRQHandler(void)
{
    HAL_CAN_IRQHandler(&hcan2);
}

static bool __CAN_Init(CAN_HandleTypeDef *hcan, CANBRate_TypeDef brate, uint32_t id)
{
	uint16_t filter_id;
    hcan->Init.Mode = CAN_MODE_NORMAL;                 //回环测试/普通
    hcan->Init.AutoRetransmission = ENABLE;                 //自动重传 需要FD

    hcan->Init.Prescaler = CAN_BaudRateInitTab[brate].PreScale;				//分频系数(Fdiv)为brp+1
    hcan->Init.SyncJumpWidth = CAN_BaudRateInitTab[brate].SJW;			//重新同步跳跃宽度(Tsjw)为tsjw+1个时间单位 CAN_SJW_1TQ~CAN_SJW_4TQ
    hcan->Init.TimeSeg1 = CAN_BaudRateInitTab[brate].BS1;					//tbs1范围CAN_BS1_1TQ~CAN_BS1_16TQ
    hcan->Init.TimeSeg2 = CAN_BaudRateInitTab[brate].BS2;					//tbs2范围CAN_BS2_1TQ~CAN_BS2_8TQ
	
    hcan->Init.TimeTriggeredMode = DISABLE;	//非时间触发通信模式 
    hcan->Init.AutoBusOff = DISABLE;			//软件自动离线管理
    hcan->Init.AutoWakeUp = DISABLE;			//睡眠模式通过软件唤醒(清除CAN->MCR的SLEEP位)
    hcan->Init.AutoRetransmission = ENABLE;	//禁止报文自动传送 
    hcan->Init.ReceiveFifoLocked = DISABLE;	//报文不锁定,新的覆盖旧的 
    hcan->Init.TransmitFifoPriority = DISABLE;	//优先级由报文标识符决定 
	
    if(HAL_CAN_Init(hcan) != HAL_OK) 		            //初始化FDCAN
    {
      return false;
    }
	filter_id = id;
    //配置RX滤波器   
    CAN_FilterTypeDef CANx_RXFilter={0,};
	if(hcan == &hcan1){
		CANx_RXFilter.SlaveStartFilterBank = 0;
	    CANx_RXFilter.FilterBank = 0;
		if(filter_id == 0){
			CANx_RXFilter.FilterMode = CAN_FILTERMODE_IDMASK;//屏蔽位模式
		}else{
			CANx_RXFilter.FilterMode = CAN_FILTERMODE_IDLIST;//通过ID列表模式
		}
		CANx_RXFilter.FilterScale = CAN_FILTERSCALE_16BIT;//32位 
		CANx_RXFilter.FilterIdHigh = (filter_id<<5);//32位ID
		CANx_RXFilter.FilterIdLow = 0;
		CANx_RXFilter.FilterMaskIdHigh = 0;//32位MASK
		CANx_RXFilter.FilterMaskIdLow = 0;
		CANx_RXFilter.FilterActivation = ENABLE;//激活过滤器
		CANx_RXFilter.FilterFIFOAssignment = CAN_FilterFIFO0;//过滤器关联到FIFO0
	}else{
		CANx_RXFilter.SlaveStartFilterBank = 14;
		CANx_RXFilter.FilterBank = 14;
		CANx_RXFilter.FilterMode = CAN_FILTERMODE_IDMASK;//屏蔽位模式
		CANx_RXFilter.FilterScale = CAN_FILTERSCALE_32BIT;//32位 
		CANx_RXFilter.FilterIdHigh = 0x0000;//32位ID
		CANx_RXFilter.FilterIdLow = 0x0000;
		CANx_RXFilter.FilterMaskIdHigh = 0x0000;//32位MASK
		CANx_RXFilter.FilterMaskIdLow = 0x0000;
		CANx_RXFilter.FilterActivation = ENABLE;//激活过滤器
		CANx_RXFilter.FilterFIFOAssignment = CAN_FilterFIFO0;//过滤器关联到FIFO0
	
	}


    if(HAL_CAN_ConfigFilter(hcan, &CANx_RXFilter) != HAL_OK)//滤波器初始化
    {
      return false;
    }	

    HAL_CAN_Start(hcan);  //开启CAN
    
    HAL_CAN_ActivateNotification(hcan,CAN_IT_RX_FIFO0_MSG_PENDING);//待处理CAN消息中断
    return true;

}

/**
 * @brief FIFO0回调函数 接收
 * 
 * @param hcan 
 */
void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan)
{
  CAN_RxHeaderTypeDef CANx_RxHeader;
  CANRxMSG_TypeDef rmsg={0,};
	uint8_t rxdata[8];
	if(hcan == &hcan1)
	{
		HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO0, &CANx_RxHeader, rxdata);
		rmsg.RTR = CANx_RxHeader.RTR;
		rmsg.IDE = CANx_RxHeader.IDE;
		rmsg.cob_id = CANx_RxHeader.StdId;
		rmsg.length = CANx_RxHeader.DLC;
		memcpy(rmsg.data,rxdata,rmsg.length);
		
		if(BSP_CAN1_Callback)
		{
			BSP_CAN1_Callback(&rmsg);
		}
		HAL_CAN_ActivateNotification(hcan,CAN_IT_RX_FIFO0_MSG_PENDING);
		
	}
	if(hcan == &hcan2)
	{
		HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO0, &CANx_RxHeader, rxdata);
    rmsg.RTR = CANx_RxHeader.RTR;
    rmsg.IDE = CANx_RxHeader.IDE;
		rmsg.cob_id = CANx_RxHeader.StdId;
		rmsg.length = CANx_RxHeader.DLC;
		memcpy(rmsg.data,rxdata,rmsg.length);
		if(BSP_CAN2_Callback)
		{
			BSP_CAN2_Callback(&rmsg);
		}
		HAL_CAN_ActivateNotification(hcan,CAN_IT_RX_FIFO0_MSG_PENDING);
	}
	
}
/**
 * @brief 初始化
 * 
 * @param brate 
 * @param id 
 * @return true 
 * @return false 
 */
bool BSP_CAN1_Init(CANBRate_TypeDef brate, uint32_t id)
{
    BSP_CAN1_Callback = NULL;
    hcan1.Instance = CAN1;
    return __CAN_Init(&hcan1, brate, id);
}
/**
 * @brief 
 * 
 * @return bool 
 */
bool BSP_CAN1_DeInit( void )
{
  if(HAL_CAN_DeInit(&hcan1) != HAL_OK)
  {
    return false;
  }
  return true;
}

/**
 * @brief 
 * 
 * @param callback 
 * @return bool 
 */
bool  BSP_CAN1_SetRxCallBack(BSP_CANx_CallBack callback)
{
  BSP_CAN1_Callback = callback;
  return true;
}
/**
 * @brief 
 * 
 * @param brate 
 * @return bool 
 */
bool BSP_CAN2_Init(CANBRate_TypeDef brate, uint32_t id)
{
    BSP_CAN2_Callback = NULL;
    hcan2.Instance = CAN2;
    return __CAN_Init(&hcan2, brate, id);
}
/**
 * @brief 
 * 
 * @return bool 
 */
bool BSP_CAN2_DeInit( void )
{
  if(HAL_CAN_DeInit(&hcan2) != HAL_OK)
  {
    return false;
  }
  return true;
}
/**
 * @brief 
 * 
 * @param callback 
 * @return bool 
 */
bool  BSP_CAN2_SetRxCallBack(BSP_CANx_CallBack callback)
{
  BSP_CAN2_Callback = callback;
  return true;
}



void HAL_CAN_MspInit(CAN_HandleTypeDef* hcan)
{
    GPIO_InitTypeDef GPIO_Initure={0};                 
    
	if(hcan->Instance == CAN1)
	{
		HAL_RCC_CAN1_CLK_ENABLED++;
    if(HAL_RCC_CAN1_CLK_ENABLED==1){
      __HAL_RCC_CAN1_CLK_ENABLE();
    }
		__HAL_RCC_GPIOB_CLK_ENABLE();
		                
	
		GPIO_Initure.Pin = CAN1_TX_PIN|CAN1_RX_PIN;   //PB9,8
		GPIO_Initure.Mode = GPIO_MODE_AF_PP;          //推挽复用
		GPIO_Initure.Pull = GPIO_PULLUP;              //上拉
		GPIO_Initure.Speed = GPIO_SPEED_FREQ_VERY_HIGH;         //快速
		GPIO_Initure.Alternate = GPIO_AF9_CAN1;       //复用为CAN1
		HAL_GPIO_Init(CAN1_TX_PORT,&GPIO_Initure);         //初始化
		
		HAL_NVIC_SetPriority(CAN1_RX0_IRQn,MASTER_CAN1_PRIORITY,SLAVA_CAN1_PRIORITY);
		HAL_NVIC_EnableIRQ(CAN1_RX0_IRQn);
	}
	else if(hcan->Instance == CAN2)
	{
		
		__HAL_RCC_CAN2_CLK_ENABLE();               
		HAL_RCC_CAN1_CLK_ENABLED++;
		if(HAL_RCC_CAN1_CLK_ENABLED == 1){
      __HAL_RCC_CAN1_CLK_ENABLE();
    }
		
		__HAL_RCC_GPIOB_CLK_ENABLE();
		GPIO_Initure.Pin = CAN2_TX_PIN|CAN2_RX_PIN;   //
		GPIO_Initure.Mode = GPIO_MODE_AF_PP;          //推挽复用
		GPIO_Initure.Pull = GPIO_PULLUP;              //上拉
		GPIO_Initure.Speed = GPIO_SPEED_FREQ_VERY_HIGH;         //快速
		GPIO_Initure.Alternate = GPIO_AF9_CAN2;       //复用
		HAL_GPIO_Init(CAN2_TX_PORT,&GPIO_Initure);         //初始化
		
		HAL_NVIC_SetPriority(CAN2_RX0_IRQn,MASTER_CAN2_PRIORITY,SLAVA_CAN2_PRIORITY);
		HAL_NVIC_EnableIRQ(CAN2_RX0_IRQn);
	}
}

//此函数会被CANx_Mode_Init中的HAL_CAN_DeInit调用
void HAL_CAN_MspDeInit(CAN_HandleTypeDef* hcan)
{    
	if(hcan->Instance == CAN1)
	{
    HAL_RCC_CAN1_CLK_ENABLED--;
    if(HAL_RCC_CAN1_CLK_ENABLED==0){
      __HAL_RCC_CAN1_CLK_DISABLE();
    }

    /**CAN1 GPIO Configuration
    PB8     ------> CAN1_RX
    PB9     ------> CAN1_TX
    */
    HAL_GPIO_DeInit(CAN1_TX_PORT, CAN1_TX_PIN|CAN1_RX_PIN);

    /* CAN1 interrupt Deinit */
    HAL_NVIC_DisableIRQ(CAN1_RX0_IRQn);
	}
	if(hcan->Instance == CAN2)
	{
    __HAL_RCC_CAN2_CLK_DISABLE();
    HAL_RCC_CAN1_CLK_ENABLED--;
    if(HAL_RCC_CAN1_CLK_ENABLED==0){
      __HAL_RCC_CAN1_CLK_DISABLE();
    }

    /**CAN2 GPIO Configuration
    PB12     ------> CAN2_RX
    PB13     ------> CAN2_TX
    */
    HAL_GPIO_DeInit(CAN2_TX_PORT, CAN2_TX_PIN|CAN2_RX_PIN);

    /* CAN2 interrupt Deinit */
    HAL_NVIC_DisableIRQ(CAN2_RX0_IRQn);
	}
}
/**
 * @brief 
 * 
 * @param id 
 * @param pdata 
 * @param length 
 * @return bool 
 */
bool  BSP_CAN1_Transmit(uint16_t id, uint8_t *pdata, uint8_t length)
{
  uint32_t TxMailbox;
  uint32_t timeout = 0;
  CAN_TxHeaderTypeDef tmsg;
  tmsg.StdId = id;
  tmsg.ExtId = 0;
  tmsg.IDE = CAN_ID_STD;
  tmsg.RTR = CAN_RTR_DATA;
  tmsg.DLC = length;
  tmsg.TransmitGlobalTime = DISABLE;

  while(HAL_CAN_GetTxMailboxesFreeLevel(&hcan1) < 3){
    if(timeout >= CAN_TRANS_TIMEOUT){
      return false;
    }
    timeout++;
  }

  if(HAL_CAN_AddTxMessage(&hcan1, &tmsg, pdata, &TxMailbox) != HAL_OK)//发送
	{
		return false;
	}
  return true;
}
/**
 * @brief 直接发送帧
 * 
 * @param tmsg 
 * @param pdata 
 * @param length 
 * @return bool 
 */
bool  BSP_CAN1_Transmit_frame(CAN_TxHeaderTypeDef *tmsg, uint8_t *pdata, uint8_t length)
{
  uint32_t TxMailbox;
  uint32_t timeout = 0;

  while(HAL_CAN_GetTxMailboxesFreeLevel(&hcan1) < 3){
    if(timeout >= CAN_TRANS_TIMEOUT){
      return false;
    }
    timeout++;
  }

  if(HAL_CAN_AddTxMessage(&hcan1, tmsg, pdata, &TxMailbox) != HAL_OK)//发送
	{
		return false;
	}
  return true;
}
/**
 * @brief 
 * 
 * @param id 
 * @param pdata 
 * @param length 
 * @return bool 
 */
bool  BSP_CAN2_Transmit(uint16_t id, uint8_t *pdata, uint8_t length)
{
  uint32_t TxMailbox;
  uint32_t timeout = 0;
  CAN_TxHeaderTypeDef tmsg;
  tmsg.StdId = id;
  tmsg.ExtId = 0;
  tmsg.IDE = CAN_ID_STD;
  tmsg.RTR = CAN_RTR_DATA;
  tmsg.DLC = length;
  tmsg.TransmitGlobalTime = DISABLE;
  while(HAL_CAN_GetTxMailboxesFreeLevel(&hcan2) < 3){
    if(timeout >= CAN_TRANS_TIMEOUT){
      return false;
    }
    timeout++;
  }
  if(HAL_CAN_AddTxMessage(&hcan2, &tmsg, pdata, &TxMailbox) != HAL_OK)//发送
	{
		return false;
	}
  return true;
}
/**
 * @brief 直接发送帧
 * 
 * @param tmsg 
 * @param pdata 
 * @param length 
 * @return bool 
 */
bool  BSP_CAN2_Transmit_frame(CAN_TxHeaderTypeDef *tmsg, uint8_t *pdata, uint8_t length)
{
  uint32_t TxMailbox;
  uint32_t timeout = 0;

  while(HAL_CAN_GetTxMailboxesFreeLevel(&hcan2) < 3){
    if(timeout >= CAN_TRANS_TIMEOUT){
      return false;
    }
    timeout++;
  }

  if(HAL_CAN_AddTxMessage(&hcan2, tmsg, pdata, &TxMailbox) != HAL_OK)//发送
	{
		return false;
	}
  return true;
}


