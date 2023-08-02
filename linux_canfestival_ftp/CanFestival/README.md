# 1、CanFestival简介
开源的canopen协议栈，实现了canopen301协议，基于部分对象字典，可用于402等更上层的软件协议。可用于各种平台，如windows linux，MCU等。  
官方源码地址：
```
https://codeload.github.com/ljessendk/CanFestival/zip/refs/heads/master
```
发现源码存在一些问题,PDO.c 的223行发现一些问题，修正后如下
```c

      // while (numMap < READ_UNS8(d->objdict, offsetObjdict, 0))
      while (numMap < READ_UNS8(d->objdict, offsetObjdict + numPdo, 0))

```
# 2、工程介绍
只留有mcu嵌入式平台所需要使用的文件，其源码还附带有example，drivers的示例，请通过上面源码链接下载查看。
## 2.1、文件框架
```shell
.
├── include
│   ├── can_driver.h
│   ├── can.h
│   ├── data.h
│   ├── dcf.h
│   ├── def.h
│   ├── emcy.h
│   ├── lifegrd.h
│   ├── lss.h
│   ├── nmtMaster.h
│   ├── nmtSlave.h
│   ├── objacces.h
│   ├── objdictdef.h
│   ├── pdo.h
│   ├── sdo.h
│   ├── states.h
│   ├── sync.h
│   ├── sysdep.h
│   ├── timer.h
│   └── timers_driver.h
├── src
│   ├── dcf.c
│   ├── emcy.c
│   ├── lifegrd.c
│   ├── lss.c
│   ├── nmtMaster.c
│   ├── nmtSlave.c
│   ├── objacces.c
│   ├── objaccessinternal.h
│   ├── pdo.c
│   ├── sdo.c
│   ├── states.c
│   ├── symbols.c
│   ├── sync.c
│   └── timer.c
└── usr
    ├── applicfg.h
    ├── canfestival.h
    ├── config.h
    ├── driver_can.c
    ├── driver_timer.c
    ├── objdict.c
    ├── objdict.h
    └── timerscfg.h

3 directories, 41 files
```
## 2.2、*include、src文件夹下的内容不需要任何修改，属于源码协议栈的内容*
## 2.3、 **usr文件夹下的是必须由用户进行配置的内容，.c文件里的接口被协议栈直接调用**
```c
// 协议栈源码src中的文件调用这些头文件默认定义的参数
#include "canfig.h"
#include "applicfg.h"
#include "canfestival.h"
#include "timerscfg.h"
// ...
```
```c
// usr中用户源码固定实现一些接口，但是源码本身不对外申明，通过协议栈include的头文件对外声明，并提供给协议栈源文件使用
// 协议栈头文件
#include "data.h"
#include "dcf.h"
#include "timers_driver.h"
#include "can_driver.h"
// can接口的固定实现
CAN_PORT canOpen(s_BOARD *board, CO_Data * d)；
UNS8 canSend(CAN_PORT port, Message *m)
int canClose(CO_Data * d)
// 基础timer的固定实现接口
void TimerInit(void)；
void setTimer(TIMEVAL value)；
TIMEVAL getElapsedTime(void)；
// 如果有一些系统级别的互斥保护需要，可以参考下面一些接口的实现
void LeaveMutex(void)；
void EnterMutex(void)；

```
# 3、用户配置文件详解
## 3.1、applicfg.h
由协议栈内部使用
*定义协议内部使用的变量类型，和状态信息的打印接口，can接口句柄类型*

