#include <stdio.h>
#include <sys/time.h>
#include <stdlib.h>
int main(int argc ,char * argv[])
{
	int cnt = 0;
	struct timeval time_begin,time_cur;
	if (gettimeofday(&time_begin,NULL) != 0){
		printf("gettime err \n");
		exit(0);
	}

	while(1)
	{
				
		if (gettimeofday(&time_cur,NULL) != 0){
			printf("gettime err \n");
			exit(0);
		}
		cnt++;
		if ((time_cur.tv_usec - time_begin.tv_usec) != 0){
			printf("gettimeofday run cnt is %d \ntime_cur is %ldus time_begin is %ldus radio is %ldus \n",cnt,time_cur.tv_usec,time_begin.tv_usec,time_cur.tv_usec - time_begin.tv_usec);
			exit(0);
		}
	}

}
