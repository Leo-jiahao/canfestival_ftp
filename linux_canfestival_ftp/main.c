/**
 * @file main.c
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
#include "./CanFestival/usr/canfestival.h"
#include "objdict.h"
#include <unistd.h>
#include "client_ftp.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
static void slave_state_change_callback(CO_Data* d, UNS8 nodeId, e_nodeState newNodeState);

static int init_step = 0;
/*Froward declaration*/
static void ConfigureSlaveNode(CO_Data* d, UNS8 nodeId);

void heartbeatTimeOut(CO_Data* d, UNS8 nodeid)
{
    slave_state_change_callback(d, nodeid, Disconnected);
}

static void slave_state_change_callback(CO_Data* d, UNS8 nodeId, e_nodeState newNodeState)
{
    if(newNodeState == Initialisation)
        printf("Node %u state is now  : Initialisation\n", nodeId);
    else if(newNodeState == Disconnected)
        printf("Node %u state is now  : Disconnected\n", nodeId);
    else if(newNodeState == Connecting)
        printf("Node %u state is now  : Connecting\n", nodeId);
    else if(newNodeState == Preparing)
        printf("Node %u state is now  : Preparing\n", nodeId);
    else if(newNodeState == Stopped)
        printf("Node %u state is now  : Stopped\n", nodeId);
    else if(newNodeState == Operational)
        printf("Node %u state is now  : Operational\n", nodeId);
    else if(newNodeState == Pre_operational)
        printf("Node %u state is now  : Pre_operational\n", nodeId);
    else if(newNodeState == Unknown_state)
        printf("Node %u state is now  : Unknown_state\n", nodeId);
    else
        printf("Error : node %u unexpected state\n", nodeId);
}




void state_change(CO_Data* d)
{
    if(d->nodeState == Initialisation)
        printf("Node state is now  : Initialisation\n");
    else if(d->nodeState == Disconnected)
        printf("Node state is now  : Disconnected\n");
    else if(d->nodeState == Connecting)
        printf("Node state is now  : Connecting\n");
    else if(d->nodeState == Preparing)
        printf("Node state is now  : Preparing\n");
    else if(d->nodeState == Stopped)
        printf("Node state is now  : Stopped\n");
    else if(d->nodeState == Operational)
        printf("Node state is now  : Operational\n");
    else if(d->nodeState == Pre_operational)
        printf("Node state is now  : Pre_operational\n");
    else if(d->nodeState == Unknown_state)
        printf("Node state is now  : Unknown_state\n");
    else
        printf("Error : unexpected node state\n");
}
static void setheartbeatPtoduceTime(CO_Data* d, UNS8 nodeId)
{
	UNS32 abortCode;

	if(getWriteResultNetworkDict (d, nodeId, &abortCode) != SDO_FINISHED)
		printf("Master : Failed in initializing slave %2.2x, step %d, AbortCode :%4.4x \n", nodeId, init_step, abortCode);

	/* Finalise last SDO transfer with this node */
	closeSDOtransfer(&TestMaster_Data, nodeId, SDO_CLIENT);

	printf("et the heartbeat Producer Time, slave node id:%d\r", nodeId);

}
void TestMaster_post_SlaveBootup(CO_Data* d, UNS8 nodeid)
{
	printf("TestMaster_post_SlaveBootup %x\n", nodeid);
	UNS8 res;
	ConfigureSlaveNode(d, nodeid);
}



/**/
static void CheckSDOAndContinue(CO_Data* d, UNS8 nodeId)
{
	UNS32 abortCode;

	if(getWriteResultNetworkDict (d, nodeId, &abortCode) != SDO_FINISHED)
		printf("Master : Failed in initializing slave %2.2x, step %d, AbortCode :%4.4x \n", nodeId, init_step, abortCode);

	/* Finalise last SDO transfer with this node */
	closeSDOtransfer(d, nodeId, SDO_CLIENT);

	ConfigureSlaveNode(d, nodeId);
}



/********************************************************
 * ConfigureSlaveNode is responsible to
 *  - setup slave TPDO 1 transmit time
 *  - setup slave TPDO 2 transmit time
 *  - setup slave Heartbeat Producer time
 *  - switch to operational mode
 *  - send NMT to slave
 ********************************************************
 * This an example of :
 * Network Dictionary Access (SDO) with Callback 
 * Slave node state change request (NMT) 
 ********************************************************
 * This is called first by TestMaster_preOperational
 * then it called again each time a SDO exchange is
 * finished.
 ********************************************************/
