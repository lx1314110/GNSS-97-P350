#include <stdio.h>
#include <stdlib.h>
#include<string.h>
#include <sys/time.h>
char * str = "printf delay test\n";
int main(int argc, char *argv[])
{
	struct timeval time_begin,time_end;
	int len;
	int i;
	int cnt;
	int bit_tatol;
	float time_elapse;
	len = strlen(str);
	if (argc != 2)
	{
		printf("argc input err \n");
		exit(0);
	}
	cnt = atoi(argv[1]);
	gettimeofday(&time_begin,NULL);
	for(i = 0;i < cnt; i++)
	{
		printf("%s",str);
	}
	bit_tatol = len * i * 8;
	gettimeofday(&time_end,NULL);
	time_elapse = time_end.tv_sec - time_begin.tv_sec +  (time_end.tv_usec - time_begin.tv_usec)/1000000.00;
	fprintf(stderr,"\n***************************\n");
	fprintf(stderr,"******tatol kb : %fkb\n",bit_tatol/1024.00);
	fprintf(stderr,"*****bit : %f bit/s\n", bit_tatol/time_elapse);
	fprintf(stderr,"*****bit : %f kb/s\n", bit_tatol/time_elapse/1024.00);
	fprintf(stderr,"*****bit : %f mb/s\n", bit_tatol/time_elapse/1024.00/1024.00);
	fprintf(stderr,"********************************");
	return 0;
}
