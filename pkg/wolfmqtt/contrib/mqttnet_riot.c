/* mqttnet.c
 *
 * Copyright (C) 2006-2020 wolfSSL Inc.
 *
 * This file is part of wolfMQTT.
 *
 * wolfMQTT is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * wolfMQTT is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1335, USA
 */

/* Include the autoconf generated config.h */
#ifdef HAVE_CONFIG_H
    #include <config.h>
#endif

#include <sys/time.h>

#include "wolfmqtt/mqtt_client.h"
#include "wolfmqtt_examples/mqttexample.h"

#include "wolfmqtt_examples/mqttnet.h"

/* Setup defaults */
#ifndef SOCK_OPEN
    #define SOCK_OPEN       socket
#endif
#ifndef SOCKET_T
    #define SOCKET_T        int
#endif
#ifndef SOERROR_T
    #define SOERROR_T       int
#endif
#ifndef SELECT_FD
    #define SELECT_FD(fd)   ((fd) + 1)
#endif
#ifndef SOCKET_INVALID
    #define SOCKET_INVALID  ((SOCKET_T)0)
#endif
#ifndef SOCK_CONNECT
    #define SOCK_CONNECT    connect
#endif
#ifndef SOCK_SEND
    #define SOCK_SEND(s,b,l,f) send((s), (b), (size_t)(l), (f))
#endif
#ifndef SOCK_RECV
    #define SOCK_RECV(s,b,l,f) recv((s), (b), (size_t)(l), (f))
#endif
#ifndef SOCK_CLOSE
    #define SOCK_CLOSE      close
#endif
#ifndef SOCK_ADDR_IN
    #define SOCK_ADDR_IN    struct sockaddr_in
#endif
#ifdef SOCK_ADDRINFO
    #define SOCK_ADDRINFO   struct addrinfo
#endif


/* Local context for Net callbacks */
typedef enum {
    SOCK_BEGIN = 0,
    SOCK_CONN,
} NB_Stat;

typedef struct _SocketContext {
    SOCKET_T fd;
    NB_Stat stat;
    //SOCK_ADDR_IN addr;
    MQTTCtx* mqttCtx;
} SocketContext;

/* Private functions */

#ifndef WOLFMQTT_NO_TIMEOUT
static void setup_timeout(struct timeval* tv, int timeout_ms)
{
    (void) tv;
    (void) timeout_ms;
}

#ifdef WOLFMQTT_NONBLOCK
static void tcp_set_nonblocking(SOCKET_T* sockfd)
{

}
#endif /* WOLFMQTT_NONBLOCK */
#endif /* !WOLFMQTT_NO_TIMEOUT */

static int NetConnect(void *context, const char* host, word16 port,
    int timeout_ms)
{
    int rc = 0;

    (void) context;
    (void) host;
    (void) port;
    (void) timeout_ms;

    return rc;
}

#ifdef WOLFMQTT_SN
static int SN_NetConnect(void *context, const char* host, word16 port,
    int timeout_ms)
{
    int rc = 0;

    return rc;
}
#endif

static int NetWrite(void *context, const byte* buf, int buf_len,
    int timeout_ms)
{
    int rc = 0;

    (void) context;
    (void) buf;
    (void) buf_len;
    (void) timeout_ms;

    return rc;
}

static int NetRead_ex(void *context, byte* buf, int buf_len,
    int timeout_ms, byte peek)
{
    int rc = 0;

    (void) context;
    (void) buf;
    (void) buf_len;
    (void) timeout_ms;
    (void) peek;

    return rc;
}

static int NetRead(void *context, byte* buf, int buf_len, int timeout_ms)
{
    return NetRead_ex(context, buf, buf_len, timeout_ms, 0);
}

#ifdef WOLFMQTT_SN
static int NetPeek(void *context, byte* buf, int buf_len, int timeout_ms)
{
    return NetRead_ex(context, buf, buf_len, timeout_ms, 1);
}
#endif

static int NetDisconnect(void *context)
{
    (void) context;

    return 0;
}


/* Public Functions */
int MqttClientNet_Init(MqttNet* net, MQTTCtx* mqttCtx)
{
    (void) net;
    (void) mqttCtx;

    return MQTT_CODE_SUCCESS;
}

#ifdef WOLFMQTT_SN
int SN_ClientNet_Init(MqttNet* net, MQTTCtx* mqttCtx)
{
    if (net) {
        SocketContext* sockCtx;

        XMEMSET(net, 0, sizeof(MqttNet));
        net->connect = SN_NetConnect;
        net->read = NetRead;
        net->write = NetWrite;
        net->peek = NetPeek;
        net->disconnect = NetDisconnect;

        sockCtx = (SocketContext*)WOLFMQTT_MALLOC(sizeof(SocketContext));
        if (sockCtx == NULL) {
            return MQTT_CODE_ERROR_MEMORY;
        }
        net->context = sockCtx;
        XMEMSET(sockCtx, 0, sizeof(SocketContext));
        sockCtx->stat = SOCK_BEGIN;
        sockCtx->mqttCtx = mqttCtx;

    #if 0 /* TODO: add multicast support */
        MulticastContext* multi_ctx;
        multi_ctx = (MulticastContext*)WOLFMQTT_MALLOC(sizeof(MulticastContext));
        if (multi_ctx == NULL) {
            return MQTT_CODE_ERROR_MEMORY;
        }
        net->multi_ctx = multi_ctx;
        XMEMSET(multi_ctx, 0, sizeof(MulticastContext));
        multi_ctx->stat = SOCK_BEGIN;
    #endif

    #if defined(WOLFMQTT_MULTITHREAD) && defined(WOLFMQTT_ENABLE_STDIN_CAP)
        /* setup the pipe for waking select() */
        if (pipe(sockCtx->pfd) != 0) {
            PRINTF("Failed to set up pipe for stdin");
            return -1;
        }
    #endif
    }

    return MQTT_CODE_SUCCESS;
}
#endif

int MqttClientNet_DeInit(MqttNet* net)
{
    if (net) {
        if (net->context) {
            WOLFMQTT_FREE(net->context);
        }
        XMEMSET(net, 0, sizeof(MqttNet));
    }
    return 0;
}

int MqttClientNet_Wake(MqttNet* net)
{
    (void) net;

    return 0;
}

void DUMMY(void)
{
    setup_timeout(NULL, 0);
    NetConnect(NULL, NULL, 0, 0);
    NetWrite(NULL, NULL, 0, 0);
    NetRead(NULL, NULL, 0, 0);
    NetDisconnect(NULL);
}
