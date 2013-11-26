
/* The purpose of the following program is to demonstrate use of gettimeofday() system call. 
   The program computes the average amount of time and the maximum amount of time required 
   to execute gettimeofday() system call. It is provided here as an example to demonstrate 
   how to call gettimeofday() system call. 

   The program is developed by eric sotol
*/
   

/*
 * Needed Includes
 */
#include <sys/time.h>		/* gettimeofday() */
#include <unistd.h>		/* gettimeofday() */
#include <stdio.h>		/* printf() */
#include <sched.h>
//#define FIFO

/*
 * NO_OF_ITERATIONS is the number of times that the gettimeofday() call will
 * be called.  This number is large to allow for a large statistical set from
 * which the average time can be acquired.
 */
#define NO_OF_ITERATIONS	100000


/*
 * MICRO_PER_SECOND defines the number of microseconds in a second
 */
#define MICRO_PER_SECOND	1000000


/*
 * Start of the Main Program
 */
int main( int argc, char *argv[] )
{
	/*
	 * times array holds the timeval structure returned by each subsequent
	 * call to gettimeofday().  These values are stored in this array and
	 * held for later analysis.
         */
#ifdef FIFO
printf("using FIFO sched..\n\n\n");
    //set priority and sched info
     pid_t _pid = getpid();
    struct sched_param param;
    int policy;

        param.sched_priority = 99;
        if( sched_setscheduler( 0, SCHED_FIFO, &param ) == -1 ) {
                fprintf(stderr,"error setting scheduler ... are you root?\n");
                exit(1);
        }

        /*
         * Set the priority of the process
         */
        param.sched_priority = 99;
        if( sched_setparam( 0, &param ) == -1 ) {
                fprintf(stderr,"Error setting priority!\n");
                exit(1);
        }
#endif
printf("default sched...");
	struct timeval times[NO_OF_ITERATIONS];

        /*
         * Other important variables
         */
        float total;
	float delta;
	float maximum;
        int count;


	/*
	 * First, loop NO_OF_ITERATIONS times and each time through the loop
	 * make a call to gettimeofday() and store the results in the times
	 * array.
         */
	struct timeval timeread;
	for( count = 0; count < NO_OF_ITERATIONS; count++ )
	{
		gettimeofday( &timeread, NULL );
		times[count] = timeread;
	}


	/*
	 * Second, perform a second loop to analysis the results.  NOTE that
         * this loop depends on a pair of time values so it is only performed
	 * NO_OF_ITERATIONS - 1 times.
         */
	total = maximum = delta = 0;
	for( count = 0; count < NO_OF_ITERATIONS - 1; count ++ ) {
		printf("\n %d",count);
		/* Calculate the seconds part of the delta between count and count + 1 */
		delta  = times[count+1].tv_sec  - times[count].tv_sec;

		/* Calculate the microseconds part of the delta between count and count + 1 */
                delta += (times[count+1].tv_usec - times[count].tv_usec)/(float)MICRO_PER_SECOND;

		/* Add the delta to the total for calculating the average */
		total += delta;

		/* If delta greater than the maximum, replace the maximum */
		if( delta > maximum )
		{
			maximum = delta;
		}
	}

	/*
	 * Third, display the results,  NOTE:  the results are only displayed to
	 * the microsecond precision.  This is because the gettimeofday() call
	 * is only accurate to microseconds and any value beyond that is
	 * "garbage."
         */
        printf( "The average time for the gettimeofday() call: %.6f\n", total / count );
        printf( "The maximum time for the gettimeofday() call: %.6f\n", maximum );
}

