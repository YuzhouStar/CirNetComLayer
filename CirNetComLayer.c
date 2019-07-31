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

#include "CirNetComLayer.h"

#define GROUP_ADDR  "230.0.0.1"
#define GROUP_PORT  3344
#define LOCAL_PORT  3345

#define BUFFER_SIZE 2048

//#define _DEBUG_LOG_

netcom_t netcom_tx;

//初始化组播收发
int init_rx(netcom_t *netcom)
{
    struct in_addr ia;

    unsigned int socklen;
    struct hostent *group;
    struct ip_mreq mreq;

    netcom->sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if(netcom->sockfd < 0)
    {
        printf("[CirNetComLayer.Rx] socket creating error\n");
        exit(0);
    }

    bzero(&mreq, sizeof(struct ip_mreq));
    if((group = gethostbyname(GROUP_ADDR)) == (struct hostent*)0)
    {
        printf("[CirNetComLayer.Rx] gethostbyname error\n");
        exit(0);
    }

    bcopy((void*)group->h_addr, (void*)&ia, group->h_length);
    bcopy(&ia, &mreq.imr_multiaddr.s_addr, sizeof(struct in_addr));

    mreq.imr_interface.s_addr = htonl(INADDR_ANY);

    if(setsockopt(netcom->sockfd, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(struct ip_mreq)) == -1)
    {
        printf("[CirNetComLayer.Rx] setsockopt error\n");
        exit(0);
    }

    socklen = sizeof(struct sockaddr_in);
    memset(&(netcom->servaddr), 0, socklen);

    netcom->servaddr.sin_family = AF_INET;
    netcom->servaddr.sin_port = htons(GROUP_PORT);

    if(inet_pton(AF_INET, GROUP_ADDR, &(netcom->servaddr.sin_addr)) < 0)
    {
        printf("[CirNetComLayer.Rx] IP address error\n");
        exit(0);
    }

    if(bind(netcom->sockfd, (struct sockaddr*)&(netcom->servaddr), sizeof(struct sockaddr_in)) == -1)
    {
        printf("[CirNetComLayer.Rx] bind error\n");
        exit(0);
    }

    return 0;
}

int init_tx(netcom_t *netcom)
{
    struct sockaddr_in cliaddr;
    unsigned int socklen;

    if((netcom->sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
    {
        printf("[CirNetComLayer.Tx] socket creating error\n");
        exit(0);
    }

    socklen = sizeof(struct sockaddr_in);

    memset(&(netcom->servaddr), 0, socklen);
    netcom->servaddr.sin_family = AF_INET;
    netcom->servaddr.sin_port = htons(GROUP_PORT);

    if(inet_pton(AF_INET, GROUP_ADDR, &(netcom->servaddr.sin_addr)) < 0)
    {
        printf("[CirNetComLayer.Tx] Converting IP address error\n");
        exit(0);
    }

    memset(&cliaddr, 0, socklen);
    cliaddr.sin_family = AF_INET;
    cliaddr.sin_port = htons(LOCAL_PORT);
    cliaddr.sin_addr.s_addr = INADDR_ANY;

    if(bind(netcom->sockfd, (struct sockaddr*)&cliaddr, sizeof(struct sockaddr_in)) == -1)
    {
        printf("[CirNetComLayer.Tx] bind error\n");
        exit(0);
    }

    return 0;
}

static void * thread_proc(void *arg)
{
    netcom_t *netcom = arg;
    printf("[CirNetComLayer] %s: %d\n", GROUP_ADDR, GROUP_PORT);

    uint8_t rx_buffer[BUFFER_SIZE];

    int n;
    unsigned int socklen;

    while(netcom->stop == 0)
    {
        //接收组播消息
        bzero(rx_buffer, BUFFER_SIZE);
        n = recvfrom(netcom->sockfd, rx_buffer, BUFFER_SIZE - 1, 0,(struct sockaddr*)&(netcom->servaddr), &socklen);
        if(n < 0)
        {
            printf("[CirNetComLayer.Thread] recvfrom error\n");
            exit(0);
        }
        else
        {
            rx_buffer[n + 1] = 0;

#ifdef _DEBUG_LOG_
            printf ("[Rx] len: %d, %s\n",n, (char *)rx_buffer);
#endif

            if(netcom->netcom_receive_handle)
            {
                netcom->netcom_receive_handle(rx_buffer[0], rx_buffer[1], rx_buffer[2], rx_buffer[3],  n - 4, &rx_buffer[4]);
            }
        }

    }

    return NULL;
}


#ifdef _DEBUG_LOG_
static void * thread_proc_tx(void *arg)
{
    netcom_t *netcom = arg;

    printf("[CirNetComLayer.tx] %s: %d\n", GROUP_ADDR, GROUP_PORT);

    uint8_t data[4] = {0x41, 0x54, 0x0D, 0x0A};

    while(netcom->stop == 0)
    {
        netcom_send(0x11, 0x05, 0xA1, 0x02, 4, data);
        sleep(1);
    }

    return NULL;
}
#endif

void netcom_init(netcom_t *netcom)
{
    netcom->sockfd = -1;
    netcom->stop = 0;
    pthread_mutex_init(&(netcom->mutex), NULL);

    init_rx(netcom);
    init_tx(&netcom_tx);
}

void netcom_start(netcom_t *netcom)
{
    pthread_t ntid;
    pthread_create(&ntid, NULL, thread_proc, netcom);

#ifdef _DEBUG_LOG_
    pthread_t ntid_tx;
    pthread_create(&ntid_tx, NULL, thread_proc_tx, netcom);
#endif

}

void netcom_send(uint8_t srcId, uint8_t dstId, uint8_t funId, uint8_t subFunId, int dataLen, uint8_t *buf)
{
    pthread_mutex_lock(&(netcom_tx.mutex));

    int len = 4 + dataLen;
    uint8_t data[len];

    data[0] = srcId;
    data[1] = dstId;
    data[2] = funId;
    data[3] = subFunId;

    memcpy(data + 4, buf, dataLen);

    if(sendto(netcom_tx.sockfd, data, len, 0, (struct sockaddr*)&(netcom_tx.servaddr), sizeof(struct sockaddr_in)) < 0)
    {
        printf("[CirNetComLayer.Thread] sendto error\n");
        exit(0);
    }

    pthread_mutex_unlock(&(netcom_tx.mutex));
}
