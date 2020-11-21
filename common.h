#include <errno.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <malloc.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <iostream>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <netinet/tcp.h>
#include <sys/time.h>

#include <vector>
#include <map>
#include <algorithm>

#define bufflen 2048*6

#define serverport 7001
#define serverport_end 7050

#define server_configport 7000

using namespace std;

typedef struct client_info {
    int sockfd;
    struct sockaddr_in addr;
} client_info;

typedef vector<client_info>::iterator vec_client_iter;
typedef map<int, vector<int> >::iterator map_iv_iter;
typedef map<int, struct sockaddr_in>::iterator map_iaddr_iter;

enum ops {connect_clients, disconnect_clients, showips_clients};

void show_time(){
    char now[64];
    time_t tt=time(NULL);
    struct tm *ttime;
    
    ttime = localtime(&tt);
    strftime(now,64,"%Y-%m-%d %H:%M:%S",ttime);
    cout << now<<endl;
}

void signal_handler (int signo) {
    if (signo == SIGCHLD) {
        pid_t pid;
        while ((pid = waitpid (-1, NULL, WNOHANG)) > 0) {
            cout << ("SIGCHLD pid ") << pid << endl;
        }
    }
}

void getnewsocket(int &fd){
    if (fd>0) close(fd); 
    if ((fd = socket (AF_INET, SOCK_STREAM, 0)) == -1) {
        perror ("couldn't build socket..");
        exit (-1);
    }
}

int setnoblock(int &m_sock){
    int flags = fcntl(m_sock, F_GETFL, 0);
    return fcntl(m_sock, F_SETFL, flags|O_NONBLOCK);
}

int getsockstatus(int sd){
    int optval;
    socklen_t optlen = sizeof(int);

    getsockopt(sd, SOL_SOCKET, SO_ERROR,(char*) &optval, &optlen);
    return optval;
}

int buildserver (int port) {
    int sockfd;

    struct sockaddr_in my_addr;

    if ((sockfd = socket (AF_INET, SOCK_STREAM, 0)) == -1) {
        perror ("couldn't build socket..");
        exit (-1);
    }


    my_addr.sin_family = AF_INET;
    my_addr.sin_port = htons (port);
    my_addr.sin_addr.s_addr = htons (INADDR_ANY);
    bzero (& (my_addr.sin_zero), sizeof (my_addr.sin_zero));

    int optval = 1;
    if (setsockopt (sockfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof (optval)) < 0) {
        perror ("setsockopt1 error");
    }

    if (bind (sockfd, (struct sockaddr*) &my_addr, sizeof (struct sockaddr)) == -1) {
        perror ("bind error:");
        exit (-1);
    }

    listen (sockfd, 5);

    return sockfd;
}


int transfer (int src_sock, int to_sock) {
    char buff[bufflen];
    int len = bufflen;
    int i;
    int getlen = 0, sendlen = 0;

    getlen = recv (src_sock, buff, len, 0);

    if (getlen > 0) {

        int retry=0;
        while (1){
            sendlen = send (to_sock, buff, getlen, 0);
            if (sendlen > 0){
                return sendlen;
            }else if(sendlen == -1 && errno == EWOULDBLOCK && retry <= 10){
                ++retry;
                continue;  
            }else{
                perror ("transfer:send error ");
                return -1;
            }
        }


    } else if (getlen == -1 && errno == EWOULDBLOCK){
        return 0;
    } else {
        perror ("transfer:recv error");
        return -1;
    }
}


int build_conn (int sock1, int sock2) {
    int tep = fork();
    if (tep < 0) {
        perror ("fork error");
        return -1;
    } else if (!tep) {
        while (transfer (sock1, sock2) != -1);
        exit (-1);
    } 

    int tep2=fork();
    if (tep2 < 0) {
        perror ("fork error");
        return -1;
    } else if (!tep2) {
        while (transfer (sock2, sock1) != -1);
        exit (-1);
    } 

    while(waitpid (tep, NULL, WNOHANG)==0 && waitpid (tep2, NULL, WNOHANG)==0) sleep(3);
    //if (!waitpid (tep, NULL, WNOHANG)) 
        kill (tep, SIGKILL);
    //if (!waitpid (tep2, NULL, WNOHANG)) 
        kill (tep2, SIGKILL);

    return tep;
}
