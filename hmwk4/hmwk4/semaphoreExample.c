/*******************************************************************************
 In this example program  we demonstrated  how semaphores can be used to protect shared resources.
 In the example program, the shared resource is the variable holding the index of the new   character
 to be printed on the screen ( shared resource can also be thought as screen ). Several processes attempt
 +to cooperatively and correctly write a character string to the screen. The program is designed to be compiled
 two ways, one without semaphore protection and one with semaphore protection. In this way, students  can examine
 how the program behaves differently with and without protection
 
 The program is written by Eric Sotol.
 *******************************************************************************/



#define PROTECT



/*
 * Needed Includes
 */
#include <errno.h>              /* errno and error codes */
#include <sys/time.h>           /* for gettimeofday() */
#include <unistd.h>             /* for gettimeofday(), getpid() */
#include <stdio.h>              /* for printf() */
#include <unistd.h>             /* for fork() */
#include <sys/types.h>          /* for wait() */
#include <sys/wait.h>           /* for wait() */
#include <signal.h>             /* for kill(), sigsuspend(), others */
#include <sys/ipc.h>            /* for all IPC function calls */
#include <sys/shm.h>            /* for shmget(), shmat(), shmctl() */
#include <sys/sem.h>            /* for semget(), semop(), semctl() */
#include <sched.h>		/* for scheduling calls/constants */


/*
 * Needed constants
 */
#define SEM_KEY		0x1234
#define SHM_KEY		0x4321
#define		NO_OF_CHILDREN	3


/*
 * The following global variables hold important information.  The g_sem_id and
 * g_shm_id hold the IPC ids for the semaphore and shared memory segment used
 * by the program.  The g_shm_addr is an integer pointer to the shared memory
 * segment that is holding the integer index into the character array.
 */
int		g_sem_id;
int		g_shm_id;
int		*g_shm_addr; //shared mem index


/*
 * The following two structures hold the semapore information needed to lock
 * and unlock the semaphores.  They are initialized at the beginning of the
 * main program and used in the PrintAlphabet() routine.  By initializing them
 * at the beginning, they can easily be used at a later point without
 * reassignment or changes.
 */
struct sembuf	g_lock_sembuf[1];
struct sembuf	g_unlock_sembuf[1];


/*
 * The following character string will be displayed to the string.  You can
 * make this anything you wish.  Without contention, it will be displayed to
 * the screen exactly as it is seen below.
 */
char g_alphabet[] = "abcdefghijklmnopqrstuvwxyz 1234567890 ABCDEFGHIJKLMNOPQRSTUVWXYZ";


/*
 * Function prototypes
 */
void PrintAlphabet( void );


/*
 * The main program
 */
int main( int argc, char *argv[] )
{
    
    //set priority and sched info
    pid_t pid = getpid();
    struct sched_param param;
    int policy;

    policy = SCHED_FIFO;
    param.sched_priority = 18;
    if( sched_setscheduler( pid, policy, &param ) == -1 )
    {
        fprintf(stderr,"Error setting scheduler/priority!\n");
        exit(1); 
    }
    
    /*
     * Some needed variables
     */
    int rtn;
    int count;
    
    /*
     * Store the process ids of the children so they can be killed
     */
    int pid[NO_OF_CHILDREN];
    
    
	/*
	 * Build the semaphore structures
	 */
	g_lock_sembuf[0].sem_num   =  0;
	g_lock_sembuf[0].sem_op    = -1;
	g_lock_sembuf[0].sem_flg   =  0;
    
	g_unlock_sembuf[0].sem_num =  0;
	g_unlock_sembuf[0].sem_op  =  1;
	g_unlock_sembuf[0].sem_flg =  0;
    
	
	/*
	 * Create the semaphore
	 */
	if( ( g_sem_id = semget( SEM_KEY, 1, IPC_CREAT | 0666 ) ) == -1 ) {
		fprintf(stderr,"semget() call failed, could not create semaphore!");
		exit(1);
	}
	if( semop( g_sem_id, g_unlock_sembuf, 1 ) == -1 ) {
		fprintf(stderr,"semop() call failed, could not initalize semaphore!");
		exit(1);
	}
    
    
	/*
	 * Create the shared memory segment
	 */
	if( (g_shm_id = shmget( SHM_KEY, sizeof(int), IPC_CREAT | 0666)) == -1 ) {
		fprintf(stderr,"Couldn't create shared memory segment!\n");
		exit(1);
	}
	if( (g_shm_addr = (int *)shmat(g_shm_id, NULL, 0)) == (int *)-1 ) {
		fprintf(stderr,"Couldn't attach to shared memory segment!\n");
		exit(1);
	}
	*g_shm_addr = 0;
    
    
    /*
     * Create three children
     */
    rtn = 1;
    for( count = 0; count < NO_OF_CHILDREN; count++ ) {
        if( rtn != 0 ) {
            pid[count] = rtn = fork();
        } else {
            break;
        }
    }
    
    
    /*
     * Check return value to determine if process is parent or child
     */
    if( rtn == 0 ) {
        
        /*
         * I am a child
         */
        //printf("Child %i started ...\n", count);
		PrintAlphabet();
        
    } else {
        sleep(4);
        
        /*
		 * Kill of the children, the program should have run long
		 * enough.
         */
        kill(pid[0], SIGTERM);
        kill(pid[1], SIGTERM);
        kill(pid[2], SIGTERM);
        
        
        /*
         * Delete the shared memory
         */
        if( shmctl(g_shm_id,IPC_RMID,NULL) != 0 ) {
            fprintf(stderr,"Couldn't remove the shared memory segment!\n");
            exit(1);
        }
        
        
        /*
         * Delete the semaphore
         */
        if( semctl( g_sem_id, 0, IPC_RMID, 0) != 0 ) {
            fprintf(stderr,"Couldn't remove the semahpore!\n");
            exit(1);
        }
        
        exit(0);
    }
}


