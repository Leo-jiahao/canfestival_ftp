/**
 * @file objdict.h
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

#ifndef _OBJDICT_H
#define _OBJDICT_H
#include "data.h"

/* Prototypes of function provided by object dictionnary */
UNS32 TestSlave_valueRangeTest (UNS8 typeValue, void * value);
const indextable * TestSlave_scanIndexOD (UNS16 wIndex, UNS32 * errorCode, ODCallback_t **callbacks);

/* Master node data struct */
extern CO_Data TestSlave_Data;
extern ODCallback_t Store_parameters_callbacks[];		/* Callbacks of index0x1010 */
extern ODCallback_t Restore_Default_Parameters_callbacks[];		/* Callbacks of index0x1011 */
extern ODCallback_t Transmit_PDO_1_Parameter_callbacks[];		/* Callbacks of index0x1800 */
extern ODCallback_t Transmit_PDO_2_Parameter_callbacks[];		/* Callbacks of index0x1801 */
extern ODCallback_t Transmit_PDO_3_Parameter_callbacks[];		/* Callbacks of index0x1802 */
extern ODCallback_t Transmit_PDO_4_Parameter_callbacks[];		/* Callbacks of index0x1803 */
extern ODCallback_t Transmit_PDO_5_Parameter_callbacks[];		/* Callbacks of index0x1804 */
extern UNS8 SlaveMap1;		/* Mapped at index 0x2000, subindex 0x00*/
extern UNS8 SlaveMap2;		/* Mapped at index 0x2001, subindex 0x00*/
extern UNS8 SlaveMap3;		/* Mapped at index 0x2002, subindex 0x00*/
extern UNS8 SlaveMap4;		/* Mapped at index 0x2003, subindex 0x00*/
extern UNS8 SlaveMap5;		/* Mapped at index 0x2004, subindex 0x00*/
extern UNS8 SlaveMap6;		/* Mapped at index 0x2005, subindex 0x00*/
extern UNS8 SlaveMap7;		/* Mapped at index 0x2006, subindex 0x00*/
extern UNS8 SlaveMap8;		/* Mapped at index 0x2007, subindex 0x00*/
extern UNS8 SlaveMap9;		/* Mapped at index 0x2008, subindex 0x00*/
extern UNS32 SlaveMap10;		/* Mapped at index 0x2009, subindex 0x00*/
extern UNS16 SlaveMap11;		/* Mapped at index 0x200A, subindex 0x00*/
extern INTEGER16 SlaveMap12;		/* Mapped at index 0x200B, subindex 0x00*/
extern INTEGER16 SlaveMap13;		/* Mapped at index 0x200C, subindex 0x00*/

extern char TestSlave_obj6000[128];
#endif // !_OBJDICT_H
