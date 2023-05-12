#include<stdio.h>
#include<sys/types.h>
#include<sys/wait.h>
#include<stdlib.h>
#include<unistd.h>


int main()
{
	char *argv[2];
	argv[0] = "ls"; // ls 함수 실행
	argv[1] = 0;
	//argv[2] = 0;

	int status;
	int pid = fork();

	// parent process
	if (pid > 0){
		printf("parent: child=%d\n", pid);
		pid = wait(&status);
		printf("child %d is done\n", pid);
	}
	// child process
	else if (pid == 0){
		printf("child: exiting\n");
	
		execve("/bin/ls", argv, NULL);
		// execve 함수 설명
		// [0]: 함수 경로
		// [1]: 함수 이름
		// [2]: 환경 (생략)
	} 
	// fork() error
	else {
		printf("fork error\n");
	}
	
	return 0;
}

