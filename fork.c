#include<stdio.h>
#include<sys/types.h>
#include<unistd.h>
#include<stdlib.h>
#include<sys/wait.h>

int main()
{
	int status; // wait() 인자로 넣을 status 생성
	
	int pid = fork(); 
	// < fork() return value > 
	// 1. parent: child's pid
	// 2. child: 0

	// parent process
	if (pid > 0){
		printf("parent: child=%d\n", pid);
		pid = wait(&status); // child process가 종료될 때까지 wait
		printf("child %d is done\n", pid);
	}
	// child process
	else if (pid == 0){
		printf("child: exiting\n");
	}
	// fork() error 
	else{
		printf("fork error\n");
	}

	return 0;
}