```c
#include <stdio.h>
#include <stdint.h>
#include <string.h>
//允许MDK编译匿名联合体
#if defined ( __CC_ARM   )
#pragma anon_unions
#endif

/* Integers */
#define INTEGER8 int8_t
#define INTEGER16 int16_t
#define INTEGER24 int32_t
#define INTEGER32 int32_t
#define INTEGER40 int64_t
#define INTEGER48 int64_t
#define INTEGER56 int64_t
#define INTEGER64 int64_t

/* Unsigned integers */
#define UNS8   uint8_t
#define UNS16  uint16_t
#define UNS32  uint32_t
#define UNS24  uint32_t
#define UNS40  uint64_t
#define UNS48  uint64_t
#define UNS56  uint64_t
#define UNS64  uint64_t

/* Reals */
#define REAL32	float
#define REAL64 double

/* Definition of error and warning macros */
/* -------------------------------------- */
#	define MSG(...) printf (__VA_ARGS__)


typedef void* CAN_HANDLE;

typedef void* CAN_PORT;
```
## 3.2、canfestival.h
*声明整个协议栈对外的接口，和部分提供给协议栈内部使用的接口*
```c

UNS8 canSend(CAN_PORT port, Message *m);

CAN_PORT canOpen(s_BOARD *board, CO_Data * d);

int canClose(CO_Data * d);

```
## 3.3、config.h
用于配置协议栈的功能参数，比如是否使用动态内存分配，块传输大小限制，从节点最大ID等
```c
#define MAX_CAN_BUS_ID 1
/* 使能动态分配，接收/发送的对象字典数据长度大于SDO_MAX_LENGTH_TRANSFER时，会动态分配SDO_DYNAMIC_BUFFER_ALLOCATION_SIZE，给到当前的SDO传输线，注意MCU端的RAM限制
*/
#define SDO_DYNAMIC_BUFFER_ALLOCATION 
#define SDO_DYNAMIC_BUFFER_ALLOCATION_SIZE (1024 * 128)
#define SDO_MAX_LENGTH_TRANSFER 32
#define SDO_MAX_SIMULTANEOUS_TRANSFERS 32
#define SDO_BLOCK_SIZE 16
#define NMT_MAX_NODE_ID 128
#define SDO_TIMEOUT_MS 3000
#define MAX_NB_TIMER 32

#define EMCY_MAX_ERRORS 8
#define LSS_TIMEOUT_MS 1000
#define LSS_FS_TIMEOUT_MS 100

#define REPEAT_SDO_MAX_SIMULTANEOUS_TRANSFERS_TIMES(repeat)\
repeat repeat repeat repeat repeat repeat repeat repeat repeat repeat repeat repeat repeat repeat repeat repeat repeat repeat repeat repeat repeat repeat repeat repeat repeat repeat repeat repeat repeat repeat repeat repeat
#define REPEAT_NMT_MAX_NODE_ID_TIMES(repeat)\
repeat repeat repeat repeat repeat repeat repeat repeat repeat repeat repeat repeat repeat repeat repeat repeat repeat repeat repeat repeat repeat repeat repeat repeat repeat repeat repeat repeat repeat repeat repeat repeat repeat repeat repeat repeat repeat repeat repeat repeat repeat repeat repeat repeat repeat repeat repeat repeat repeat repeat repeat repeat repeat repeat repeat repeat repeat repeat repeat repeat repeat repeat repeat repeat repeat repeat repeat repeat repeat repeat repeat repeat repeat repeat repeat repeat repeat repeat repeat repeat repeat repeat repeat repeat repeat repeat repeat repeat repeat repeat repeat repeat repeat repeat repeat repeat repeat repeat repeat repeat repeat repeat repeat repeat repeat repeat repeat repeat repeat repeat repeat repeat repeat repeat repeat repeat repeat repeat repeat repeat repeat repeat repeat repeat repeat repeat repeat repeat
#define REPEAT_EMCY_MAX_ERRORS_TIMES(repeat)\
repeat repeat repeat repeat repeat repeat repeat repeat

#define DEBUG_WAR_CONSOLE_ON
#define DEBUG_ERR_CONSOLE_ON
```
## 3.4、objdict.h
对外声明定义的对象字典描述结构体，和对象字典参数，查询函数对象字典索引的函数。
```c
UNS32 TestMaster_valueRangeTest (UNS8 typeValue, void * value);
const indextable * TestMaster_scanIndexOD (UNS16 wIndex, UNS32 * errorCode, ODCallback_t **callbacks);

/* Master node data struct */
extern CO_Data TestMaster_Data;
extern UNS8 MasterMap1;		/* Mapped at index 0x2000, subindex 0x00*/
// ...

```
## 3.5、timerscfg.h
时基定时器参数单位。
定义时基值类型，定时器最大数值，对于STM32F4，只有TIM2 TIM5最大计数值为32，0xFFFFFFFF，其他定时器都是16位的0xFFFF。
```c
// Whatever your microcontroller, the timer wont work if
// TIMEVAL is not at least on 32 bits
#define TIMEVAL UNS32

// using 16 bits timer
#define TIMEVAL_MAX 0xFFFF

// The timer is incrementing every 10 us.
#define MS_TO_TIMEVAL(ms) ((ms) * 100)
#define US_TO_TIMEVAL(us) ((us) / 10)
```

对于linux系统
```c
#ifndef __TIMERSCFG_H__
#define __TIMERSCFG_H__


#include <pthread.h>

/* Time unit : us */
/* Time resolution : 64bit (~584942 years) */
#define TIMEVAL unsigned long long
#define TIMEVAL_MAX ~(TIMEVAL)0
#define MS_TO_TIMEVAL(ms) ms*1000L
#define US_TO_TIMEVAL(us) us

#define TASK_HANDLE pthread_t


#endif /* __TIMERSCFG_H__ */
```

