#include "common.h"
#include "crps.h"

int main(int argc, char *argv[]) 
{
    int i = serverport;
    int kep_pid[serverport_end - serverport + 1] = {0};
    for (i = serverport; i <= serverport_end; ++i) {
        int tep = fork();
        if (tep < 0) {
            perror ("main: fork error");
        } else if (!tep) {
            signal (SIGCHLD, signal_handler);
            mainloop (i);
            exit (-1);
        }
        kep_pid[i - serverport] = tep;
    }

    while (1) {
        int tepid = wait (NULL);

        for (i = 0; i < serverport_end - serverport + 1; ++i) {
            if (kep_pid[i] == tepid) {

                int tepp = fork();
                if (tepp < 0) {
                    perror ("main: fork error");
                } else if (!tepp) {
                    mainloop (i + serverport);
                    exit (-1);
                }
                kep_pid[i] = tepp;
                break;
            }
        }
    }
    return 0;
}

int mainloop(int port) {
    int clientfd = 0, serverfd = 0;
    struct sockaddr_in client_addr;

    int fromfd = 0;
    struct sockaddr_in from_addr;

    serverfd = buildserver (port);

    size_t tep = sizeof (struct sockaddr);

    struct tcp_info info1,info2;
    int len = sizeof (struct tcp_info);
    int cnt_while=0;

    while (1) {
        while(1){
            getsockopt (clientfd, IPPROTO_TCP, TCP_INFO, &info1, (socklen_t *) &len);
            if (info1.tcpi_state != TCP_ESTABLISHED) {
                clientfd = accept (serverfd, (struct sockaddr*) &client_addr, (socklen_t*) &tep);
                if(clientfd ==-1 && errno==EWOULDBLOCK ){
                    continue;
                }else{
                    perror ("accept error..");
                    sleep(4);
                }
            }else{
                break;
            }
            
        }

        tep = sizeof (struct sockaddr);

        while (1){
            getsockopt (fromfd, IPPROTO_TCP, TCP_INFO, &info2, (socklen_t *) &len);
            if (info2.tcpi_state != TCP_ESTABLISHED) {
                fromfd = accept (serverfd, (struct sockaddr*) &from_addr, (socklen_t*) &tep);                
                if(fromfd ==-1 && errno==EWOULDBLOCK ){
                    
                }else{
                    perror ("accept error..");
                    sleep(4);
                }               
            }else{
                break;
            }
            
        }

        getsockopt (clientfd, IPPROTO_TCP, TCP_INFO, &info1, (socklen_t *) &len);
        getsockopt (fromfd, IPPROTO_TCP, TCP_INFO, &info2, (socklen_t *) &len);

        if (info1.tcpi_state == TCP_ESTABLISHED && info2.tcpi_state == TCP_ESTABLISHED){
            build_conn (clientfd, fromfd);
        }
        sleep(3);
    }
    close(serverfd);
    close(clientfd);
    close(fromfd);

    return 0;
}