static void ConfigureSlaveNode(CO_Data* d, UNS8 nodeId)
{
	UNS8 res;
	printf("Master : ConfigureSlaveNode %2.2x\n", nodeId);
	switch(++init_step){
		case 1: 
		{	/*disable Slave's TPDO 1 */
			UNS32 TPDO_COBId = 0x00000180 + nodeId;
			
			printf("Master : disable slave %2.2x TPDO 1 \n", nodeId);
			res = writeNetworkDictCallBack (d, /*CO_Data* d*/
					/**TestSlave_Data.bDeviceNodeId, UNS8 nodeId*/
					nodeId, /*UNS8 nodeId*/
					0x1800, /*UNS16 index*/
					0x01, /*UNS8 subindex*/
					4, /*UNS8 count*/
					0, /*UNS8 dataType*/
					&TPDO_COBId,/*void *data*/
					CheckSDOAndContinue, /*SDOCallback_t Callback*/
                    0); /* use block mode */
		}			
		break;

		case 2: 
		{	/*setup Slave's TPDO 1 to be transmitted on SYNC*/
			UNS8 Transmission_Type = TRANS_RTR_SYNC;
			
			printf("Master : set slave %2.2x TPDO 1 transmit type\n", nodeId);
			res = writeNetworkDictCallBack (d, /*CO_Data* d*/
					/**TestSlave_Data.bDeviceNodeId, UNS8 nodeId*/
					nodeId, /*UNS8 nodeId*/
					0x1800, /*UNS16 index*/
					0x02, /*UNS8 subindex*/
					1, /*UNS8 count*/
					0, /*UNS8 dataType*/
					&Transmission_Type,/*void *data*/
					CheckSDOAndContinue, /*SDOCallback_t Callback*/
                    0); /* use block mode */
		}			
		break;

		case 3: 
		{	/*re-enable Slave's TPDO 1 */
			UNS32 TPDO_COBId = 0x00000180 + nodeId;
			
			printf("Master : re-enable slave %2.2x TPDO 1\n", nodeId);
			res = writeNetworkDictCallBack (d, /*CO_Data* d*/
					/**TestSlave_Data.bDeviceNodeId, UNS8 nodeId*/
					nodeId, /*UNS8 nodeId*/
					0x1800, /*UNS16 index*/
					0x01, /*UNS8 subindex*/
					4, /*UNS8 count*/
					0, /*UNS8 dataType*/
					&TPDO_COBId,/*void *data*/
					CheckSDOAndContinue, /*SDOCallback_t Callback*/
                    0); /* use block mode */
		}			
		break;
					
		case 4: 
		{	/*disable Slave's RPDO 1 */
			UNS32 TPDO_COBId = 0x00000200 + nodeId;
			
			printf("Master : disable slave %2.2x RPDO 1\n", nodeId);
			res = writeNetworkDictCallBack (d, /*CO_Data* d*/
					/**TestSlave_Data.bDeviceNodeId, UNS8 nodeId*/
					nodeId, /*UNS8 nodeId*/
					0x1400, /*UNS16 index*/
					0x01, /*UNS8 subindex*/
					4, /*UNS8 count*/
					0, /*UNS8 dataType*/
					&TPDO_COBId,/*void *data*/
					CheckSDOAndContinue, /*SDOCallback_t Callback*/
                    0); /* use block mode */
		}			
		break;

					
		case 5:
		{	
			UNS8 Transmission_Type = TRANS_RTR_SYNC;
			
			printf("Master : set slave %2.2x RPDO 1 receive type\n", nodeId);
			res = writeNetworkDictCallBack (d, /*CO_Data* d*/
					/**TestSlave_Data.bDeviceNodeId, UNS8 nodeId*/
					nodeId, /*UNS8 nodeId*/
					0x1400, /*UNS16 index*/
					0x02, /*UNS8 subindex*/
					1, /*UNS8 count*/
					0, /*UNS8 dataType*/
					&Transmission_Type,/*void *data*/
					CheckSDOAndContinue, /*SDOCallback_t Callback*/
                    0); /* use block mode */
		}	
		break;

		case 6: 
		{	/*re-enable Slave's RPDO 1 */
			UNS32 TPDO_COBId = 0x00000200 + nodeId;
			
			printf("Master : re-enable %2.2x RPDO 1\n", nodeId);
			res = writeNetworkDictCallBack (d, /*CO_Data* d*/
					/**TestSlave_Data.bDeviceNodeId, UNS8 nodeId*/
					nodeId, /*UNS8 nodeId*/
					0x1400, /*UNS16 index*/
					0x01, /*UNS8 subindex*/
					4, /*UNS8 count*/
					0, /*UNS8 dataType*/
					&TPDO_COBId,/*void *data*/
					CheckSDOAndContinue, /*SDOCallback_t Callback*/
                    0); /* use block mode */
		}			
		break;
		
		case 7:	
		{
			/*set the heartbeat Producer Time*/
			UNS16 Heartbeat_Producer_Time = 0x03E8; 
			printf("Master : set slave %2.2x heartbeat producer time \n", nodeId);
			res = writeNetworkDictCallBack (d, /*CO_Data* d*/
					/**TestSlave_Data.bDeviceNodeId, UNS8 nodeId*/
					nodeId, /*UNS8 nodeId*/
					0x1017, /*UNS16 index*/
					0x00, /*UNS8 subindex*/
					2, /*UNS8 count*/
					0, /*UNS8 dataType*/
					&Heartbeat_Producer_Time,/*void *data*/
					CheckSDOAndContinue, /*SDOCallback_t Callback*/
                    0); /* use block mode */
		}			
		break;

		case 8: 
		{	/*disable Slave's TPDO 2 */
			UNS32 TPDO_COBId = 0x00000280 + nodeId;
			
			printf("Master : disable slave %2.2x TPDO 2 \n", nodeId);
			res = writeNetworkDictCallBack (d, /*CO_Data* d*/
					/**TestSlave_Data.bDeviceNodeId, UNS8 nodeId*/
					nodeId, /*UNS8 nodeId*/
					0x1801, /*UNS16 index*/
					0x01, /*UNS8 subindex*/
					4, /*UNS8 count*/
					0, /*UNS8 dataType*/
					&TPDO_COBId,/*void *data*/
					CheckSDOAndContinue, /*SDOCallback_t Callback*/
                    0); /* use block mode */
		}			
		break;

		case 9: 
		{	
			UNS32 TPDO_COBId = 0x00000380 + nodeId;
			
			printf("Master : disable slave %2.2x TPDO 3 \n", nodeId);
			res = writeNetworkDictCallBack (d, /*CO_Data* d*/
					/**TestSlave_Data.bDeviceNodeId, UNS8 nodeId*/
					nodeId, /*UNS8 nodeId*/
					0x1802, /*UNS16 index*/
					0x01, /*UNS8 subindex*/
					4, /*UNS8 count*/
					0, /*UNS8 dataType*/
					&TPDO_COBId,/*void *data*/
					CheckSDOAndContinue, /*SDOCallback_t Callback*/
                    0); /* use block mode */
		}
		break;			

		case 10: 
		{	
			UNS32 TPDO_COBId = 0x00000480 + nodeId;
			
			printf("Master : disable slave %2.2x TPDO 4 \n", nodeId);
			res = writeNetworkDictCallBack (d, /*CO_Data* d*/
					/**TestSlave_Data.bDeviceNodeId, UNS8 nodeId*/
					nodeId, /*UNS8 nodeId*/
					0x1803, /*UNS16 index*/
					0x01, /*UNS8 subindex*/
					4, /*UNS8 count*/
					0, /*UNS8 dataType*/
					&TPDO_COBId,/*void *data*/
					CheckSDOAndContinue, /*SDOCallback_t Callback*/
                    0); /* use block mode */
		}			
		break;			
		
		case 11:
			/* Put the master in operational mode */
			setState(d, Operational);
			  
			/* Ask slave node to go in operational mode */
			masterSendNMTstateChange (d, nodeId, NMT_Start_Node);
			
	}
			
}
/**
 * @brief 在主站进入preoperation时可以对从站进行配置，
 * 和从站重新上电后再次配置等效
 * 
 * @param d 
 */
