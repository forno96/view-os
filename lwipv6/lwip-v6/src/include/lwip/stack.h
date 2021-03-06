/*
 * Copyright (c) 2008 Renzo Davoli University of Bologna
 * 
 * Author: Renzo Davoli <renzo@cs.unibo.it>
 *
 */
#ifndef __LWIP_STACK_H__
#define __LWIP_STACK_H__

#include "lwip/pbuf.h"
#include "lwip/tcp.h"
#include "lwip/netif.h"
#include "lwip/ip_frag.h"
#include "lwip/tcpip.h"
#include <poll.h>

struct pbuf;

struct ip_route_list; /* needs def */
struct ip_addr_list; /* needs def */

#if IPv4_FRAGMENTATION || IPv6_FRAGMENTATION
struct ip_reassbuf; 
#endif

struct raw_pcb;

#define packet_pcb raw_pcb
struct udp_pcb;

struct tcp_pcb;  
struct tcp_seg; /* needs def */
struct tcp_hdr;
struct tcp_pcb;

/* IP_ROUTE_POOL_SIZE IP_ADDR_POOL_SIZE IP_REASS_POOL_SIZE need defs*/
#define NETIF_POS2FD(stack,fpos) ((stack)->numif_pfd[(fpos)].fd)

#define LWIP_STACK_FLAG_FORWARDING 0x1
#define LWIP_STACK_FLAG_USERFILTER 0x2
#define LWIP_STACK_FLAG_UF_NAT     0x10000

#if LWIP_USERFILTER
struct stack_userfilter;
#if LWIP_NAT
struct stack_nat;
#endif
#endif

/* Allows binding to TCP/UDP sockets below 1024 */
#define LWIP_CAP_NET_BIND_SERVICE 1<<10
/* Allow broadcasting, listen to multicast */
#define LWIP_CAP_NET_BROADCAST    1<<11
/* Allow interface configuration */
#define LWIP_CAP_NET_ADMIN        1<<12
/* Allow use of RAW sockets */
/* Allow use of PACKET sockets */
#define LWIP_CAP_NET_RAW          1<<13

struct stack {
	/* global */
	u32_t	stack_flags;
#if LWIP_CAPABILITIES
	lwip_capfun stack_capfun;
#endif

	/* lwip-v6/src/core/netif.c */
	struct netif *netif_list;
	sys_sem_t  netif_cleanup_mutex;
	int netif_pipe[2];

	/* lwip-v6/src/core/ipv6/ip6.c */
	u16_t ip_id;

	/* lwip-v6/src/core/ipv6/ip6_route.c */
	struct ip_route_list *ip_route_head;

#if IPv4_FRAGMENTATION || IPv6_FRAGMENTATION
	/* lwip-v6/src/core/ipv6/ip6_frag.c */
	struct ip_reassbuf ip_reassembly_pools[IP_REASS_POOL_SIZE];
#endif

	/* lwip-v6/src/core/raw.c */
	struct raw_pcb *raw_pcbs;

	/* lwip-v6/src/core/tcp_in.c */
	struct tcp_seg   inseg     ;
	struct tcp_hdr  *tcphdr    ;
	u32_t            seqno     ;
	u32_t            ackno     ;
	u8_t             flags     ;
	u16_t            tcplen    ;
	u8_t             recv_flags;
	struct pbuf     *recv_data ;
	struct tcp_pcb         *tcp_input_pcb;
	u16_t uniqueid;

	/* lwip-v6/src/core/packet.c */
	u16_t active_pfpacket;
	struct packet_pcb *packet_pcbs;

	/* lwip-v6/src/core/udp.c */
	struct udp_pcb        *udp_pcbs;
	struct udp_pcb *pcb_cache;

	/* lwip-v6/src/core/tcp.c */
	u32_t tcp_ticks;
	union tcp_listen_pcbs_t tcp_listen_pcbs;
	struct tcp_pcb *tcp_active_pcbs;  /* List of all TCP PCBs that are in a */
	struct tcp_pcb *tcp_tw_pcbs;      /* List of all TCP PCBs in TIME-WAIT. */
	u8_t tcp_timer;

	/* lwip-v6/src/api/tcpip.c */
	sys_mbox_t     stack_queue;
	sys_sem_t      tcpip_init_sem;
	tcpip_handler  tcpip_init_done;
	void *         tcpip_init_done_arg;
	sys_sem_t      tcpip_shutdown_sem;
	tcpip_handler  tcpip_shutdown_done;
	void *         tcpip_shutdown_done_arg;
	int tcpip_tcp_timer_active;

	/* lwip-v6/src/netif/loopif.c */
	int netif_num[NETIF_NUMIF];

#if LWIP_USERFILTER
	struct stack_userfilter *stack_userfilter;
#if LWIP_NAT
	struct stack_nat *stack_nat;
#endif
#endif

};
#endif
