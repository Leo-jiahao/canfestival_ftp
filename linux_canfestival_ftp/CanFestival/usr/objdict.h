/**
 * @file objdict.h
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

#ifndef _OBJDICT_H
#define _OBJDICT_H
#include "data.h"

/* Prototypes of function provided by object dictionnary */
UNS32 TestMaster_valueRangeTest (UNS8 typeValue, void * value);
const indextable * TestMaster_scanIndexOD (UNS16 wIndex, UNS32 * errorCode, ODCallback_t **callbacks);

/* Master node data struct */
extern CO_Data TestMaster_Data;
extern UNS8 MasterMap1;		/* Mapped at index 0x2000, subindex 0x00*/
extern UNS8 MasterMap2;		/* Mapped at index 0x2001, subindex 0x00*/
extern UNS8 MasterMap3;		/* Mapped at index 0x2002, subindex 0x00*/
extern UNS8 MasterMap4;		/* Mapped at index 0x2003, subindex 0x00*/
extern UNS8 MasterMap5;		/* Mapped at index 0x2004, subindex 0x00*/
extern UNS8 MasterMap6;		/* Mapped at index 0x2005, subindex 0x00*/
extern UNS8 MasterMap7;		/* Mapped at index 0x2006, subindex 0x00*/
extern UNS8 MasterMap8;		/* Mapped at index 0x2007, subindex 0x00*/
extern UNS8 MasterMap9;		/* Mapped at index 0x2008, subindex 0x00*/
extern UNS32 MasterMap10;		/* Mapped at index 0x2009, subindex 0x00*/
extern UNS16 MasterMap11;		/* Mapped at index 0x200A, subindex 0x00*/
extern INTEGER16 MasterMap12;		/* Mapped at index 0x200B, subindex 0x00*/
extern INTEGER16 MasterMap13;		/* Mapped at index 0x200C, subindex 0x00*/

#endif // !_OBJDICT_H