void TestMaster_preOperational(CO_Data* d)
{
	init_step = 0;
	printf("TestMaster_preOperational\n");
	ConfigureSlaveNode(&TestMaster_Data, 2);
	
}

void TestMaster_post_emcy(CO_Data* d, UNS8 nodeID, UNS16 errCode, UNS8 errReg, const UNS8 errSpec[5])
{
	printf("Master received EMCY message. Node: %2.2x  ErrorCode: %4.4x  ErrorRegister: %2.2x\n", nodeID, errCode, errReg);
}
void TestMaster_post_sync(CO_Data* d)
{
	printf("Master received sync\n");
}
void TestMaster_post_TPDO(CO_Data* d)
{
	printf("Master received TPDO\n");
}

static void blockSDO_callback(CO_Data* d, UNS8 nodeId)
{
	UNS32 abortCode;

	if(getWriteResultNetworkDict (d, nodeId, &abortCode) != SDO_FINISHED)
		printf("Master : Failed in initializing slave %2.2x, step %d, AbortCode :%4.4x \n", nodeId, init_step, abortCode);

	/* Finalise last SDO transfer with this node */
	closeSDOtransfer(&TestMaster_Data, nodeId, SDO_CLIENT);


}


//============================================================================
/* Callback function that check the write SDO demand */
void CheckWriteSDO(CO_Data* d, UNS8 nodeid)
{
	(void)d;
	UNS32 abortCode;

	if(getWriteResultNetworkDict(d, nodeid, &abortCode) != SDO_FINISHED)
		printf("\nResult : Failed in getting information for slave %2.2x, AbortCode :%4.4x \n", nodeid, abortCode);
	else
		printf("\nSend data OK\n");

	/* Finalize last SDO transfer with this node */
	closeSDOtransfer(d, nodeid, SDO_CLIENT);
}
/**
 * @brief 传输过程的回调函数
 * 
 * @param tell 
 */
void ftp_callback(int tell)
{
	printf("============transmited Bytes:%d===================\n", tell);
}
//============================================================================	
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