## 3.6、objdict.c
定义对象字典表。
更据canopen的定义，不同cob-id字段对应有不同功能，如  
|Index range 索引范围|Description 描述|
|-------------------|------------------|
|0000h|Reserved 保留|
|0001h to 025Fh|Data types 数据类型|
|0260h to 0FFFh|Reserved 保留|
|1000h to 1FFFh|Communication profile area 通讯对象子协议区|
|2000h to 5FFFh|Manufacturer-specific profile area 制造商特定子协议区|
|6000h to 9FFFh|Standardized profile area 标准化设备子协议区|
再详细看1000h段
|Index range 索引范围|Description 描述|
|-------------------|------------------|
|1000h to 1029h|General communication objects 通用通讯对象|
|1200h to 12FFh|SDO parameter objects SDO 参数对象|
|1300h to 13FFh|CANopen safety objects 安全对象|
|1400h to 1BFFh|PDO parameter objects PDO 参数对象|
|...|。。。|
*对于1000h to 1029h段，需要配置一些特殊的对象字典的参数来开启一些功能*，
如需要配置心跳时间启动canopen的同步心跳功能  
index 0x1006 :   Communication / SYNC Cycle Period.  
对于0x1200h to 12ffh字段，用于配置sdo会话,比如配置0x602的cobid为主设备查看0x582的从设备，从节点id为2
```c
/* index 0x1280 :   Client SDO 1 Parameter. */
                    UNS8 TestMaster_highestSubIndex_obj1280 = 3; /* number of subindex - 1*/
                    UNS32 TestMaster_obj1280_COB_ID_Client_to_Server_Transmit_SDO = 0x602;	/* 1538 */
                    UNS32 TestMaster_obj1280_COB_ID_Server_to_Client_Receive_SDO = 0x582;	/* 1410 */
                    UNS8 TestMaster_obj1280_Node_ID_of_the_SDO_Server = 0x2;	/* 2 */
                    subindex TestMaster_Index1280[] = 
                     {
                       { RO, uint8, sizeof (UNS8), (void*)&TestMaster_highestSubIndex_obj1280 },
                       { RW, uint32, sizeof (UNS32), (void*)&TestMaster_obj1280_COB_ID_Client_to_Server_Transmit_SDO },
                       { RW, uint32, sizeof (UNS32), (void*)&TestMaster_obj1280_COB_ID_Server_to_Client_Receive_SDO },
                       { RW, uint8, sizeof (UNS8), (void*)&TestMaster_obj1280_Node_ID_of_the_SDO_Server }
                     };
```
配置后，可以调用协议栈接口和从节点进行通信  
配置如下对象字典可以配置PDO功能
1400h->15ffh RPDO通讯参数
1600h->17ffh RPDO映射参数
1800h->19ffh TPDO通讯参数
1a00h->1bffh TPDO映射参数
通讯参数配置示例：
```c
/* index 0x1400 :   Receive PDO 1 Parameter. */
                    UNS8 TestMaster_highestSubIndex_obj1400 = 5; /* number of subindex - 1*/
                    UNS32 TestMaster_obj1400_COB_ID_used_by_PDO = 0x200;	/* 512 */
                    UNS8 TestMaster_obj1400_Transmission_Type = 0x1;	/* 1 */
                    UNS16 TestMaster_obj1400_Inhibit_Time = 0x0;	/* 0 */
                    UNS8 TestMaster_obj1400_Compatibility_Entry = 0x0;	/* 0 */
                    UNS16 TestMaster_obj1400_Event_Timer = 0x0;	/* 0 */
                    subindex TestMaster_Index1400[] = 
                     {
                       { RO, uint8, sizeof (UNS8), (void*)&TestMaster_highestSubIndex_obj1400 },
                       { RW, uint32, sizeof (UNS32), (void*)&TestMaster_obj1400_COB_ID_used_by_PDO },
                       { RW, uint8, sizeof (UNS8), (void*)&TestMaster_obj1400_Transmission_Type },
                       { RW, uint16, sizeof (UNS16), (void*)&TestMaster_obj1400_Inhibit_Time },
                       { RW, uint8, sizeof (UNS8), (void*)&TestMaster_obj1400_Compatibility_Entry },
                       { RW, uint16, sizeof (UNS16), (void*)&TestMaster_obj1400_Event_Timer }
                     };
```
映射参数配置示例：
```c
/* index 0x1600 :   Receive PDO 1 Mapping. */
                    UNS8 TestMaster_highestSubIndex_obj1600 = 10; /* number of subindex - 1*/
                    UNS32 TestMaster_obj1600[] = 
                    {
                      0x20000001,	/* 536870913 */
                      0x20010001,	/* 536936449 */
                      0x20020001,	/* 537001985 */
                      0x20030001,	/* 537067521 */
                      0x20040001,	/* 537133057 */
                      0x20050001,	/* 537198593 */
                      0x20060001,	/* 537264129 */
                      0x20070001,	/* 537329665 */
                      0x20080008,	/* 537395208 */
                      0x20090020	/* 537460768 */
                    };
                    subindex TestMaster_Index1600[] = 
                     {
                       { RW, uint8, sizeof (UNS8), (void*)&TestMaster_highestSubIndex_obj1600 },
                       { RW, uint32, sizeof (UNS32), (void*)&TestMaster_obj1600[0] },
                       { RW, uint32, sizeof (UNS32), (void*)&TestMaster_obj1600[1] },
                       { RW, uint32, sizeof (UNS32), (void*)&TestMaster_obj1600[2] },
                       { RW, uint32, sizeof (UNS32), (void*)&TestMaster_obj1600[3] },
                       { RW, uint32, sizeof (UNS32), (void*)&TestMaster_obj1600[4] },
                       { RW, uint32, sizeof (UNS32), (void*)&TestMaster_obj1600[5] },
                       { RW, uint32, sizeof (UNS32), (void*)&TestMaster_obj1600[6] },
                       { RW, uint32, sizeof (UNS32), (void*)&TestMaster_obj1600[7] },
                       { RW, uint32, sizeof (UNS32), (void*)&TestMaster_obj1600[8] },
                       { RW, uint32, sizeof (UNS32), (void*)&TestMaster_obj1600[9] }
                     };
```
**说明，通过1400h的通讯参数配置了0x200的cobid为RPDO，在通过1600中的映射参数，配置cobid为0x200的can帧中8个字节每个一个bit对应数据，比如**
**对象字典0x2000的一个bool数据，0x2001的一个bool数据，...，0x2008的一个字节数据，0x2009的一个u32数据，**
总共使用了其中的6个字节

