#include<unistd.h>
#include<sys/types.h>
#include<stdlib.h>
#include<signal.h>
#include<stdio.h>
#include<sys/wait.h>
int system_run(const char * pathname, char * const arv[] )
{
	pid_t pid;
	int status;

	if (NULL == pathname){
		return -1;		
	}
	if ((pid = fork()) < 0){
		status = -2;
	}else if(pid == 0){
		execv(pathname,arv);
		perror("execv");
		exit(-3);
	}else{
			status=pid;
	}

	return status;
}

#if 0
void handle_child(int sig)
{	
	int status;
	pid_t pid;
	while((pid=waitpid(-1,&status,WNOHANG)) > 0)
	{
		printf("chlid is exit %d\n",pid);	
	}

}
int main(int argc, char *argv[])
{
	
	char * ls_argv[]={NULL};
	int pid;
	signal(SIGCHLD,handle_child);
	pid=system_run("/bin/ps",argv);
	printf("run pid is %d\n",pid);
	pid=system_run("/bin/ps",argv);
	printf("run pid is %d\n",pid);
	while(1){
		printf("main is run \n");
		sleep(1);
	}
}
#endif
