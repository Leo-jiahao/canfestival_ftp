/**
 * @file stm32f405_demo1_flash.c
 * @author Leo-jiahao (leo884823525@gmail.com)
 * @brief 
 * @version 0.1
 * @date 2023-06-19
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
#include "stm32f405_demo1_flash.h"
#include "stm32f4xx_hal.h"


/**
 * @brief 
 * 
 * @param address 
 * @return uint32_t 
 */
uint32_t BSP_GetSectorIndex(uint32_t address)
{
    uint32_t sector = 0;
    if ((address < ADDR_FLASH_SECTOR_1) && (address >= ADDR_FLASH_SECTOR_0))
    {
        sector = FLASH_SECTOR_0;
    }
    else if ((address < ADDR_FLASH_SECTOR_2) && (address >= ADDR_FLASH_SECTOR_1))
    {
        sector = FLASH_SECTOR_1;
    }
    else if ((address < ADDR_FLASH_SECTOR_3) && (address >= ADDR_FLASH_SECTOR_2))
    {
        sector = FLASH_SECTOR_2;
    }
    else if ((address < ADDR_FLASH_SECTOR_4) && (address >= ADDR_FLASH_SECTOR_3))
    {
        sector = FLASH_SECTOR_3;
    }
    else if ((address < ADDR_FLASH_SECTOR_5) && (address >= ADDR_FLASH_SECTOR_4))
    {
        sector = FLASH_SECTOR_4;
    }
    else if ((address < ADDR_FLASH_SECTOR_6) && (address >= ADDR_FLASH_SECTOR_5))
    {
        sector = FLASH_SECTOR_5;
    }
    else if ((address < ADDR_FLASH_SECTOR_7) && (address >= ADDR_FLASH_SECTOR_6))
    {
        sector = FLASH_SECTOR_6;
    }
    else if ((address < ADDR_FLASH_SECTOR_8) && (address >= ADDR_FLASH_SECTOR_7))
    {
        sector = FLASH_SECTOR_7;
    }
    else if ((address < ADDR_FLASH_SECTOR_9) && (address >= ADDR_FLASH_SECTOR_8))
    {
        sector = FLASH_SECTOR_8;
    }
    else if ((address < ADDR_FLASH_SECTOR_10) && (address >= ADDR_FLASH_SECTOR_9))
    {
        sector = FLASH_SECTOR_9;
    }
    else if ((address < ADDR_FLASH_SECTOR_11) && (address >= ADDR_FLASH_SECTOR_10))
    {
        sector = FLASH_SECTOR_10;
    }
    else /*(Address < FLASH_END_ADDR) && (Address >= ADDR_FLASH_SECTOR_11))*/
    {
        sector = FLASH_SECTOR_11;
    }
    return sector;
}

/**
 * @brief 获取扇区的首地址
 * 
 * @param sector 
 * @return uint32_t 
 */
uint32_t BSP_GetFlashAddr(uint32_t sector)
{
    switch(sector)
	{
		case FLASH_SECTOR_0:
			return ADDR_FLASH_SECTOR_0;
		
		case FLASH_SECTOR_1:
			return ADDR_FLASH_SECTOR_1;
		
		case FLASH_SECTOR_2:
			return ADDR_FLASH_SECTOR_2;
		
		case FLASH_SECTOR_3:
			return ADDR_FLASH_SECTOR_3;
		
		case FLASH_SECTOR_4:
			return ADDR_FLASH_SECTOR_4;
		
		case FLASH_SECTOR_5:
			return ADDR_FLASH_SECTOR_5;
		
		case FLASH_SECTOR_6:
			return ADDR_FLASH_SECTOR_6;
		
		case FLASH_SECTOR_7:
			return ADDR_FLASH_SECTOR_7;
		
		case FLASH_SECTOR_8:
			return ADDR_FLASH_SECTOR_8;
		
		case FLASH_SECTOR_9:
			return ADDR_FLASH_SECTOR_9;
		
		case FLASH_SECTOR_10:
			return ADDR_FLASH_SECTOR_10;
		
		case FLASH_SECTOR_11:
			return ADDR_FLASH_SECTOR_11;
		
		default:
			return ADDR_FLASH_SECTOR_END;
	}
}
/**
 * @brief 
 * 
 * @param SectorNumber 
 * @return true 
 * @return false 
 */
bool BSP_EraseSector(uint32_t SectorNumber)
{
    __set_PRIMASK(1);//关中断
	HAL_FLASH_Unlock();
	
	uint32_t SectorError;
	FLASH_EraseInitTypeDef EraseInitStruct;
	
	EraseInitStruct.TypeErase = FLASH_TYPEERASE_SECTORS;       	
	EraseInitStruct.Sector = SectorNumber;   					
	EraseInitStruct.NbSectors = 1;                             	
	EraseInitStruct.VoltageRange = FLASH_VOLTAGE_RANGE_3;      	
	if(HAL_FLASHEx_Erase(&EraseInitStruct,&SectorError) != HAL_OK)
	{
		goto FLASH_ERRR;
	}

		
	HAL_FLASH_Lock();    
	__set_PRIMASK(0);
	return true;	
	
FLASH_ERRR:
	HAL_FLASH_Lock();    
	__set_PRIMASK(0);
	return false;
}
/**
 * @brief 
 * 
 * @param dest 
 * @param src 
 * @param len 
 * @return true 
 * @return false 
 */
bool BSP_WriteFlash(uint32_t dest, uint32_t *src, uint32_t len)
{
	HAL_StatusTypeDef FlashStatus;
    uint32_t *buffer = src;
    __set_PRIMASK(1);//关中断
	HAL_FLASH_Unlock();
	

	for(int i = 0,j = 0; i < len; i++,j++)
	{
		FlashStatus = FLASH_WaitForLastOperation(FLASH_WAIT_TIME);	//等待flash上次操作完成
		if(FlashStatus != HAL_OK)
		{
			goto FLASH_ERRR;
		}
        
		FlashStatus = HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD,(dest + j * 4),buffer[i]);
		if(FlashStatus != HAL_OK)
		{
			goto FLASH_ERRR;
		}
	}	
	
	HAL_FLASH_Lock();    
	__set_PRIMASK(0);
	return true;	
	
FLASH_ERRR:
	HAL_FLASH_Lock();    
	__set_PRIMASK(0);
	return false;
}
/**
 * @brief 
 * 
 * @param dest 
 * @param src 
 * @param len 
 * @return true 
 * @return false 
 */
bool BSP_WriteByteFlash(uint32_t dest, uint8_t *src, uint32_t len)
{
  HAL_StatusTypeDef FlashStatus;
  uint8_t *buffer = src;
  __set_PRIMASK(1);//关中断
	HAL_FLASH_Unlock();
	
	
  for (int i = 0; i < len; i++)
  {
    /* code */
    FlashStatus = FLASH_WaitForLastOperation(FLASH_WAIT_TIME);	//等待flash上次操作完成
		if(FlashStatus != HAL_OK)
		{
			goto FLASH_ERRR;
		}



    FlashStatus = HAL_FLASH_Program(FLASH_TYPEPROGRAM_BYTE, dest + i, buffer[i]);
    if(FlashStatus != HAL_OK)
		{
			goto FLASH_ERRR;
		}
  }	
	
	HAL_FLASH_Lock();    
	__set_PRIMASK(0);
	return true;	
	
FLASH_ERRR:
	HAL_FLASH_Lock();    
	__set_PRIMASK(0);
	return false;
}
