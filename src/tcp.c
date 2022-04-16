#include "tcp.h"

#include <string.h>
#include <limits.h>
#include <arpa/inet.h>

descriptor_t tcp_create_acceptor(uint16_t port) {
    descriptor_t sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        return -1;
    }

    sockaddr_in_t serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(port);

    if (bind(sock, (sockaddr_t *) &serv_addr, sizeof(serv_addr)) < 0)
        return -1;

    if (listen(sock, INT_MAX) < 0)
        return -1;

    return sock;
}

descriptor_t tcp_accept_session(descriptor_t sock) {
    sockaddr_storage_t session_addr;
    socklen_t s_size = sizeof(session_addr);

    descriptor_t session = accept(sock, (sockaddr_t *) &session_addr, &s_size);
    if (session < 0)
        return -1;

    return session;
}

int tcp_create_connector(client_stream_t *dst, const char *addr, uint16_t port) {
    memset(&dst->serv_addr, 0, sizeof(dst->serv_addr));
    dst->serv_addr.sin_family = AF_INET;
    dst->serv_addr.sin_port = htons(port);
    dst->fd = socket(AF_INET, SOCK_STREAM, 0);
    if (inet_pton(AF_INET, addr, &dst->serv_addr.sin_addr) <= 0) {
        return -1;
    }

    return 0;
}

int tcp_connect(const client_stream_t *s) {
    return connect(s->fd, (sockaddr_t*)&s->serv_addr, sizeof(s->serv_addr));
}