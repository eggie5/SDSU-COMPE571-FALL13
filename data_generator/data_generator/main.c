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
#include <sys/mman.h>
#include <sys/types.h>

#define BUFLEN 1514
#define MAX_PACKET 11776 //bits -- 1.4KB
#define OVERHEAD 35 //approx overhead for sendto return/usecond for this env. MAX is 40ish
#define SRV_IP "127.0.0.1"
#define PORT 32000
#define MICRO_PER_SECOND	1000000
#define MILLION 1000000
//30 us is the min

static int *glob_var;

int main(int argc, char** argv)
{
    //he wants 350 max
    //200mbps seems to be my max on 1 process
    int r=201; //mbps form terminal arg
    int t=10; //seconds form terminal arg
    int packet_count=0;
    
    printf("r=%d t=%d\n", r,t);
    
    //socket stuff
    struct sockaddr_in si_other;
    int slen=sizeof(si_other);
    
    int _socket;

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
    
    
    //App. logic
    int r_bits = r * MILLION;
    
    int workers = ceil(r / (float)200);
    printf("launching %d worker(s)\n", workers);

    int num_of_packets = r_bits / MAX_PACKET ;
    num_of_packets = num_of_packets/workers; //packets/worker
    
    double sleep_time = (1 / (double)num_of_packets) * MILLION; //sleep in us
    sleep_time= sleep_time - OVERHEAD ;
    
    if(sleep_time <1)
    {
        printf("neg sleep time: %lf, exiting", sleep_time);
        return 0;
    }
    if(sleep_time<30)
    {
        printf("WARNING: sleeep time is < 30us -- this percission is not reliable\n");
    }
    
    printf("%d / %d = %d, sleep=%lf\n", r_bits, MAX_PACKET, num_of_packets, sleep_time);
    
    for(int i=0; i<workers; i++)
    {
        if (fork()==0)
        {
            printf("child started: pid=%d\n", getpid());
            struct timeval start_time;
            struct timeval stop_time;
            float drift;
            gettimeofday( &start_time, NULL );
            
            //do work
            for(;;)
            {
                if (sendto(_socket, buf, BUFLEN, 0, (struct sockaddr *)&si_other, slen)==-1)
                    printf("Error: sendto()\n");
                    
                packet_count++;
                usleep(sleep_time);
                
                gettimeofday( &stop_time, NULL );
                
                drift = (float)(stop_time.tv_sec  - start_time.tv_sec);
                drift += (stop_time.tv_usec - start_time.tv_usec)/(float)MICRO_PER_SECOND;
                
                if(drift>=(double) t)
                    break;
                
            }
            
            printf("child closed: %d - %d packets sent\n", getpid(), packet_count);
            return 0;
        }
        else
        {
            //printf("i am parent, pid=%d\n", getpid());
        }
    }
    
    printf("waiting for children\n");
    close(_socket);
    wait(NULL);
    
    printf("exiting main\n");
    return 0;

}
