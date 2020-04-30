#ifndef NET_H
#define NET_H

#include "platform.h"
#include "io.h"
#include "cnt.h"

typedef struct 
{
    INT fd;
    struct sockaddr_in addr;
}     * NET_ENDPOINT;

INT net_accept(NET_ENDPOINT p_srv, IO_FILE *p_cli);
INT net_server_socket(INT p_port, NET_ENDPOINT *p_ep);
INT net_read_header( IO_FILE, IO_BUFFER );
INT net_handle_request( CNT, IO_FILE );
#endif
