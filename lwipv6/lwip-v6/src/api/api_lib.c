/*   This is part of LWIPv6
 *   Developed for the Ale4NET project
 *   Application Level Environment for Networking
 *
 *   Copyright 2004,2011 Renzo Davoli University of Bologna - Italy
 *   
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License along
 *   with this program; if not, write to the Free Software Foundation, Inc.,
 *   51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */   
/*
 * Copyright (c) 2001-2004 Swedish Institute of Computer Science.
 * All rights reserved. 
 * 
 * Redistribution and use in source and binary forms, with or without modification, 
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission. 
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED 
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF 
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT 
 * SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, 
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT 
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING 
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY 
 * OF SUCH DAMAGE.
 *
 * This file is part of the lwIP TCP/IP stack.
 * 
 * Author: Adam Dunkels <adam@sics.se>
 *
 */

/* This is the part of the API that is linked with
   the application */

#include "lwip/opt.h"
#include "lwip/api.h"
#include "lwip/api_msg.h"
#include "lwip/memp.h"


struct
netbuf *netbuf_new(void)
{
  struct netbuf *buf;

  buf = memp_malloc(MEMP_NETBUF);
  if (buf != NULL) {
    buf->p = NULL;
    buf->ptr = NULL;
    return buf;
  } else {
    return NULL;
  }
}

void
netbuf_delete(struct netbuf *buf)
{
  if (buf != NULL) {
    if (buf->p != NULL) {
      pbuf_free(buf->p);
      buf->p = buf->ptr = NULL;
    }
    memp_free(MEMP_NETBUF, buf);
  }
}

void *
netbuf_alloc(struct netbuf *buf, u16_t size)
{
  /* Deallocate any previously allocated memory. */
  if (buf->p != NULL) {
    pbuf_free(buf->p);
  }
  buf->p = pbuf_alloc(PBUF_TRANSPORT, size, PBUF_RAM);
  if (buf->p == NULL) {
     return NULL;
  }
  buf->ptr = buf->p;
  return buf->p->payload;
}

void
netbuf_free(struct netbuf *buf)
{
  if (buf->p != NULL) {
    pbuf_free(buf->p);
  }
  buf->p = buf->ptr = NULL;
}

void
netbuf_ref(struct netbuf *buf, void *dataptr, u16_t size)
{
  if (buf->p != NULL) {
    pbuf_free(buf->p);
  }
  buf->p = pbuf_alloc(PBUF_TRANSPORT, 0, PBUF_REF);
  buf->p->payload = dataptr;
  buf->p->len = buf->p->tot_len = size;
  buf->ptr = buf->p;
}

void
netbuf_chain(struct netbuf *head, struct netbuf *tail)
{
  pbuf_chain(head->p, tail->p);
  head->ptr = head->p;
  memp_free(MEMP_NETBUF, tail);
}

u16_t
netbuf_len(struct netbuf *buf)
{
  return buf->p->tot_len;
}

err_t
netbuf_data(struct netbuf *buf, void **dataptr, u16_t *len)
{
  if (buf->ptr == NULL) {
    return ERR_BUF;
  }
  *dataptr = buf->ptr->payload;
  *len = buf->ptr->len;
  return ERR_OK;
}

s8_t
netbuf_next(struct netbuf *buf)
{
  if (buf->ptr->next == NULL) {
    return -1;
  }
  buf->ptr = buf->ptr->next;
  if (buf->ptr->next == NULL) {
    return 1;
  }
  return 0;
}

void
netbuf_first(struct netbuf *buf)
{
  buf->ptr = buf->p;
}

void
netbuf_copy_partial(struct netbuf *buf, void *dataptr, u16_t len, u16_t offset)
{
  struct pbuf *p;
  u16_t i, left;

  left = 0;

  if(buf == NULL || dataptr == NULL) {
    return;
  }
  
  /* This implementation is bad. It should use bcopy
     instead. */
  for(p = buf->p; left < len && p != NULL; p = p->next) {
    if (offset != 0 && offset >= p->len) {
      offset -= p->len;
    } else {    
			for(i = offset; i < p->len; ++i) {
				((char *)dataptr)[left] = ((char *)p->payload)[i];
				if (++left >= len) {
					return;
				}
      }
      offset = 0;
    }
  }
}

