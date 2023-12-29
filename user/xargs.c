#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int main(int argc,char ** argv){
	char buf[200];
	if(argc<2){
		printf("xargs needs more than 2 params\n");
		exit(0);
	}
	char *cmd = argv[1];
	char *param[32];
	int i;
	for(i=1;i<argc;i++){
		param[i-1] = argv[i];
	}
	while(gets(buf,512)){
		if(buf[0] == '\0') break;
		buf[strlen(buf)-1]= '\0';
		if(fork() == 0){
			param[i-1] = buf;
			exec(cmd,param);
		}
		wait(0);
	}
	exit(0);
}