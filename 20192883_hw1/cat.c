#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/types.h>
#include<string.h>
#include<fcntl.h>
#include<errno.h>

#define BUFF_SIZE 16

void cat(int fd)
{
	int nbytes; // 파일에 쓸 데이터의 크기 
	char* buf;  // 버퍼
	
//	printf("fd:%d\n", fd);
	
	// memory 할당
	if (!(buf = (char*) malloc (sizeof(char) * BUFF_SIZE)))
	{
		fprintf(stdout, "Failed to allocate memory\n");
		exit(-1);
	}

	/* 
	[test]
	while ((nbytes = read(fd, buf, BUFF_SIZE)) > 0)
	{
		write(STDOUT_FILENO, buf, nbytes);
	}
	*/
	


	// 파일에 데이터가 있을 때 읽어오기
	while((nbytes = read(fd, buf, sizeof(buf))) > 0)
	{	
		// write() 함수는 실제 쓰여진 바이트의 수를 리턴함
		// 실제 쓴 값과 파일의 데이터 크기가 다르면 에러 메시지 출력
		if (write(STDOUT_FILENO, buf, nbytes) != nbytes)
		{
			printf("cat: write error\n");
			return;
		}
	}
	

	// read() 함수에서 오류가 난 경우 -1 리턴
	if (nbytes < 0)
	{
		printf("cat: read error\n");
		return;
	}
}

int main(int argc, char *argv[])
{
	int fd;
	
	// 인자가 없으면 즉시 종료
	if (argc <= 1)
	{
		cat(0);
		exit(0);
	}


	// argv[0]은 실행경로
	// argv[1]부터 받아야 할 인자
	for (int i=1; i<argc; i++)
	{
		// 여기서는 인자가 하나이므로 argv[1]을 연다 (실패시 에러 메시지 출력)
		if ((fd = open(argv[i], O_RDWR | O_CREAT, S_IRWXU)) < 0)
		{
			fprintf(stdout, "cat: cannot open %s\n", argv[i]);
			exit(0);
		}
		
		// open()함수의 리턴값인 fd를 cat()함수에 인자로 입력 
		cat(fd);
		close(fd);
	}

	exit(0);
}







	