void
netbuf_copy(struct netbuf *buf, void *dataptr, u16_t len)
{
  netbuf_copy_partial(buf, dataptr, len, 0);
}

struct ip_addr *
netbuf_fromaddr(struct netbuf *buf)
{
  /*printf("netbuf_fromaddr %x:%x:%x:%x\n",
		  buf->fromaddr.addr[0],
		  buf->fromaddr.addr[1],
		  buf->fromaddr.addr[2],
		  buf->fromaddr.addr[3]); */

  return &(buf->fromaddr);
}

u16_t
netbuf_fromport(struct netbuf *buf)
{
  /*printf("netbuf_fromport %d %x\n",buf->fromport,buf);*/
  return buf->fromport;
}

struct
netconn *netconn_new_with_proto_and_callback(struct stack *stack, enum netconn_type t, u16_t proto,
                                   void (*callback)(struct netconn *, enum netconn_evt, u16_t len))
{
  struct netconn *conn;
  struct api_msg msg;

  conn = memp_malloc(MEMP_NETCONN);
  if (conn == NULL) {
    return NULL;
  }
  
  conn->stack = stack;
  
  conn->err = ERR_OK;
  conn->type = t;
  conn->pcb.tcp = NULL;

  if ((conn->mbox = sys_mbox_new()) == SYS_MBOX_NULL) {
    memp_free(MEMP_NETCONN, conn);
    return NULL;
  }
  /*conn->recvmbox = SYS_MBOX_NULL;*/
  if ((conn->recvmbox = sys_mbox_new()) == SYS_MBOX_NULL) {
    memp_free(MEMP_NETCONN, conn);
    return NULL;
  }
  conn->connected = 0;
  conn->acceptmbox = SYS_MBOX_NULL;
  conn->sem = SYS_SEM_NULL;
  conn->state = NETCONN_NONE;
  conn->socket = 0;
  conn->callback = callback;
  conn->recv_avail = 0;

  msg.type = API_MSG_NEWCONN;
  msg.msg.msg.bc.port = proto; /* misusing the port field */
  msg.msg.conn = conn;
  
  api_msg_post(conn->stack, &msg);  
  
  sys_mbox_fetch(conn->mbox, NULL);
  if ( conn->err != ERR_OK ) {
    memp_free(MEMP_NETCONN, conn);
    return NULL;
  }

  return conn;
}


struct
netconn *netconn_new(struct stack *stack, enum netconn_type type)
{
  return netconn_new_with_proto_and_callback(stack, type,0,NULL);
}

struct
netconn *netconn_new_with_callback(struct stack *stack, enum netconn_type type,
                                   void (*callback)(struct netconn *, enum netconn_evt, u16_t len))
{
  return netconn_new_with_proto_and_callback(stack, type,0,callback);
}


err_t
netconn_delete(struct netconn *conn)
{
  struct api_msg msg;
  void *mem;
	//fprintf(stderr, "netconn_delete %p\n",conn);
  
  if (conn == NULL) {
    return ERR_OK;
  }
  
  msg.type = API_MSG_DELCONN;
  msg.msg.conn = conn;
  
  api_msg_post(conn->stack, &msg);  
  
  sys_mbox_fetch(conn->mbox, NULL);

  /* Drain the recvmbox. */
  if (conn->recvmbox != SYS_MBOX_NULL) {
    while (sys_arch_mbox_fetch(conn->recvmbox, &mem, 1) != SYS_ARCH_TIMEOUT) {
      if (conn->type == NETCONN_TCP) {
				if (mem != NULL)
					pbuf_free((struct pbuf *)mem);
      } else {
				netbuf_delete((struct netbuf *)mem);
      }
    }
    sys_mbox_free(conn->recvmbox);
    conn->recvmbox = SYS_MBOX_NULL;
  }

  /* Drain the acceptmbox. */
  if (conn->acceptmbox != SYS_MBOX_NULL) {
    while (sys_arch_mbox_fetch(conn->acceptmbox, &mem, 1) != SYS_ARCH_TIMEOUT) {
      netconn_delete((struct netconn *)mem);
    }
    
    sys_mbox_free(conn->acceptmbox);
    conn->acceptmbox = SYS_MBOX_NULL;
  }

  sys_mbox_free(conn->mbox);
  conn->mbox = SYS_MBOX_NULL;
  if (conn->sem != SYS_SEM_NULL) {
    sys_sem_free(conn->sem);
  }
  
  conn->sem = SYS_SEM_NULL; /* this should not be commented out!*/
  memp_free(MEMP_NETCONN, conn);
  return ERR_OK;
}

