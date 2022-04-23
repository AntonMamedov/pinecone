#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <getopt.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>

#include "tcp.h"

#define HELP_OPTION_STRING "pinecone [--help] [-p | --port <port>]\n"\
                           "[-a | --addr <server address, default 127.0.0.1>]\n"\
                           "[--ping <check connection>]\n"
#define NO_PORT_STRING "NO PORT"
#define DEFAULT_PINECONE_PORT "DEFAULT_PINECONE_PORT"
#define DEFAULT_PINECONE_HOST "127.0.0.1"
#define KB 1024
#define SIZE_UINT64 8

enum options {
    HELP = 1 << 0,
    PING = 1 << 2,
    PORT = 1 << 3,
    ADDR = 1 << 4,
};

static const struct option LONG_OPTS[] = {
        {"help", no_argument,       NULL, HELP},
        {"ping", no_argument,       NULL, PING},
        {"port", required_argument, NULL, PORT},
        {"addr", required_argument, NULL, ADDR}
};

static const char *SHORT_OPTS = "ha:p:";
static const byte_t PING_MSG[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 'P', 'I', 'N', 'G'};
static const byte_t PONG_MSG[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 'P', 'O', 'N', 'G'};

typedef struct option_data {
    int port;
    bool ping;
    const char *addr;
} option_data_t;

int main(int argc, char *argv[]) {
    if (argc <= 1) {
        printf(HELP_OPTION_STRING);
        exit(0);
    }
    option_data_t opts = {-1, false, NULL};
    int opts_val = 0;
    while (opts_val != -1) {
        opts_val = getopt_long(argc, argv, SHORT_OPTS, LONG_OPTS, NULL);
        switch (opts_val) {
            case HELP:
            case 'h':
                printf(HELP_OPTION_STRING);
                exit(0);
            case PORT:
            case 'p':
                opts.port = atoi(optarg);
                break;
            case ADDR:
            case 'a':
                opts.addr = optarg;
                break;
            case PING:
                opts.ping = true;
                break;
        }
    }

    if (opts.port < 0) {
        char *post_str = getenv(DEFAULT_PINECONE_PORT);
        opts.port = atoi(post_str);
        if (post_str == NULL || opts.port < 0) {
            printf(NO_PORT_STRING);
            exit(0);
        }
    }

    if (opts.addr == NULL) {
        opts.addr = DEFAULT_PINECONE_HOST;
    }

    client_stream_t clent;
    if (tcp_create_connector(&clent, opts.addr, opts.port) > 0) {
        exit(1);
    }

    if (tcp_connect(&clent) > 0) {
        exit(1);
    }

    if (opts.ping) {
        size_t writes_byte = write(clent.fd, PING_MSG, sizeof(PING_MSG) - 1);
        if (writes_byte < sizeof(PING_MSG) - 1)
            exit(1);
        char buffer[sizeof(PONG_MSG) - 1] = {0};
        size_t read_byte = read(clent.fd, buffer, sizeof(PONG_MSG) - 1);
        if (read_byte < sizeof(PONG_MSG) - 1)
            exit(1);

        if (strncmp(PONG_MSG, buffer, sizeof(PONG_MSG) - 1) != 0) {
            printf("");
        }
        exit(0);
    }

    size_t buffer_size = KB + SIZE_UINT64 + 1;
    byte_t *buffer = malloc(buffer_size);
    byte_t *current_buffer_pos = buffer + SIZE_UINT64;
    size_t bytes_in_buffer = SIZE_UINT64;

    while (1) {
        int bytes_read = read(stdin, current_buffer_pos, KB);
        bytes_in_buffer += bytes_read != EOF ? bytes_read : 0;
        if (bytes_read == EOF)
            break;
        buffer_size *= 2;
        buffer = realloc(buffer, buffer_size);
        if (buffer == NULL) {
            exit(1);
        }
        current_buffer_pos += bytes_in_buffer;
    }

    size_t msg_size = bytes_in_buffer;
    int current_msg_size_byte = SIZE_UINT64 - 1;

    while (msg_size != 0 && current_msg_size_byte >= 0) {
        buffer[current_msg_size_byte] = msg_size % UINT8_MAX;
        msg_size /= UINT8_MAX;
        current_msg_size_byte--;
    }

    if (current_msg_size_byte < 0) {
        exit(1);
    }

    size_t writes_byte = write(clent.fd, buffer, bytes_in_buffer);

    if (writes_byte < bytes_in_buffer)
        exit(1);
    close(clent.fd);
    return 0;
}