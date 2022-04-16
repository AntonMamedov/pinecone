#pragma once

#include <stdint.h>
#include <sys/socket.h>
#include <netinet/in.h>

typedef int descriptor_t;
typedef uint8_t byte_t;

typedef struct sockaddr_in sockaddr_in_t;
typedef struct sockaddr_storage sockaddr_storage_t;
typedef struct sockaddr sockaddr_t;


typedef struct client_stream {
    descriptor_t fd;
    sockaddr_in_t serv_addr;
} client_stream_t;

descriptor_t tcp_create_acceptor(uint16_t port);

descriptor_t tcp_accept_session(descriptor_t sock);

int tcp_create_connector(client_stream_t *dst, const char *addr, uint16_t port);

int tcp_connect(const client_stream_t *s);