#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#define SIZE 64 //mbps

int main(int argc, char** argv)
{
    
    int r=128; //mbps form terminal arg
    int t=10; //seconds form terminal arg
    
    int time=1; //this is the bool for the timer

    int workers = r/SIZE;
    
    printf("workers=%d\n", workers);
    
    for(int i=0; i<workers; i++)
    {
        if (fork()==0)
        {
            printf("i am child, pid=%d\n", getpid());
            
            //now do send routine
            while(time)
            {
                //sendto(sfd, "packetbody...", SIZE, ...);
                printf("pid: %d, sending packet: %d\n", getpid(), SIZE);
                sleep(1);
            }
            return 0;
        }
        else
        {
            //printf("i am parent, pid=%d\n", getpid());
        }
    }
    
    printf("waiting for children\n");
    wait(NULL);
    
    printf("exiting main\n");
    return 0;
}
