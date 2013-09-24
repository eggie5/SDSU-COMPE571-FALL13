#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <stdlib.h>
#include <sys/wait.h>

#define BUFLEN 1514
#define MAX_PACKET 11776 //bits
#define OVERHEAD 40 //approx overhead for sendto return/usecond for this env. MAX is 40ish
#define SRV_IP "127.0.0.1"
#define PORT 8002

int main(int argc, char** argv)
{
    
    int r=2; //mbps form terminal arg
    int t=20; //seconds form terminal arg
    int packet_count=0;
    
    //socket stuff
    struct sockaddr_in si_other;
    int slen=sizeof(si_other);
    
    int _socket, i;

    char buf[BUFLEN];
    
    _socket=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (_socket==-1)
    {
        printf("Error: socket");
        exit(0);
    }
    memset((char *) &si_other, sizeof(si_other), 0);
    si_other.sin_family = AF_INET;
    si_other.sin_port = htons(PORT);
    if (inet_aton(SRV_IP, &si_other.sin_addr)==0)
    {
        fprintf(stderr, "inet_aton() failed\n");
        _exit(1);
    }
    
    int r_bits = r * pow(10, 6);

    int num_of_packets = r_bits / MAX_PACKET ;
    
    double sleep_time = (1 / (double)num_of_packets) * pow(10,6); //sleep in us
    sleep_time= sleep_time - OVERHEAD ;
    
    if(sleep_time <1)
    {
        printf("neg sleep time: %lf", sleep_time);
        return 0;
    }
    
    printf("%d / %d = %d, sleep=%lf\n", r_bits, MAX_PACKET, num_of_packets, sleep_time);
    
    for(i=0; i<t; i++)
    {
        for(int i=0; i< num_of_packets; i++)
        {
            if (sendto(_socket, buf, BUFLEN, 0, (struct sockaddr *)&si_other, slen)==-1)
                    printf("Error: sendto()\n");
            
            packet_count++;
            
            usleep(sleep_time);
        }
    }
    
    close(_socket);
    printf("exiting main\n");
    printf("packet_count=%d", packet_count);
    return 0;
}
