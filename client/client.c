#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>

enum { LIMIT = 1024 };
char buffer[LIMIT];
void get_messaage(int argc, char** argv) {
    int it_buffer = 0;
    for (int i = 3; i < argc; ++i) {
        int cur_len = strlen(argv[i]);
        for (int j = 0; j < cur_len; ++j) {
            buffer[it_buffer++] = argv[i][j];
        }
        buffer[it_buffer++] = ' ';
    }
}

int create_connection(char* node, char* service) {
    struct addrinfo *res = NULL;
    int gai_err;
    struct addrinfo hint = {
            .ai_family = AF_UNSPEC,
            .ai_socktype = SOCK_STREAM,
    };
    if (gai_err = getaddrinfo(node, service, &hint, &res)) {
        fprintf(stderr, "gai error: %s\n", gai_strerror(gai_err));
        return -1;
    }
    int sock = -1;
    for (struct addrinfo *ai = res; ai; ai = ai->ai_next) {
        sock = socket(ai->ai_family, ai->ai_socktype, 0);
        if (sock < 0) {
            perror("socket");
            continue;
        }
        if (connect(sock, ai->ai_addr, ai->ai_addrlen) < 0) {
            perror("connect");
            close(sock);
            sock = -1;
            continue;
        }
        break;
    }
    freeaddrinfo(res);
    return sock;
}

int main(int argc, char* argv[]) {
    char node[LIMIT];
    char service[LIMIT];
    int iter_node = 0, iter_service = 0, is_serv = 0;
    for (int i = 0; i < strlen(argv[1]); ++i) {
        if (argv[1][i] == ':') {
            node[iter_node++] = '\0';
            is_serv = 1;
            continue;
        }
        if (is_serv == 0) {
            node[iter_node++] = argv[1][i];
        } else {
            service[iter_service++] = argv[1][i];
        }
    }
    service[iter_service++] = '\0';
    int sockfd = create_connection(node, service);
    get_messaage(argc, argv);

    write(sockfd, buffer, sizeof(buffer));
    char buff[LIMIT];
    recv(sockfd, buff, sizeof(buff), 0);
    printf("%s\n", buff);
    close(sockfd);
    exit(0);
}
