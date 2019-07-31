/*                                            __
 *    _      ____  __  (_|_)___ _____  ____ _/ /_____ _____
 *   | | /| / / / / / / / / __ `/ __ \/ __ `/ __/ __ `/ __ \
 *   | |/ |/ / /_/ / / / / /_/ / / / / /_/ / /_/ /_/ / /_/ /
 *   |__/|__/\____/_/ /_/\____/_/ /_/\____/\__/\____/\____/
 *               /___/              /____/
 *
 *  ---------------------------------------------------------------------------
 *
 *  文件: main.c
 *  描述: CirNetComLayer示例代码
 *
 *
 *  创建: 2019-7-30
 *  作者: Tao
 *  说明: 通过网络方式通讯，作为CIR3.0 CAN总线通讯的备份
 *
 *  修改: 2019-7-30
 *  作者: Tao
 *  说明:
 *
 *  ---------------------------------------------------------------------------
 */

#include <stdio.h>
#include "CirNetComLayer.h"

netcom_t netcom;

void NetCom_OnDataReceived(uint8_t srcId,uint8_t dstId, uint8_t funId,uint8_t subFunId, int dataLen,uint8_t *buf)
{
    int i;
    printf("srcId:%x, dstID:%x, funID:%x, subFunId:%x, dataLen:%d, buf: ",
           srcId, dstId, funId, subFunId, dataLen);

    for(i = 0; i < dataLen; i++)
    {
        printf("0x%02X ", buf[i]);
    }

    printf("\n");

}


int main()
{
    printf("--------CirNetComLayer----------\n");

    netcom.netcom_receive_handle = NetCom_OnDataReceived;

    system("route add -net 224.0.0.0 netmask 224.0.0.0 eth0");

    netcom_init(&netcom);
    netcom_start(&netcom);

    uint8_t data[6] = {0x31, 0x32, 0x33, 0x34, 0x35, 0x36};

    while(1)
    {
        //netcom_send(0x31, 0x32, 0x33, 0x34, 6, data);
        sleep(1);
    }

    return 0;
}
