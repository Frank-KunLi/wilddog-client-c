/*
 * wilddog_esp8266.c
 *
 *  Created on: 2015-8-25 -- baikal.hu
 *				
 */
#ifndef WILDDOG_PORT_TYPE_ESP	
#include <stdio.h>
#endif
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include "wilddog_port.h"
#include "wilddog.h"
#include "wilddog_config.h"
#include "wilddog_endian.h"
#include "wilddog_common.h"
#include "utlist.h"
#include "test_lib.h"

#include "espconn.h"
#include "ets_sys.h"
#include "os_type.h"
#include "osapi.h"
#include "mem.h"


extern struct espconn socket;
extern os_timer_t test_timer1;
extern int dns_flag;


ip_addr_t address;


struct recv_buf_node
{
	struct recv_buf_node *next;
	char *buf;
	unsigned short len;
};

struct recv_buf_node *head;


STATIC void FAR
recv_list_init(void)
{
	head = wmalloc(sizeof(struct recv_buf_node));
	head->buf = NULL;
	head->next = NULL;
	head->len = 0;

}

STATIC void FAR
recv_list_deinit(void)
{
    struct recv_buf_node *tmp = head;

    while(head != NULL)
    {
        tmp = head->next;
        wfree(head->buf);
        wfree(head);
        head = tmp;
    }
}


STATIC void FAR
send_cb(void *arg)
{
    struct espconn *pespconn = arg;

    wilddog_debug("send_cb\n");
}

STATIC void FAR
recv_cb(void *arg, char *buf, unsigned short len)
{
    struct espconn *pespconn = arg;
	struct recv_buf_node *node;

	int i;
	printf("recv_cb buf\n");
	for(i = 0; i < len; i++)
	{
		printf("0x%x  ", *(buf+i));
	}
	printf("\n\n");

    wilddog_debug("recv_cb\n");
	wilddog_debug("recv len:%d\n", len);
	if(head->next == NULL && head->len == 0)
	{
		wilddog_debug("first recv node\n");
		head->buf = wmalloc(len);
		head->len = len;
		memcpy(head->buf, buf, len);
	}
	else
	{
		wilddog_debug("add recv node\n");
		node = wmalloc(sizeof(struct recv_buf_node));
		node->len = len;
		node->buf = wmalloc(len);
        node->next = NULL;
		memcpy(node->buf, buf, len);
		LL_APPEND(head, node);
		
	}
}


void FAR dns_found(const char *name, ip_addr_t *ipaddr, void *arg)
{
	struct espconn *pespconn = (struct espconn *)arg;
	if(ipaddr == NULL)
	{
		printf("user_esp_platform_dns_found NULL\n");
	}

	if (ipaddr != NULL)
	{
		os_printf("user_esp_platform_dns_found %d.%d.%d.%d\n",
			*((uint8 *)&ipaddr->addr), *((uint8 *)&ipaddr->addr + 1),
			*((uint8 *)&ipaddr->addr + 2), *((uint8 *)&ipaddr->addr + 3));

		wilddog_debug("add:  0x%x\n", ipaddr);
		memcpy(&(address.addr), &ipaddr->addr, 4);
		dns_flag = TRUE;
	}

	
    espconn_regist_recvcb(pespconn, recv_cb);
    espconn_regist_sentcb(pespconn, send_cb);
	os_printf("dns found end\n");
}


int FAR gethost()
{
    int ret;	

	wilddog_debug("add:  0x%x\n", &address);
	ret = espconn_gethostbyname(&socket, "s-dal5-coap-1.wilddogio.com", &address, dns_found);
	os_printf("gethost end\n");
	os_timer_arm(&test_timer1, 1000, 0);

}

int FAR wilddog_gethostbyname( Wilddog_Address_T* addr, char* host )
{
	int ret;	

	wilddog_debug("wilddog_gethostbyname");
	wilddog_debug("addr: %d %d %d %d\n", *((uint8 *)&(address.addr)), *((uint8 *)&(address.addr) + 1),
			*((uint8 *)&(address.addr) + 2), *((uint8 *)&(address.addr) + 3));
	memcpy(addr->ip, &(address.addr), 4);
	wilddog_debug("ip:%d %d %d %d\n", addr->ip[0], addr->ip[1], addr->ip[2], addr->ip[3]);
	if(addr->ip[0] != 0 && addr->ip[1] != 0 && addr->ip[2] != 0 && addr->ip[3] != 0)
	{
		wilddog_debug("get host by name had succuss\n");
		return 0;
	}
	else
	{
		wilddog_debug("get host by name had failed\n");
		return -1;
	}
}

int FAR wilddog_openSocket( int* socketId )
{
	socket.type = ESPCONN_UDP;
    socket.proto.udp = (esp_udp *)os_zalloc(sizeof(esp_udp));
    socket.proto.udp->local_port = espconn_port();   
	socket.proto.udp->remote_port = 5683;

	recv_list_init();
	//socket.proto.udp = &udp;
	socket.type = ESPCONN_UDP;
	socket.state = ESPCONN_NONE;

    //espconn_regist_recvcb(&socket, recv_cb);
	
	if(espconn_create(&socket) != 0)
	{
	    recv_list_deinit();
		return -1;
	}
	
     *socketId = (int) (&socket);
	printf("open socket success\n");
    return 0;

}

int FAR wilddog_closeSocket( int socketId )
{
    espconn_delete( (struct espconn*) socketId );
	wfree(socket.proto.udp);
    if ( socketId )
    {
        socketId = 0;
    }

    recv_list_deinit();
    return 0;
}

int FAR wilddog_send
    ( 
    int socketId, 
    Wilddog_Address_T* addr_in, 
    void* tosend, 
    s32 tosendLength 
    )
{
	int ret;
	struct espconn *socket = (struct espconn*) socketId;

	wilddog_debug("remote ip:%d %d %d %d\n", addr_in->ip[0], addr_in->ip[1], addr_in->ip[2], addr_in->ip[3]);
	memcpy(socket->proto.udp->remote_ip, addr_in->ip, 4);
	wilddog_debug("length:%d\n", tosendLength);
	int i;
	for(i = 0; i < tosendLength; i++)
	{
		printf("0x%x  ", *((char *)tosend+i));
	}
	printf("\n\n");
	
	ret = espconn_sent(socket, tosend, tosendLength);
	wilddog_debug("espconn_sent ret:%d\n", ret);

}
int FAR wilddog_receive
    ( 
    int socketId, 
    Wilddog_Address_T* addr_in, 
    void* buf, 
    s32 bufLen, 
    s32 timeout
    )
{
	struct recv_buf_node *tmp;
	int len;
	tmp = head;
	s32 count = timeout;

	while(count > 0)
	{
		if(head->len == 0)
		{
			os_delay_us(10000);
		}
		else
		{
			len = head->len;
			if(head->next != NULL)
			{
				wilddog_debug("recv the head len:%d\n", head->len);
			 	memcpy(buf, head->buf, head->len);
				
				head = head->next;
				wfree(tmp->buf);
				wfree(tmp);
			}
			else
			{
				wilddog_debug("2recv the head len:%d\n", head->len);
				memcpy(buf, head->buf, head->len);
				wfree(head->buf);
				head->buf = NULL;
				head->next = NULL;
				head->len = 0;
			}
            break;
		}
		count -= 10;
	}

	return len;
}
