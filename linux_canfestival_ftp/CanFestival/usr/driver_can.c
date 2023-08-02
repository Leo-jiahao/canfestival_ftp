/**
 * @file driver_can.c
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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stddef.h>		/* for NULL */
#include <errno.h>

#include "config.h"


#include <unistd.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include "linux/can.h"
#include "linux/can/raw.h"

#ifndef __USE_MISC
#define __USE_MISC
#endif

#include "net/if.h"


#include "data.h"
#include "dcf.h"

#ifndef PF_CAN
#define PF_CAN 29
#endif

#ifndef AF_CAN
#define AF_CAN PF_CAN
#endif

#include "timers_driver.h"

#define CAN_IFNAME     "can%s"
#define CAN_SOCKET     socket
#define CAN_CLOSE      close
#define CAN_RECV       recv
#define CAN_SEND       send
#define CAN_BIND       bind
#define CAN_IOCTL      ioctl
#define CAN_ERRNO(err) errno
#define CAN_SETSOCKOPT setsockopt



#include "can_driver.h"


typedef struct
{
  pthread_t tid;
  int fd;
  void* d;
  char used;
} CANPort; /* taken from drivers/unix.c */


/**
 * CAN send routine
 * @param port CAN port
 * @param m CAN message
 * @return success or error
 */
UNS8 canSend(CAN_PORT port, Message *m)
{
    if(!port){
        return 1;
    }
    int res;
    struct can_frame frame;

    frame.can_id = m->cob_id;
    if (frame.can_id >= 0x800)
        frame.can_id |= CAN_EFF_FLAG;
    frame.can_dlc = m->len;
    if (m->rtr)
        frame.can_id |= CAN_RTR_FLAG;
    else
        memcpy (frame.data, m->data, 8);

    #if defined DEBUG_MSG_CONSOLE_ON
    MSG("out : ");
    print_message(m);
    #endif
    res = CAN_SEND (((CANPort*)port)->fd, &frame, sizeof (frame), 0);
    
    if (res < 0){
        fprintf (stderr, "Send failed: %s\n", strerror (CAN_ERRNO (res)));
        return 1;
    }

    return 0;
}

/**
 * @brief 
 * 
 * CAN Receiver Task
 * @param port CAN port
 */
void canReceiveLoop(CO_Data * d)
{
    Message m;
    int res;
    struct can_frame frame;
    while (((CANPort*)d->canHandle)->used) {
        res = CAN_RECV(((CANPort*)d->canHandle)->fd, &frame, sizeof(frame), MSG_WAITALL);
        if (res < 0)
        {
            fprintf (stderr, "Recv failed: %s\n", strerror (CAN_ERRNO (res)));
            continue;
        }
        // printf("frame_id:0X%4X, dlc:%d ", frame.can_id, frame.can_dlc);
        // for (int i = 0; i < frame.can_dlc; i++)
        // {
        //     printf("0X%2X ",frame.data[i]);
        //     /* code */
        // }
        // printf("\n");
        
        m.cob_id = frame.can_id & CAN_EFF_MASK;
        m.len = frame.can_dlc;
        if (frame.can_id & CAN_RTR_FLAG)
            m.rtr = 1;
        else
            m.rtr = 0;
        memcpy (m.data, frame.data, 8);   

        EnterMutex();
        canDispatch(d, &m);
        LeaveMutex();
    }
}

/**
 * CAN open routine
 * @param board device name and baudrate
 * @param d CAN object data
 * @return valid CAN_PORT pointer or NULL
 */
CAN_PORT canOpen(s_BOARD *board, CO_Data * d)
{
    struct ifreq ifr;
    struct sockaddr_can addr;
    int err;
    int ret;
    CANPort *fd0 = (CANPort *)malloc (sizeof (CANPort));


    if(!fd0)
    {
        return NULL;
    }

    fd0->fd = CAN_SOCKET (PF_CAN, SOCK_RAW, CAN_RAW);
    if (fd0->fd < 0)
    {
        fprintf (stderr, "Socket creation failed: %s\n",
            strerror (CAN_ERRNO (fd0.fd)));
        goto error_ret;
    }

    if (*board->busname >= '0' && *board->busname <= '9')
        snprintf (ifr.ifr_name, IFNAMSIZ, CAN_IFNAME, board->busname);
    else
        strncpy (ifr.ifr_name, board->busname, IFNAMSIZ);
    err = CAN_IOCTL (fd0->fd, SIOCGIFINDEX, &ifr);
    if (err)
        {
        fprintf (stderr, "Getting IF index for %s failed: %s\n",
            ifr.ifr_name, strerror (CAN_ERRNO (err)));
        goto error_close;
        }
    
    {
        int loopback = 1;
        err = CAN_SETSOCKOPT(fd0->fd, SOL_CAN_RAW, CAN_RAW_LOOPBACK,
                &loopback, sizeof(loopback));
        if (err) {
            fprintf(stderr, "rt_dev_setsockopt: %s\n", strerror (CAN_ERRNO (err)));
            goto error_close;
        }
    }
  
  
    addr.can_family = AF_CAN;
    addr.can_ifindex = ifr.ifr_ifindex;
    err = CAN_BIND (fd0->fd, (struct sockaddr *) &addr, sizeof (addr));
    if (err)
    {
        fprintf (stderr, "Binding failed: %s\n", strerror (CAN_ERRNO (err)));
        goto error_close;
    }
    d->canHandle = fd0;

    fd0->used = 1;

    ret = pthread_create(&fd0->tid, NULL, canReceiveLoop, d);

    if(ret){
        fprintf (stderr, "pthread_create:%s\n", strerror (CAN_ERRNO (err)));
        goto error_close;
    }
    return fd0;

error_close:
    CAN_CLOSE (fd0->fd);

error_ret:
    free (fd0);
    return NULL;
}

/**
 * CAN close routine
 * @param d CAN object data
 * @return success or error
 */
int canClose(CO_Data * d)
{
    CANPort *fd0 = d->canHandle;
    if (fd0)
    {
        fd0->used = 0;
        CAN_CLOSE (fd0->fd);

        WaitReceiveTaskEnd(&fd0->tid);

        free (d->canHandle);

        d->canHandle = NULL;
    }
  return 0;
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


#ifdef __KERNEL__
EXPORT_SYMBOL (canOpen);
EXPORT_SYMBOL (canClose);
EXPORT_SYMBOL (canSend);
EXPORT_SYMBOL (canChangeBaudRate);
#endif


