#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/prctl.h>
#include <signal.h>
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

pid_t child[10];
void sig_handler(int signum) {
    printf("\n%d Received signal %d\n", getpid(), signum);
     
    printf("KILL %d\n", child[0]);
    kill(child[0],SIGKILL);

    printf("KILL %d\n", child[1]);
    kill(child[1],SIGKILL);
}

void sig_die(int signum) {
    printf("\n%d Received signal %d\n", getpid(), signum);
    printf("\n%d Killed myself\n", getpid());
    exit(0);
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
    int on = 1;

    int status = setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR,
                           (const char *) &on, sizeof(on));
    if (-1 == status) 
        die("setsockopt(...,SO_REUSEADDR,...)");
    
    /* bind serv information to mysocket */
    bind(sockfd, (struct sockaddr *)&serv, sizeof(struct sockaddr));
    listen(sockfd, 5);

    int conn;
    int i;
    for (i=0; i<process_num; i++) { 
        printf("Starting %d %d ", i, getpid());
        if (! (child[i] = fork()) ) {
            signal(SIGINT, sig_die);
            prctl(PR_SET_PDEATHSIG, SIGINT);
            while ( (conn = accept(sockfd, (struct sockaddr *)&dest, &socksize) ) >= 0 ) {
                printf("%d --- recieve data\n", getpid());
                copyData(conn, 1);
                printf("%d --- done\n", getpid());
                close(conn);
            }
            if (conn < 0) {
                perror("accept");
                return 1;
            }
        }
        printf("PID %d\n", child[i]);
    }
    printf("Working\n");

    signal(SIGINT, sig_handler);
    signal(SIGTERM, sig_handler);
    
    pid_t wpid;
    status = 0;
    while ((wpid = wait(&status)) > 0);

    //while(wait() > 0) { /* no-op */ ; }
    close(sockfd);
    printf("COMPLETED\n");

    return 0;
}

