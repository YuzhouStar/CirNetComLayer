/*                                            __
 *    _      ____  __  (_|_)___ _____  ____ _/ /_____ _____
 *   | | /| / / / / / / / / __ `/ __ \/ __ `/ __/ __ `/ __ \
 *   | |/ |/ / /_/ / / / / /_/ / / / / /_/ / /_/ /_/ / /_/ /
 *   |__/|__/\____/_/ /_/\____/_/ /_/\____/\__/\____/\____/
 *               /___/              /____/
 *
 *  ---------------------------------------------------------------------------
 *
 *  文件: CirNetComLayer.c
 *  描述: 通过UDP组播方式实现网络收发数据
 *
 *
 *  创建: 2019-7-30
 *  作者: Tao
 *  说明: 通过网络方式通讯，作为CIR3.0 CAN总线通讯的备份
 *
 *  修改: 2019-7-30
 *  作者: Tao
 *  说明: 如果在ARM linux下运行出现setsockopt: No such device，则需要将广播地址加入路由表
 *        route add -net 224.0.0.0 netmask 224.0.0.0 eth0
 *
 *  ---------------------------------------------------------------------------
 */

#ifndef CIRNETCOMLAYER_H
#define CIRNETCOMLAYER_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <netdb.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <pthread.h>

typedef struct netcom_struct
{
    pthread_mutex_t mutex;
    int sockfd;
    struct sockaddr_in servaddr;

    int stop;
    void (*netcom_receive_handle)(uint8_t srcId,uint8_t dstId,
                                   uint8_t funId,uint8_t subFunId, int dataLen,uint8_t *buf);
} netcom_t;


void netcom_init(netcom_t *netcom);
void netcom_start(netcom_t *netcom);
void netcom_send(uint8_t srcId, uint8_t dstId, uint8_t funId, uint8_t subFunId, int dataLen, uint8_t *buf);


#endif // CIRNETCOMLAYER_H