enum netconn_type
netconn_type(struct netconn *conn)
{
  return conn->type;
}

struct stack *netconn_stack(struct netconn* conn)
{
	return conn->stack;
}

err_t
netconn_peer(struct netconn *conn, struct ip_addr *addr, u16_t *port)
{
	struct api_msg msg;
	err_t olderr = conn->err;

	if (conn == NULL) {
		return ERR_VAL;
	}
	msg.type = API_MSG_PEER;
  msg.msg.conn = conn;
  msg.msg.err = ERR_OK;
	msg.msg.msg.bp.ipaddr = addr;
	msg.msg.msg.bp.port = port;

	switch (conn->type) {
		case NETCONN_RAW:
#if LWIP_PACKET
		case NETCONN_PACKET_RAW:
		case NETCONN_PACKET_DGRAM:
#endif
			/* return an error as connecting is only a helper for upper layers */
			return ERR_CONN;
		case NETCONN_UDPLITE:
		case NETCONN_UDPNOCHKSUM:
		case NETCONN_UDP:
			api_msg_post(conn->stack, &msg);
			sys_mbox_fetch(conn->mbox, NULL);
			return msg.msg.err;
		default:
			return ERR_ARG;
	}
}

err_t
netconn_addr(struct netconn *conn, struct ip_addr *addr, u16_t *port)
{
	struct api_msg msg;

	if (conn == NULL) { 
		return ERR_VAL;
	}         

	msg.type = API_MSG_PEER;
  msg.msg.conn = conn;
  msg.msg.err = ERR_OK;
	msg.msg.msg.bp.ipaddr = addr;
	msg.msg.msg.bp.port = port;

	switch (conn->type) {
		case NETCONN_PACKET_RAW:
		case NETCONN_PACKET_DGRAM:
			return ERR_CONN;
		case NETCONN_RAW:
		case NETCONN_UDPLITE:
		case NETCONN_UDPNOCHKSUM:
		case NETCONN_UDP:
		case NETCONN_TCP:
			api_msg_post(conn->stack, &msg);
			sys_mbox_fetch(conn->mbox, NULL);
			return msg.msg.err;
		default:
			return ERR_ARG;
	}
}

err_t
netconn_bind(struct netconn *conn, struct ip_addr *addr,
      u16_t port)
{
	struct api_msg msg;

  if (conn == NULL) {
    return ERR_VAL;
  }

  if (conn->type != NETCONN_TCP &&
     conn->recvmbox == SYS_MBOX_NULL) {
    if ((conn->recvmbox = sys_mbox_new()) == SYS_MBOX_NULL) {
      return ERR_MEM;
    }
  }
  
  msg.type = API_MSG_BIND;
  msg.msg.conn = conn;
  msg.msg.msg.bc.ipaddr = addr;
  msg.msg.msg.bc.port = port;
  
  api_msg_post(conn->stack, &msg);
  
  sys_mbox_fetch(conn->mbox, NULL);
  return conn->err;
}


err_t
netconn_connect(struct netconn *conn, struct ip_addr *addr,
       u16_t port)
{
	struct api_msg msg;
  
  if (conn == NULL) {
    return ERR_VAL;
  }

  if (conn->recvmbox == SYS_MBOX_NULL) {
    if ((conn->recvmbox = sys_mbox_new()) == SYS_MBOX_NULL) {
      return ERR_MEM;
    }
  }
  
  msg.type = API_MSG_CONNECT;
  msg.msg.conn = conn;  
  msg.msg.msg.bc.ipaddr = addr;
  msg.msg.msg.bc.port = port;
  
  api_msg_post(conn->stack, &msg);
  
  sys_mbox_fetch(conn->mbox, NULL);
	if (conn->err == ERR_OK)
		conn->connected = 1;

  return conn->err;
}

