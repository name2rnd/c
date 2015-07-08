#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
//#include "sockutil.h"
void die(char *message) {
    perror(message);
    exit(1);
}

void copyData(int from, int to) {
    char buf[1024];
    int amount;
    while ((amount = read(from, buf, sizeof(buf))) > 0) {
        if (write(to, buf, amount) != amount) {
            die("write");
            return;
        }
        if (write(from, buf, amount) != amount) {
            die("write");
            return;
        }
    }
    if (amount < 0)
        die("read");
}

int main(int argc, char *argv[] ) {
    struct sockaddr_un address;
    int sock, conn;
    int process_num = 2;
    if (argc > 1) {
        if (atoi(argv[1]) <= 10 ) { 
            process_num = atoi(argv[1]);
        }
        else {
            printf("WARNING: max childs id 10\n");
        }
    }
    printf("STARTING WITH %d childs...\n", process_num);
    socklen_t addrLength;

    if((sock = socket(PF_UNIX, SOCK_STREAM, 0)) < 0) {
        perror("socket");
        return 1;
    }

    unlink("./sample-socket");
    address.sun_family = AF_UNIX;
    strcpy(address.sun_path, "./sample-socket");

    addrLength = sizeof(address.sun_family) +
                strlen(address.sun_path);

    if (bind(sock, (struct sockaddr *) &address, addrLength)) {
        printf("failed");
        return 0;
    }

    if (listen(sock, 5)) {
        perror("listen");
        return 1;
    }
    int i;
    pid_t child;
    for (i=0; i < process_num; i++) {
        printf("Starting child number %d\n", i);
        if (! (child = fork() ) ) { 
            while ( (conn = accept(sock, (struct sockaddr *) &address, &addrLength) ) >= 0 ) {
                printf("\nPID: %d\n", getpid());
                printf("---recieve data\n");
                copyData(conn, 1);
                printf("---done\n");
                close(conn);
            }
            if (conn < 0) {
                perror("accept");
                return 1;
            } 
            exit(0);
        }
        printf("PID %d\n", child);
    }

    close(sock);
    printf("COMPLETED\n");

    return 0;
}

