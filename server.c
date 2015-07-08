#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#define PORTNUM 2300
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

    int process_num = 2;
    if (argc > 1) {
        if (atoi(argv[1]) <= 10 ) { 
            process_num = atoi(argv[1]);
        }
        else {
            printf("WARNING: max childs id 10\n");
        }
    }
    
    struct sockaddr_in dest, serv;
    //int portnum = 50000;
    socklen_t socksize = sizeof(struct sockaddr_in);

    memset(&serv, 0, sizeof(serv));           /* zero the struct before filling the fields */
    serv.sin_family = AF_INET;
    serv.sin_addr.s_addr = htonl(INADDR_ANY); /* set our address to any interface */
    serv.sin_port = htons(PORTNUM);           /* set the server port number */    

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    /* bind serv information to mysocket */
    bind(sockfd, (struct sockaddr *)&serv, sizeof(struct sockaddr));
    listen(sockfd, 1);

    int conn;

    while ( (conn = accept(sockfd, (struct sockaddr *)&dest, &socksize) ) >= 0 ) {
        printf("Incoming connection from %s - sending welcome\n", inet_ntoa(dest.sin_addr));
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
    close(sockfd);
    printf("COMPLETED\n");

    return 0;
}

