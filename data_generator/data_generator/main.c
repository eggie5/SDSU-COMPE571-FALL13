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
#define OVERHEAD 39 //approx overhead for sendto return/usecond for this env. MAX is 40ish
#define SRV_IP "130.191.3.100"
#define PORT 32351

int main(int argc, char** argv)
{
    
    int r=256; //mbps form terminal arg
    int t=5; //seconds form terminal arg
    
    //socket stuff
    struct sockaddr_in si_other;
    int s, i, slen=sizeof(si_other);
    char buf[BUFLEN];
    
    if ((s=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP))==-1)
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
            if (sendto(s, buf, BUFLEN, 0, (struct sockaddr *)&si_other, slen)==-1)
                    printf("Error: sendto()\n");
            
            usleep(sleep_time);
        }
    }
    
    close(s);
    printf("exiting main\n");
    return 0;
}