err_t
netconn_disconnect(struct netconn *conn)
{
	struct api_msg msg;
  
  if (conn == NULL) {
    return ERR_VAL;
  }

  msg.type = API_MSG_DISCONNECT;
  msg.msg.conn = conn;  
  
  api_msg_post(conn->stack, &msg);
  
  sys_mbox_fetch(conn->mbox, NULL);
	if (conn->err == ERR_OK)
		conn->connected = 0;
  return conn->err;

}

err_t
netconn_listen(struct netconn *conn)
{
	struct api_msg msg;

  if (conn == NULL) {
    return ERR_VAL;
  }

  if (conn->acceptmbox == SYS_MBOX_NULL) {
    conn->acceptmbox = sys_mbox_new();
    if (conn->acceptmbox == SYS_MBOX_NULL) {
      return ERR_MEM;
    }
  }
  
  msg.type = API_MSG_LISTEN;
  msg.msg.conn = conn;
  
  api_msg_post(conn->stack, &msg);
  
  sys_mbox_fetch(conn->mbox, NULL);
  return conn->err;
}

struct netconn *
netconn_accept(struct netconn *conn)
{
  struct netconn *newconn;
  
  if (conn == NULL) {
    return NULL;
  }
  
  sys_mbox_fetch(conn->acceptmbox, (void **)&newconn);
  /* Register event with callback */
  if (conn->callback)
      (*conn->callback)(conn, NETCONN_EVT_RCVMINUS, 0);
  
	if (newconn != NULL)
		newconn->connected = 1;
  return newconn;
}

struct netbuf *
netconn_recv(struct netconn *conn)
{
	struct api_msg msg;
  struct netbuf *buf;
  struct pbuf *p;
  u16_t len;
    
  if (conn == NULL) {
    return NULL;
  }
  
	/*printf("netconn_recv %p %p\n",conn,conn->recvmbox);*/
  if (conn->recvmbox == SYS_MBOX_NULL) {
    conn->err = ERR_CONN;
    return NULL;
  }

  if (conn->err != ERR_OK) {
    return NULL;
  }

  if (conn->type == NETCONN_TCP) {
		/* RD1013 do not inspect pcb. use conn->connected instead */
		if (conn->connected == 0)
	 	{
      conn->err = ERR_CONN;
      return NULL;
    }

    buf = memp_malloc(MEMP_NETBUF);

    if (buf == NULL) {
      conn->err = ERR_MEM;
      return NULL;
    }
    
    sys_mbox_fetch(conn->recvmbox, (void **)&p);

    if (p != NULL)
    {
        len = p->tot_len;
        conn->recv_avail -= len;
    }
    else
        len = 0;
    
    /* Register event with callback */
      if (conn->callback)
        (*conn->callback)(conn, NETCONN_EVT_RCVMINUS, len);

    /* If we are closed, we indicate that we no longer wish to receive
       data by setting conn->recvmbox to SYS_MBOX_NULL. */
    if (p == NULL) {
      memp_free(MEMP_NETBUF, buf);
      sys_mbox_free(conn->recvmbox);
      conn->recvmbox = SYS_MBOX_NULL;
      return NULL;
    }

    buf->p = p;
    buf->ptr = p;
    buf->fromport = 0;
    /*buf->addr.fromaddr = NULL;*/

    /* Let the stack know that we have taken the data. */
    msg.type = API_MSG_RECV;
    msg.msg.conn = conn;
    if (buf != NULL) {
      msg.msg.msg.len = buf->p->tot_len;
    } else {
      msg.msg.msg.len = 1;
    }
    api_msg_post(conn->stack, &msg);

    sys_mbox_fetch(conn->mbox, NULL);
  } else {
    sys_mbox_fetch(conn->recvmbox, (void **)&buf);
		conn->recv_avail -= buf->p->tot_len;
    /* Register event with callback */
    if (conn->callback)
        (*conn->callback)(conn, NETCONN_EVT_RCVMINUS, buf->p->tot_len);
  }

  

    
  LWIP_DEBUGF(API_LIB_DEBUG, ("netconn_recv: received %p (err %d) len %d\n", (void *)buf, conn->err, buf->p->tot_len));


  return buf;
}

