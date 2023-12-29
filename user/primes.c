#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
void pipeline(int fd){
	int first = 1;
	int prev = 0;
	int forked = 0;
	int pipes[2];
	while(1){
		int now;
		int ret = read(fd,&now,1);
		if(ret == 0){
			close(pipes[1]);
			wait(0);
			exit(0);
		}
		if(first == 1){
			first = 0;
			prev = now;
			printf("prime %d\n",now);
		}else if(now % prev){
			if(forked == 0){
				pipe(pipes);
				forked = 1;
				if(fork()== 0){
					close(pipes[1]);
					close(fd);
					pipeline(pipes[0]);
				}
			}
			write(pipes[1],&now,1);
		}

	}
}
int main(int argc, char *argv[])
{
	int pipe1[2];
	pipe(pipe1);
	for(int i=2;i<=35;i++){
		write(pipe1[1],&i,1);
	}
	close(pipe1[1]);
	pipeline(pipe1[0]);
	wait(0);
	exit(0);
}
