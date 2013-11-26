/*******************************************************************************
*    	Example program for Homework 4
*	Programmer: Eric Sorton
*       Purpose: This program demostrates the priority inversion of two
*		 processes.  To demonstrate this, three processes are created
*		 along with one shared memory segment and one sempahore.  The
*		 shared memory segment is used to allow the processes to
*		 communicate who has the lock.  The semaphore is the critical
*		 resource that both two processes need.
*
*		 Three processes will be started, a high priority process,
*		 a middle priority process, and a low priority process.  The
*		 high and low priority processes both need to get the lock.
*		 It is critical that the high priority process get the lock
*		 quickly when it needs it.
*
*		 However, this is not the case, the following scenario happens:
*
*			1) Low priority process gets lock
*			2) High priority process needs lock
*			3) Middle priority process runs and takes up a lot
*				of CPU time delaying the time that it takes
*				the High priority process to get the lock.
*			4) When Middle finally finishes, Low runs, releases
*				Lock.
*			5) Finally, High gets the Lock.
*
*******************************************************************************/

/*
 * Needed Includes
 */
#include <errno.h>		/* errno and error codes */
#include <sys/time.h>		/* for gettimeofday() */
#include <unistd.h>		/* for gettimeofday(), getpid() */
#include <stdio.h>		/* for printf() */
#include <unistd.h>		/* for fork() */
#include <sys/types.h>		/* for wait() */
#include <sys/wait.h>		/* for wait() */
#include <signal.h>		/* for kill(), sigsuspend(), others */
#include <sys/ipc.h>		/* for all IPC function calls */
#include <sys/shm.h>		/* for shmget(), shmat(), shmctl() */
#include <sys/sem.h>		/* for semget(), semop(), semctl() */
#include <sched.h>		/* for scheduling calls/constants */


/*
 * The IPC keys
 */
#define SHM_KEY		4321			/* Choose a number */
#define SEM_KEY		1234			/* Choose a number */


/*
 * The number of children that will be created
 */
#define NO_OF_CHILDREN	3


/*
 * The priorities (these may need adjusted on different machines)
 */
#define PARENT_PRIORITY		5
#define HIGH_PRIORITY		4
#define MIDDLE_PRIORITY		3
#define LOW_PRIORITY		2


/*
 * The two following sembuf's are used to lock and unlock the semaphore.  They
 * are declared as globals so that all of the functions can have access to
 * them.  Initialization occurs in the main routine!
 */
struct sembuf g_lock_sembuf[1];
struct sembuf g_lock_nowait_sembuf[1];
struct sembuf g_unlock_sembuf[1];


/*
 * The following two global variables hold the IPC ids
 */
int g_shm_id;
int g_sem_id;


/*
 * The pointer points to an integer in shared memory which stores a single
 * value.  This value indicates whether or not the low priority process has
 * the lock.
 */
int *g_shm_addr;

#define		UNLOCKED	1
#define		HIGH_HAS_LOCK	2
#define		LOW_HAS_LOCK	3


/*
 * The functions that will be run by each child
 */
void High( void );
void Middle( void );
void Low( void );


/*
 * Start of the Main Program
 */
