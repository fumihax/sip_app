
#include "network.h"

int main(int argc, char** argv)
{
    int  sock, cc;
    char rmsg[128];
    char smsg[]="OK\r\n";
    struct addrinfo* sv_addr;

    if (argc<3) {
        print_message("Usage... %s server port\n", argv[0]);
        exit(1);
    }

    sock = udp_client_socket(argv[1], atoi(argv[2]), &sv_addr);
    if (sock<0) Error("udp_client_socket");
 
    bzero(rmsg, 128);

    cc = udp_send(sock, smsg, 5, sv_addr);
    if (cc<0) Error("udp_send");

    cc = udp_recv(sock, rmsg, 128, sv_addr);
    while (rmsg[0]!='\r' || rmsg[1]!='\n') {
        printf("%s",rmsg);
        cc = udp_recv(sock, rmsg, 128, sv_addr);
    }

    socket_close(sock);
}


