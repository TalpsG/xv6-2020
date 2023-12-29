#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int main(int argc, char *argv[])
{
	int pipe1[2];
	int pipe2[2];
	pipe(pipe1);
	pipe(pipe2);
	if(fork() == 0){
		char buf;
		int pid = getpid();
		read(pipe1[0],&buf,1);
		printf("%d: received ping\n",pid);
		write(pipe2[1],&buf,1);
		exit(0);
	}
	char buf = 'a';
	int pid = getpid();
	write(pipe1[1],&buf,1);
	read(pipe2[0],&buf,1);
	printf("%d: received pong\n",pid);
	exit(0);
}