err_t
netconn_send(struct netconn *conn, struct netbuf *buf)
{
  struct stack *stack = conn->stack;
  
	struct api_msg msg;

  if (conn == NULL) {
    return ERR_VAL;
  }

	/*
  if (conn->err != ERR_OK) {
    return conn->err;
  }*/

  LWIP_DEBUGF(API_LIB_DEBUG, ("netconn_send: sending %d bytes\n", buf->p->tot_len));
  msg.type = API_MSG_SEND;
  msg.msg.conn = conn;
  msg.msg.msg.p = buf->p;
	api_msg_post(stack, &msg);

  sys_mbox_fetch(conn->mbox, NULL);
  return conn->err;
}

err_t
netconn_write(struct netconn *conn, void *dataptr, u16_t size, u8_t copy)
{
  struct stack *stack = conn->stack;
  
	struct api_msg msg;
  u16_t len;
  
  if (conn == NULL) {
    return ERR_VAL;
  }

  if (conn->err != ERR_OK) {
    return conn->err;
  }
  
  if (conn->sem == SYS_SEM_NULL) {
    conn->sem = sys_sem_new(0);
    if (conn->sem == SYS_SEM_NULL) {
      return ERR_MEM;
    }
  }

  msg.type = API_MSG_WRITE;
  msg.msg.conn = conn;

  conn->state = NETCONN_WRITE;
  while (conn->err == ERR_OK && size > 0) {
    msg.msg.msg.w.dataptr = dataptr;
    msg.msg.msg.w.copy = copy;
    
    if (conn->type == NETCONN_TCP) {
			int avail;
      while ((avail=tcp_sndbuf(conn->pcb.tcp)) == 0) {
        sys_sem_wait(conn->sem);
        if (conn->err != ERR_OK) {
          goto ret;
        }
      }
      if (size > avail) {
         /* We cannot send more than one send buffer's worth of data at a
            time. */
         len = avail;
      } else {
         len = size;
      }
    } else {
      len = size;
    }
    
    LWIP_DEBUGF(API_LIB_DEBUG, ("netconn_write: writing %d bytes (%d)\n", len, copy));
    msg.msg.msg.w.len = len;
		api_msg_post(stack, &msg);
    sys_mbox_fetch(conn->mbox, NULL);    
    if (conn->err == ERR_OK) {
      dataptr = (void *)((char *)dataptr + len);
      size -= len;
    } else if (conn->err == ERR_MEM) {
      conn->err = ERR_OK;
      sys_sem_wait(conn->sem);
    } else {
      goto ret;
    }
  }
  
ret:
  conn->state = NETCONN_NONE;
	// X1004 /*
  if (conn->sem != SYS_SEM_NULL) {
    sys_sem_free(conn->sem);
    conn->sem = SYS_SEM_NULL;
  }
	// */
  
  return conn->err;
}

err_t
netconn_close(struct netconn *conn)
{
  struct stack *stack = conn->stack;
  
	struct api_msg msg;

  if (conn == NULL) {
    return ERR_VAL;
  }

  conn->state = NETCONN_CLOSE;
 again:
  msg.type = API_MSG_CLOSE;
  msg.msg.conn = conn;
	api_msg_post(stack, &msg);
  sys_mbox_fetch(conn->mbox, NULL);
  if (conn->err == ERR_MEM &&
     conn->sem != SYS_SEM_NULL) {
    sys_sem_wait(conn->sem);
    goto again;
  }
  conn->state = NETCONN_NONE;
  return conn->err;
}

err_t
netconn_callback(struct netconn *conn, err_t (*fun)(struct netconn *conn, void *), void *arg)
{
	struct api_msg msg;

	if (conn == NULL) {
		return ERR_VAL;
	}

	msg.type = API_MSG_CALLBACK;
	msg.msg.conn = conn;
	msg.msg.err = ERR_OK;
	msg.msg.msg.cb.fun = fun;
	msg.msg.msg.cb.arg = arg;

	api_msg_post(conn->stack, &msg);
	sys_mbox_fetch(conn->mbox, NULL);
	return msg.msg.err;
}

err_t
netconn_err(struct netconn *conn)
{
  return conn->err;
}