int main( int argc, char *argv[] )
{
	/*
	 * The following structure is used to set a processes priority
	 */
	struct sched_param param;


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
	 * Set the scheduler to the Round Robin scheduler
	 */
	param.sched_priority = PARENT_PRIORITY;
	if( sched_setscheduler( 0, SCHED_RR, &param ) == -1 ) {
		fprintf(stderr,"error setting scheduler ... are you root?\n");
		exit(1);
	}

	/*
	 * Set the priority of the process
	 */
	param.sched_priority = PARENT_PRIORITY;
	if( sched_setparam( 0, &param ) == -1 ) {
		fprintf(stderr,"Error setting priority!\n");
		exit(1);
	}


        /*
         * Initalize the sembuf structures
         */
        g_lock_sembuf[0].sem_num   =  0;
        g_lock_sembuf[0].sem_op    = -1;
        g_lock_sembuf[0].sem_flg   =  0;
        g_lock_nowait_sembuf[0].sem_num   =  0;
        g_lock_nowait_sembuf[0].sem_op    = -1;
        g_lock_nowait_sembuf[0].sem_flg   =  IPC_NOWAIT;
        g_unlock_sembuf[0].sem_num =  0;
        g_unlock_sembuf[0].sem_op  =  1;
        g_unlock_sembuf[0].sem_flg =  0;

        /*
         * Create the semaphore and intialize it.  Use the semget() calls and
         * semop() calls described in the Experiment #5.
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
         * Create the shared memory segment and attach to it
         */
        if( (g_shm_id = shmget( SHM_KEY, sizeof(int), IPC_CREAT | 0666)) == -1 ) {
                fprintf(stderr,"Couldn't create shared memory segment!\n");
                exit(1);
        }
        if( (g_shm_addr = (int *)shmat(g_shm_id, NULL, 0)) == (int *)-1 ) {
                fprintf(stderr,"Couldn't attach to shared memory segment!\n");
                exit(1);
        }
	*g_shm_addr = UNLOCKED;


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
                printf("Child %i started ...\n", count);

		switch( count ) {
			case 1:
				High();
				break;
			case 2:
				Middle();
				break;
			case 3:
				Low();
				break;
		}

                exit(0);

	} else {
		sleep(10);

		/*
		 * Kill of the children, deadlock should have occured by now
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
 * The High Priority Process
 */
void High( void )
{
	/*
	 * The following structure is used to set a processes priority
	 */
	struct sched_param param;


	/*
	 * Other variables
	 */
	int rtn;


	/*
	 * Set the priority of the process
	 */
	param.sched_priority = HIGH_PRIORITY;
	if( sched_setparam( 0, &param ) == -1 ) {
		fprintf(stderr,"Error setting priority!\n");
		exit(1);
	}

	printf( "High priority process started ...\n" );

	/*
	 * The low priority process will loop forever, attempting to get the
	 * lock at a low rate.  Once it gets the lock, it will hold it for
	 * a short period of time and then release it.
	 */
	while(1) {
		usleep(1000);

		printf("High trying to get the lock!\n");

		while( (rtn = semop( g_sem_id, g_lock_nowait_sembuf, 1 )), rtn != 0 && errno == EAGAIN ) {
			usleep(1000);
		}

		if( rtn != 0 ) {
			fprintf(stderr,"semop() call failed, could not lock semaphore!");
			exit(1);
                }

		*g_shm_addr = HIGH_HAS_LOCK;

		printf("High has the lock!\n");

		usleep(100);

		printf("High release the lock!\n");

		if( semop( g_sem_id, g_unlock_sembuf, 1 ) == -1 )
		{
			fprintf(stderr,"semop() call failed, could not unlock semaphore!");
			exit(1);
                }

		*g_shm_addr = UNLOCKED;
	}
}


/*
 * The Middle Priority Process
 */
void Middle( void )
{
	/*
	 * The following structure is used to set a processes priority
	 */
	struct sched_param param;

	/*
	 * Other variables
	 */
	int i;

	/*
	 * Set the priority of the process
	 */
	param.sched_priority = MIDDLE_PRIORITY;
	if( sched_setparam( 0, &param ) == -1 ) {
		fprintf(stderr,"Error setting priority!\n");
		exit(1);
	}

	printf( "Middle priority process started ...\n" );

	/*
	 * The middle process loops forever each time through the loop, it
	 * checks to see if the low priority process has the lock.  If it does,
	 * it will then enter a long loop, grabbing the processor!
	 */
	while(1) {
		if( *g_shm_addr == LOW_HAS_LOCK ) {
			fprintf(stderr,"Low has lock, Middle entering long loop!\n");
			for( i = 0; i < 100000000; i++ );
			fprintf(stderr,"Low has lock, Middle exiting long loop!\n");
		}
		usleep(100);
	}
}


/*
 * The Low Priority Process
 */
void Low( void )
{
	/*
	 * The following structure is used to set a processes priority
	 */
	struct sched_param param;

	/*
	 * Set the priority of the process
	 */
	param.sched_priority = LOW_PRIORITY;
	if( sched_setparam( 0, &param ) == -1 ) {
		fprintf(stderr,"Error setting priority!\n");
		exit(1);
	}

	printf( "Low priority process started ...\n" );

	/*
	 * The low priority process will loop forever, attempting to get the
	 * lock at a low rate.  Once it gets the lock, it will hold it for
	 * a short period of time and then release it.
	 */
	while(1) {
		usleep(10000);

		if( semop( g_sem_id, g_lock_sembuf, 1 ) == -1 )
		{
			fprintf(stderr,"semop() call failed, could not lock semaphore!");
			exit(1);
                }

		*g_shm_addr = LOW_HAS_LOCK;

		fprintf(stderr,"Low has the lock!\n");

		usleep(1000);

		fprintf(stderr,"Low release the lock!\n");

		if( semop( g_sem_id, g_unlock_sembuf, 1 ) == -1 )
		{
			fprintf(stderr,"semop() call failed, could not lock semaphore!");
			exit(1);
                }

		*g_shm_addr = UNLOCKED;
	}
}