/*
 * This routine actually does the printing.  It calculates a psuedoradom number
 * from 1-3 to determine the number of characters to print.  If protect is set,
 * it then gets the semaphore.  Next, PrintAlphabet() will retrieve the current
 * index from the shared memory segment.  It loops, printing out the specified
 * number of characters.  Finally, it increments the index as necessary and
 * unlocks the semaphore if necessary.
 */
void PrintAlphabet( void )
{
	struct timeval tv;
	int number;
	int tmp_index;
	int i;
    
	/*
	 * Give all children a chance to start
	 */
	sleep(1);
    
	/*
	 * Enter the main loop
	 */
	while(1) {
        
        //timing
        struct timeval start_time;
        struct timeval stop_time;
        float running_average;
        
		/*
		 * Get the current time, the microseconds is used as a psuedo
		 * random number.  Calculate the random number based on a simple
		 * algorithm.
		 */
		if( gettimeofday( &tv, NULL ) == -1 ) {
			fprintf(stderr,"Couldn't get time of day, exiting.\n");
			exit(1);
		}
		number = ((tv.tv_usec / 47) % 3) + 1;
        
        //start time
        gettimeofday( &start_time, NULL );
        
		/*
		 * The #ifdef PROTECT will only include this piece of code if
		 * the macro PROTECT is defined.  Uncomment the beginning code
		 * to define PROTECT.  semop() is called to lock the semaphore.
		 */
#ifdef PROTECT
		if( semop( g_sem_id, g_lock_sembuf, 1 ) == -1 )
        {
            fprintf(stderr,"semop() call failed, could not lock semaphore!");
            exit(1);
        }
#endif
        
		/*
		 * Read the index from the shared memory segment
		 */
		tmp_index = *g_shm_addr;
        
        
		/*
		 * Loop the specified number of times, be sure not to overrun
		 * the bounds of the array, the if statement ensures this.
		 */
		for( i = 0; i < number; i++ ) {
			if( ! (tmp_index + i > sizeof(g_alphabet)) ) {
				//fprintf(stderr,"%c", g_alphabet[tmp_index + i]);
				usleep(1);
			}
		}
        
        
		/*
		 * Update the shared memory address for the number of characters
		 * that we wrote this time through the loop.
		 */
		*g_shm_addr = tmp_index + i;
        
        
		/*
		 * If the specified index is longer than sizeof the alphabet,
		 * write a return to start the next line and reset the index
		 * to zero.
		 */
		if( tmp_index + i > sizeof(g_alphabet) ) {
			//fprintf(stderr,"\n");
			*g_shm_addr = 0;
		}
        
        
		/*
		 * Unlock the semaphore if the PROTECT macro is defined
		 */
#ifdef PROTECT
		if( semop( g_sem_id, g_unlock_sembuf, 1 ) == -1 )
        {
            fprintf(stderr,"semop() call failed, could not lock semaphore!");
            exit(1);
        }
#endif
        //end time
        gettimeofday( &stop_time, NULL );
        float diff=(float)(stop_time.tv_usec  - start_time.tv_usec);
        printf("%lf\n", diff);
	}
}

