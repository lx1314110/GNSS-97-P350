#include <stdio.h>
#include <sys/time.h>
#include <stdlib.h>
#include <time.h>
int main(int argc ,char * argv[])
{

	struct timespec time_begin,time_cur;
	if (clock_gettime(CLOCK_MONOTONIC,&time_begin) != 0){
		printf("gettime err \n");
		exit(0);
	}

	while(1)
	{
				
		if (clock_gettime(CLOCK_MONOTONIC,&time_cur) != 0){
			printf("gettime err \n");
			exit(0);
		}
		if ((time_cur.tv_nsec - time_begin.tv_nsec) != 0){
			printf("CLOCK_MONOTONIC time_cur is %ldns time_begin is %ldns radio is %ldns \n",time_cur.tv_nsec,time_begin.tv_nsec,time_cur.tv_nsec - time_begin.tv_nsec);
			break;
		}
	}

	/*real time */
	if (clock_gettime(CLOCK_REALTIME,&time_begin) != 0){
	printf("gettime err \n");
	exit(0);
	}
	while(1)
	{
				
		if (clock_gettime(CLOCK_REALTIME,&time_cur) != 0){
			printf("gettime err \n");
			exit(0);
		}
		if ((time_cur.tv_nsec - time_begin.tv_nsec) != 0){
			printf(" CLOCK_REALTIME time_cur is %ldns time_begin is %ldns radio is %ldns \n",time_cur.tv_nsec,time_begin.tv_nsec,time_cur.tv_nsec - time_begin.tv_nsec);
			exit(0);
		}
	}
}
