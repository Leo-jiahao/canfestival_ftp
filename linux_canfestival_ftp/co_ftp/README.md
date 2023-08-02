# CO_FTP
基于CANOPEN（CANFESTIVAL-3）的一种文件传输方案。

# 1、介绍
分为服务器端和客户端，概念和CANFestival的客户端和服务端是一致的。  
一般情况下，客户端需要部署在嵌入式linux等资源相对丰富的系统上，使用的测试平台也是这种。  
服务端的部署平台没有限制，只测试了在MCU端的有效性，在其他平台也可使用。


# 2、server_ftp
服务端的配置需要做以下部分工作：

## 2.1、配置server_ftp.h，

```c
//以下是允许修改的配置参数
#include "SEGGER_RTT.h"

#define FTP_MSG(...) SEGGER_RTT_printf(0,__VA_ARGS__)


#define FTP_BLOCK_MAX_SIZE          1024
#define FTP_FILE_NAME_MAX_SIZE      32
#define FTP_CO_DATA_OBJ_INDEX       0x3000

#define FTP_LOG_ENABLE   1
```
解析：
1. FTP_MSG(...)，配置日志输出接口，
2. FTP_BLOCK_MAX_SIZE，配置文件分块传输的最大块大小
3. FTP_CO_DATA_OBJ_INDEX，配置FTP对象字典的主索引，*需要和client_ftp.h保持一致*
4. FTP_LOG_ENABLE，使能日志输出

## 2.2、 配置canfestival的objdict.c文件，将server_ftp.h中声明的字典对象添加进协议栈。
```c
//server_ftp.h

#include "data.h"

...

#define FTP_BLOCK_MAX_SIZE          1024
#define FTP_FILE_NAME_MAX_SIZE      32
#define FTP_CO_DATA_OBJ_INDEX       0x3000

...

extern ODCallback_t CO_Data_FTP_callbacks[7];

extern subindex CO_Data_FTP[7];
```

```c
//objdict.c
...
#include "server_ftp.h"
...

const indextable TestSlave_objdict[] = 
{
    ...
    { (subindex*)CO_Data_FTP,        sizeof(CO_Data_FTP)/sizeof(CO_Data_FTP[0]),     FTP_CO_DATA_OBJ_INDEX},
    ...
};

...
const indextable * TestSlave_scanIndexOD (UNS16 wIndex, UNS32 * errorCode, ODCallback_t **callbacks)
{
    ...
    case 0x200B: i = 32;break;
	  case 0x200C: i = 33;break;
    //主要添加此行
    case FTP_CO_DATA_OBJ_INDEX: i = 34;*callbacks = CO_Data_FTP_callbacks; break;
    case 0x6000: i = 35;*callbacks = TestSlave_Index6000_callbacks;break;
    ...
}
```

## 2.3、在用户区传入处理函数
示例

**注意处理函数的执行时长不能长于，canfestival的溢出时间，config.h中的SDO_TIMEOUT_MS**

```c
...
#include "server_ftp.h"
...

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
		
      SEGGER_RTT_printf(0,"File Received successfully: %d bytes\r\n",file_size);
    
	}
  }

  return FTP_SUCCESSFUL;
    
}

...
void canopen_demo(void)
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
...
```

# 3、client_ftp
客户端端的配置需要做以下部分工作：

## 3.1、配置client_ftp.h

```C
...
//使能日志输入
#define FTP_LOG_ENABLE   1

//配置日志输出接口
#define FTP_MSG(...) printf(__VA_ARGS__)

//配置文件名最大长度
#define FTP_FILE_NAME_MAX_SIZE      32

//配置对象字典的主索引，需要和server_ftp.h保持一致
#define FTP_CO_DATA_OBJ_INDEX       (0x3000)

...
```

## 3.2、用户使用

注意：*客户端使用，建议在富资源的平台使用，比如嵌入式linux系统，将整个文件读入缓存，client_ftp会直接操作整个缓存*

```C
...
//每块发送成功后的回调函数，比如上位机显示进度条等需要
void ftp_callback(int tell)
{
	printf("============transmited Bytes:%d===================\n", tell);
}
...
int main(int argc, char **argv)
{
	int res;
    s_BOARD u_board={
        .busname = "can0",
        .baudrate = "1MB",
    };
    /* 从站向主站发送的心跳斑纹超时后的回调函数 */
    TestMaster_Data.heartbeatError = heartbeatTimeOut;
    /* 当前节点进入到初始化状态的回调函数,可以writeLocalDict配置本地的PDO等 */
    TestMaster_Data.initialisation = state_change;

    /* 节点进入preoperation的回调函数*/
    TestMaster_Data.preOperational = state_change;

    /* 节点进入operation的回调函数*/
    TestMaster_Data.operational = state_change;
    TestMaster_Data.stopped = state_change;

    /*节点接收到同步包后的回调,之后协议栈内部再处理*/
    TestMaster_Data.post_sync = TestMaster_post_sync;

    /*节点接收到同步包后post_sync先处理，在协议栈处理，最后调用。通常可以在一段PDO接收后进行某些判断*/
    TestMaster_Data.post_TPDO = TestMaster_post_TPDO;
    
    /*接收到紧急事件包之后回调*/
    TestMaster_Data.post_emcy = TestMaster_post_emcy;
    
    /*接收到外部节点的bootup包的回调*/
    TestMaster_Data.post_SlaveBootup = TestMaster_post_SlaveBootup;
    
    /*外部节点的心跳报文中的状态发生变化的回调*/
    TestMaster_Data.post_SlaveStateChange = slave_state_change_callback;
    TimerInit();
    system("ifconfig can0 down");
    system("ip link set can0 up type can bitrate 1000000 triple-sampling on");
    if(!canOpen(&u_board, &TestMaster_Data)){
        goto fail;
    }
    
    setState(&TestMaster_Data, Initialisation);

	char u_string[] = "canfesitival_block test SDO_DYNAMIC_BUFFER_ALLOCATION SDO_DYNAMIC_BUFFER_ALLOCATION_SIZE";

	char *test_file_path = "./test.bin";
	int fd = open(test_file_path, O_RDONLY);
	int file_size = lseek(fd,0,SEEK_END);
	char *buffer = (char *)malloc(file_size);
	lseek(fd,0,SEEK_SET);
	read(fd,buffer,file_size);
	close(fd);
	printf("\nfile size :%d(Bytes)\n", file_size);
	res = client_ftp(&TestMaster_Data, 2, "test.bin", buffer, file_size, ftp_callback);
	printf("client_ftp:%d\n",res);
    while (1)
    {
		
		usleep(20000000);

    }
    

fail:
    canClose(&TestMaster_Data);
    return 0;
}

```
# 4、测试

