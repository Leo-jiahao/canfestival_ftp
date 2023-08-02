/**
 * @file stm32f405_demo1_can.h
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
#ifndef __STM32F405_DEMO1_CAN_H__
#define __STM32F405_DEMO1_CAN_H__
#include "stm32f4xx_hal.h"
#include <stdbool.h>
#define CAN1_TX_PORT              GPIOB
#define CAN1_TX_CLK_ENABLE()      __HAL_RCC_GPIOB_CLK_ENABLE()
#define CAN1_TX_CLK_DISABLE()     __HAL_RCC_GPIOB_CLK_DISABLE()

#define CAN1_RX_PORT              GPIOB
#define CAN1_RX_CLK_ENABLE()      __HAL_RCC_GPIOB_CLK_ENABLE()
#define CAN1_RX_CLK_DISABLE()     __HAL_RCC_GPIOB_CLK_DISABLE()


#define CAN2_TX_PORT              GPIOB
#define CAN2_TX_CLK_ENABLE()      __HAL_RCC_GPIOB_CLK_ENABLE()
#define CAN2_TX_CLK_DISABLE()     __HAL_RCC_GPIOB_CLK_DISABLE()

#define CAN2_RX_PORT              GPIOB
#define CAN2_RX_CLK_ENABLE()      __HAL_RCC_GPIOB_CLK_ENABLE()
#define CAN2_RX_CLK_DISABLE()     __HAL_RCC_GPIOB_CLK_DISABLE()

#define CAN1_TX_PIN               GPIO_PIN_9
#define CAN1_RX_PIN               GPIO_PIN_8
#define CAN2_TX_PIN               GPIO_PIN_13
#define CAN2_RX_PIN               GPIO_PIN_12


#define CAN_TRANS_TIMEOUT   (0xFFFF)

typedef enum
{
  CAN_ERR = -1,
  CAN_20K = 0,
  CAN_50K = 1,
  CAN_100K = 2,
  CAN_250K = 3,
  CAN_500K = 4,
  CAN_1M   = 5

}CANBRate_TypeDef;

typedef struct __attribute__((__packed__))
{
  uint32_t RTR;
  uint32_t IDE;
	uint16_t 	cob_id;
	uint8_t 	length;
	uint8_t		data[8];
}CANRxMSG_TypeDef;

typedef void (*BSP_CANx_CallBack)(CANRxMSG_TypeDef *rmsg);


/********************************** CAN1 ***************************************/
#define MASTER_CAN1_PRIORITY			3//抢占优先级
#define SLAVA_CAN1_PRIORITY				0//子优先级
/********************************** CAN2 ***************************************/
#define MASTER_CAN2_PRIORITY			3//抢占优先级
#define SLAVA_CAN2_PRIORITY				0//子优先级

bool             BSP_CAN1_Init(CANBRate_TypeDef brate, uint32_t id);
bool             BSP_CAN2_Init(CANBRate_TypeDef brate, uint32_t id);
bool             BSP_CAN1_DeInit(void);
bool             BSP_CAN2_DeInit(void);
bool             BSP_CAN1_Transmit(uint16_t ID, uint8_t *pdata, uint8_t length);
bool             BSP_CAN1_Transmit_frame(CAN_TxHeaderTypeDef *tmsg, uint8_t *pdata, uint8_t length);

bool             BSP_CAN2_Transmit(uint16_t ID, uint8_t *pdata, uint8_t length);
bool             BSP_CAN2_Transmit_frame(CAN_TxHeaderTypeDef *tmsg, uint8_t *pdata, uint8_t length);

bool             BSP_CAN1_SetRxCallBack(BSP_CANx_CallBack callback);
bool             BSP_CAN2_SetRxCallBack(BSP_CANx_CallBack callback);


#endif 
