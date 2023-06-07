#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdio.h>
#include <netdb.h>

enum { LIMIT = 1024 };

typedef struct ParsedCmd {
    char *cmd_name;
    char **args;
} ParsedCmd;

ParsedCmd parsedCmd;

void parse_args(char* buffer, int length) {
    char* args[LIMIT];
    int iter = 0;
    args[0] = calloc(LIMIT, sizeof(args[0]));
    int it_len = 0;
    for (int i = 0; i < length; ++i) {
        if (buffer[i] == ' ') {
            args[iter][it_len++] = '\0';
            iter++;
            args[iter] = calloc(LIMIT, sizeof(args[iter]));
            it_len = 0;
        } else {
            args[iter][it_len++] = buffer[i];
        }
    }
    args[iter][it_len++] = '\0';
    for (int i = iter + 1; i < LIMIT; ++i) {
        args[iter] = NULL;
    }
    parsedCmd.cmd_name = args[0];
    parsedCmd.args = args;
}

void process_client(int client_socket, int server_socket) {
    pid_t pid = fork();
    if (pid == 0) {
        dup2(client_socket, STDOUT_FILENO);
        execvp(parsedCmd.cmd_name, parsedCmd.args);
        exit(0);
    } else {
        dup2(server_socket, STDOUT_FILENO);
        waitpid(pid, 0, 0);
        close(server_socket);
    }
}

int create_listener(char* service) {
    struct addrinfo *res = NULL;
    int gai_err;
    struct addrinfo hint = {
            .ai_family = AF_UNSPEC,
            .ai_socktype = SOCK_STREAM,
            .ai_flags = AI_PASSIVE,
            };
    if (gai_err = getaddrinfo(NULL, service, &hint, &res)) {
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
        if (bind(sock, ai->ai_addr, ai->ai_addrlen) < 0) {
            perror("bind");
            close(sock);
            sock = -1;
            continue;
        }
        if (listen(sock, SOMAXCONN) < 0) {
            perror("listen");
            close(sock);
            sock = -1;
            continue;
        }
        break;
    }
    freeaddrinfo(res);
    return sock;
}

int main(int argc, char** argv) {
    int sock = create_listener(argv[1]);
    for (;;) {
        int connection = accept(sock, NULL, NULL);
        char buffer[LIMIT];
        read(connection, buffer, LIMIT);
        parse_args(buffer, LIMIT);
        process_client(connection, sock);
    }
}
