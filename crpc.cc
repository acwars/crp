#include "common.h"

int main(int argc, char *argv[])
{

    if (argc != 4) {

        cout << "USAGE:\nclient serverip  serverport  localport  " << endl;
        cout << "-- serverip  (your server's ip)" << endl;
        cout << "-- serverport (which server port you want to connect to server,this should equal the other side's port   eg: ssh  root@serverip -p serverport)" << endl;
        cout << "-- which port you want to expose,for ssh it should be 22" << endl;

        return 0;
    }

    signal (SIGCHLD, signal_handler);

    struct sockaddr_in serveraddr;

    serveraddr.sin_addr.s_addr = inet_addr (argv[1]);
    serveraddr.sin_port = htons (atoi (argv[2]));
    serveraddr.sin_family = AF_INET;
    bzero (& (serveraddr.sin_zero), sizeof (serveraddr.sin_zero));

    struct sockaddr_in localaddr;

    localaddr.sin_addr.s_addr = inet_addr ("127.0.0.1");
    localaddr.sin_port = htons (atoi (argv[3]));
    localaddr.sin_family = AF_INET;
    bzero (& (localaddr.sin_zero), sizeof (localaddr.sin_zero));

    int sock_server=0, sock_local=0;

    getnewsocket(sock_server);
    getnewsocket(sock_local);

    struct tcp_info info1,info2;
    int len = sizeof (info1);
    char tep_char;

    while (1) {
        while (1){
            getsockopt (sock_server, IPPROTO_TCP, TCP_INFO, &info1, (socklen_t *) &len);
            if (info1.tcpi_state != TCP_ESTABLISHED  ) {
                int ttt=connect (sock_server, (struct sockaddr*) &serveraddr, sizeof (struct sockaddr));
                if (ttt ) {
                    if (errno == EISCONN){
                        getnewsocket(sock_server);
                    }else{
                        perror ("connect to server error..");
                    }
                    sleep(4);
                }
            }else break;
        }

        while (1){
            getsockopt (sock_local, IPPROTO_TCP, TCP_INFO, &info2, (socklen_t *) &len);
            if (info2.tcpi_state != TCP_ESTABLISHED  ) {

                int ttt=connect (sock_local, (struct sockaddr*) &localaddr, sizeof (struct sockaddr));
                if (ttt) {
                     if (errno == EISCONN){
                        getnewsocket(sock_local);
                    }else{
                         perror ("connect to localport error..");
                    }
                    sleep(4);
                }
            }else break;
            
        }

        getsockopt (sock_server, IPPROTO_TCP, TCP_INFO, &info1, (socklen_t *) &len);
        getsockopt (sock_local, IPPROTO_TCP, TCP_INFO, &info2, (socklen_t *) &len);

        if (info1.tcpi_state == TCP_ESTABLISHED && info2.tcpi_state == TCP_ESTABLISHED){
            build_conn (sock_server, sock_local);
        }
        sleep (3);
    }
    close(sock_local);
    close(sock_server);

    return 0;
}
