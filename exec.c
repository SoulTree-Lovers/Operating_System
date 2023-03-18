#include<stdio.h>
#include<sys/types.h>
#include<sys/wait.h>
#include<stdlib.h>
#include<unistd.h>


int main()
{
	char *argv[3];
	argv[0] = "ls";
	argv[1] = 0;
	//argv[2] = 0;

	int status;
	int pid = fork();

	if (pid > 0){
		printf("parent: child=%d\n", pid);
		pid = wait(&status);
		printf("child %d is done\n", pid);
	} else if (pid == 0){
		printf("child: exiting\n");
	
		execve("/bin/ls", argv, NULL);
	} else {
		printf("fork error\n");
	}
	
	return 0;
}