而在 PDO 预定义中，人为规定了TPDO和RPDO，规定了Node-ID在PDO中的位置，规定了PDO的编号，
|object对象|CAN-ID|
|---------|-------|
|TPDO1 发送过程数据对象 1|181h to 1FFh（180h +node-ID）|
|RPDO1 接收过程数据对象 1|201h to 27Fh（200h +node-ID）|
|TPDO2 发送过程数据对象 2|281h to 2FFh（280h +node-ID）|
|RPDO2 接收过程数据对象 2|301h to 37Fh（300h +node-ID）|
|TPDO3 发送过程数据对象 3|381h to 3FFh（380h +node-ID）|
|RPDO3 接收过程数据对象 3|401h to 47Fh（400h +node-ID）|
|TPDO4 发送过程数据对象 4|481h to 4FFh（480h +node-ID）|
|RPDO4 接收过程数据对象 4|501h to 57Fh（500h +node-ID）|

quick index参数对从站和主站的配置不一样
```c
// 从站
quick_index TestSlave_firstIndex = {
  10, /* SDO_SVR，对应主站中配置的SDO参数对象字典，所以为10（0x1200），配置的600h 和580h */
  0, /* SDO_CLT ，主站才需要配置为有效值*/
  0, /* PDO_RCV ，RPDO通讯参数开始index*/
  0, /* PDO_RCV_MAP，RPDO映射参数开始地址*/
  11, /* PDO_TRS ，同理*/
  16 /* PDO_TRS_MAP ，同理*/
};

quick_index TestSlave_lastIndex = {
  10, /* SDO_SVR */
  0, /* SDO_CLT，主站才需要配置为有效值*/
  0, /* PDO_RCV ，RPDO通讯参数结束index*/
  0, /* PDO_RCV_MAP ，同理*/
  15, /* PDO_TRS ，同理*/
  20 /* PDO_TRS_MAP ，同理*/
};
```

# 4、重要
dcf.c文件 94 行
```c
inline void start_and_seek_node(CO_Data* d, UNS8 nodeId){
// MCU端更换为
static inline void start_and_seek_node(CO_Data* d, UNS8 nodeId){
```
dcf.c文件 59行
```c
inline void start_node(CO_Data* d, UNS8 nodeId){
// MCU端更换为
static inline void start_node(CO_Data* d, UNS8 nodeId){
```

# 5、canopen详解
参考链接：https://www.zhihu.com/column/canopen


# 6、一些注意事项
如果遇到linux上 can 发送 No buffer space available，这个问题是系统给的缓冲队列空间不足
```shell
cd /sys/class/net/can0
cat tx_queue_len
10
echo 4096 >tx_queue_len
```
