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
#include <sys/time.h>

#define BUFLEN 1514
#define MAX_PACKET 11776 //bits -- 1.4KB
#define OVERHEAD 40 //approx overhead for sendto return/usecond for this env. MAX is 40ish
#define SRV_IP "127.0.0.1"
#define PORT 32000
#define MICRO_PER_SECOND 1000000
#define MILLION 1000000
#define PROCESS_MBIT_THRESHOLD 180
#define SLEEP_MIN 30

void usage()
{
    printf("\n");
    printf("\n -r : Mbps ");
    printf("\n -t : Trial time ");
    
    printf("\n\n");
    
}


int main(int argc, char** argv)
{
    int i;
    //he wants 350 max
    //200mbps seems to be my max on 1 process
    int r; //mbps form terminal arg
    int t; //seconds form terminal arg
    //int workers=6;
    int packet_count=0;
    int overhead=OVERHEAD;
    
    
    if (argc < 3 )
    {
        usage();
        exit(0);
    }
    else
	{
        for (i=1; i < argc ; i+=2)
        {
            if (argv[i][0] == '-')
            {
                switch(argv[i][1])
                {
                    case 'r':
                        r = atoi(argv[i+1]);
                        break;
                    case 't':
                        t = atoi(argv[i+1]);
                        break;
                    default: 
                        return(0);
                        break;
                }
                
            }
        }
    }

    //adaptive drift offset
    if(r<180)
    {
       overhead=41;
    }
    else if (r<458)
    {
        overhead=20;
    }
    else if (r<=600)
    {
        overhead=10;
    }
    else if(r<800)
    {
        overhead=1;
    }
    
    printf("overhead=%d\n",overhead);
    
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
    int r_bits = r * MILLION; //required bits/second
    
   
    int num_of_packets = r_bits / MAX_PACKET ; // packets/second
    
    double sleep_time = (1 / (double)num_of_packets);
    double usleep_time=sleep_time * MILLION; //sleep in us
    usleep_time= usleep_time - overhead ;
    printf("raw  sleep=%lf\n", sleep_time);
    printf("raw usleep=%lf\n", usleep_time);
    
    int workers = ceil((float)SLEEP_MIN / usleep_time);
    printf("launching %d worker(s)\n", workers);

    usleep_time=usleep_time * workers; //scale for parallel workers
    
    if(usleep_time < SLEEP_MIN)
    {
        printf("WARNING: usleep_time < 30us -- this percission is not reliable: %lf, exiting\n", usleep_time);
        return 0;
    }
    
    printf("%d / %d = %d, sleep=%lf\n", r_bits, MAX_PACKET, num_of_packets, usleep_time);
    
    for(int i=0; i<workers; i++)
    {
        if (fork()==0)
        {
            printf("child started: pid=%d, w/ sleep:%lf\n", getpid(), usleep_time);
            struct timeval start_time;
            struct timeval stop_time;
            float elapsed;
            gettimeofday( &start_time, NULL );
            
            //do work
            for(;;)
            {
                if (sendto(_socket, buf, BUFLEN, 0, (struct sockaddr *)&si_other, slen)==-1)
                    printf("Error: sendto()\n");
                    
                packet_count++;
                usleep(usleep_time);
                
                gettimeofday( &stop_time, NULL );
                
                elapsed = (float)(stop_time.tv_sec  - start_time.tv_sec);
                elapsed += (stop_time.tv_usec - start_time.tv_usec)/(float)MICRO_PER_SECOND;
                
                if(elapsed>=(double) t)
                    break;
                
            }
            
            printf("child closed: %d - %d packets sent\n", getpid(), packet_count);
            _exit(0);
            //return 0;
        }
        else
        {
            //printf("i am parent, pid=%d\n", getpid());
        }
    }
    
    printf("waiting for children\n");
    wait(NULL);
    close(_socket);
    
    printf("exiting main\n");
    return 0;

}
