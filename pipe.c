#include<stdio.h>
#include<stdlib.h>
#include<sys/types.h>
#include<unistd.h>


int main()
{
	int p[2];
	// p[0]: readfd, p[1]: writefd
	char* argv[2];
		

	argv[0] = "wc";
	argv[1] = 0;

	pipe(p);
	
	// child process
	if(fork() == 0)
	{
		close(0); // close stdin
		dup(p[0]); // 
		close(p[0]);
		close(p[1]);
		
		execve("/usr/bin/wc", argv, NULL);
	} 
	// parent process
	else {
		close(p[0]); // 
		write(p[1], "hello world\n", 12); // p[1]에 string write
		close(p[1]);
	}
}
