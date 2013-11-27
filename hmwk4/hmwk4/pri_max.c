#include <stdio.h>
#include <stdlib.h>
#include <sched.h>
#include <unistd.h>

#define CHECK(sts,msg)  \
  if (sts == -1) {      \
    perror(msg);        \
    exit(-1);           \
  }

int main (int argc, char const *argv[])
{
	int sts;
	printf("\n");
	struct sched_param sched_params;
	sts=sched_getparam(0, &sched_params);
	CHECK(sts, "sched_getparam");

	int old_policy = sched_getscheduler(0);
	printf("initial policy: %d\n", old_policy);
	printf("initial priority: %d\n", sched_params.sched_priority);
	int max = sched_get_priority_max(SCHED_FIFO);
	printf("max=%d\n", max);
	
	struct sched_param new_sched_params;
	new_sched_params.sched_priority=max;
	
  	sts = sched_setscheduler(0, SCHED_FIFO, &new_sched_params);
	CHECK(sts,"sched_setscheduler");

	sts=sched_setparam(0, &new_sched_params);
	CHECK(sts, "sched_setparam");
	sched_getparam(0, &new_sched_params);
	printf("new priority: %d\n", new_sched_params.sched_priority);
	return 0;
}
