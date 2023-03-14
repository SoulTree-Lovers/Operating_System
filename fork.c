#include<stdio.h>
#include<sys/types.h>
#include<unistd.h>
#include<stdlib.h>
#include<sys/wait.h>

int main()
{
	int status;
	int pid = fork();
	

	if (pid > 0){
		printf("parent: child=%d\n", pid);
		pid = wait(&status);
		printf("child %d is done\n", pid);
	}
	else if (pid == 0){
		printf("child: exiting\n");
	}
	else{
		printf("fork error\n");
	}

	return 0;
}

