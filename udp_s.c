
#include "network.h"

int main(void)
{
    int  sock, cc, no;
    unsigned char* p;
    char rmsg[12];

    char smsg[]="おめでとう．転送成功です．\r\n";
    struct addrinfo* cl_addr;
    
    sock = udp_server_socket(8000, &cl_addr);
    if (sock<0) Error("udp_server_socket");
 
    Loop{
        memset(rmsg, 0, 12);
        cc = udp_recv(sock, rmsg, 5, cl_addr);
        if (!strncmp("OK\r\n", rmsg, 4)) {
            fprintf(stderr, "connect!!\n");
            fflush(stderr); 
            udp_send(sock, smsg, 0, cl_addr);
            udp_send(sock, "\r\n", 0, cl_addr);
        }
    }
}