*client 日志：*
```shell
file size :92204(Bytes)
FTP LOG:/home/user/workspace/canfestival_demo/client_ftp.c,196 : Step:1, Write file name 0X0 
client_ftp:1
FTP LOG:/home/user/workspace/canfestival_demo/client_ftp.c,211 : Step:2, Read Block Max size 0X0 
FTP LOG:/home/user/workspace/canfestival_demo/client_ftp.c,225 : Step:3, Write File size: 0X1682c 
FTP LOG:/home/user/workspace/canfestival_demo/client_ftp.c,243 : Step:4, Read Next Block Index 0X0 
============transmited Bytes:1024===================
FTP LOG:/home/user/workspace/canfestival_demo/client_ftp.c,281 : Step:6, Write This Block Data 0X0 
FTP LOG:/home/user/workspace/canfestival_demo/client_ftp.c,243 : Step:4, Read Next Block Index 0X0 
============transmited Bytes:2048===================
FTP LOG:/home/user/workspace/canfestival_demo/client_ftp.c,281 : Step:6, Write This Block Data 0X1 
FTP LOG:/home/user/workspace/canfestival_demo/client_ftp.c,243 : Step:4, Read Next Block Index 0X0 
============transmited Bytes:3072===================
FTP LOG:/home/user/workspace/canfestival_demo/client_ftp.c,281 : Step:6, Write This Block Data 0X2 
FTP LOG:/home/user/workspace/canfestival_demo/client_ftp.c,243 : Step:4, Read Next Block Index 0X0 
============transmited Bytes:4096===================

...

============transmited Bytes:92160===================
FTP LOG:/home/user/workspace/canfestival_demo/client_ftp.c,281 : Step:6, Write This Block Data 0X59 
FTP LOG:/home/user/workspace/canfestival_demo/client_ftp.c,243 : Step:4, Read Next Block Index 0X0 
============transmited Bytes:92204===================
FTP LOG:/home/user/workspace/canfestival_demo/client_ftp.c,281 : Step:6, Write This Block Data 0X5a 
```

*server 日志：*
```shell
00> FTP LOG:..\Modules\canopen\server_ftp.c,112 : Step:1, test.bin 0X0 
00> FTP LOG:..\Modules\canopen\server_ftp.c,125 : Step:2, FILE SIZE 0X1682C 
00> FTP LOG:..\Modules\canopen\server_ftp.c,141 : Step:3, Received Block Index 0X0 
00> FTP LOG:..\Modules\canopen\server_ftp.c,142 : Step:3, Received Block Size 0X400 
00> FTP LOG:..\Modules\canopen\server_ftp.c,143 : Step:3, Ftell 0X0 
00> FTP LOG:..\Modules\canopen\server_ftp.c,157 : Step:4, User callback returned Success 0X400 
00> FTP LOG:..\Modules\canopen\server_ftp.c,141 : Step:3, Received Block Index 0X1 
00> FTP LOG:..\Modules\canopen\server_ftp.c,142 : Step:3, Received Block Size 0X400 
00> FTP LOG:..\Modules\canopen\server_ftp.c,143 : Step:3, Ftell 0X400 

...

00> FTP LOG:..\Modules\canopen\server_ftp.c,143 : Step:3, Ftell 0X16400 
00> FTP LOG:..\Modules\canopen\server_ftp.c,157 : Step:4, User callback returned Success 0X16800 
00> FTP LOG:..\Modules\canopen\server_ftp.c,141 : Step:3, Received Block Index 0X5A 
00> FTP LOG:..\Modules\canopen\server_ftp.c,142 : Step:3, Received Block Size 0X2C 
00> FTP LOG:..\Modules\canopen\server_ftp.c,143 : Step:3, Ftell 0X16800 
00> File Received successfully: 92204 bytes
00> FTP LOG:..\Modules\canopen\server_ftp.c,157 : Step:4, User callback returned Success 0X1682C 
00> FTP LOG:..\Modules\canopen\server_ftp.c,159 : Step:5, Received File OK, File size: 0X1682C 
```

后续对数据进行比对，收发双方完全一致

