#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"
void find(char *path,char *target){
	int fd,sub_fd;
	struct dirent de;
	struct stat st;
	struct stat sub_st;
	char buf[512];
	fd = open(path,0);
	fstat(fd,&st);
	while((read(fd,&de,sizeof(de))) == sizeof(de)){
		//inum 意义不明，暂时按照ls的写法写
		if(de.inum == 0) continue;
		if(strcmp(de.name,".") == 0) continue;
		if(strcmp(de.name,"..") == 0) continue;
		strcpy(buf,path);
		int len = strlen(buf);
		buf[len] = '/';
		memmove(buf+len+1,de.name,DIRSIZ);
		buf[len+1+DIRSIZ] = '\0';
		if(strcmp(target,de.name) == 0){
			printf("%s\n",buf);
		}
		if((sub_fd = open(buf,0)) < 0){
			printf("%s not found\n",buf);
			close(sub_fd);
			continue;
		}
		if(fstat(sub_fd,&sub_st) < 0){
			printf("%s cannot stat\n",buf);
			close(sub_fd);
			continue;
		}
		close(sub_fd);
		if(sub_st.type == T_DIR){
			find(buf,target);
		}
	}
	close(fd);
}
int main(int argc, char *argv[])
{
	if(argc != 3){
		printf("find needs 2 params : dest directory and filename\n");
		exit(0);
	}
	int fd = open(argv[1],0);
	if(fd < 0){
		printf("%s not found\n",argv[1]);
		close(fd);
		exit(0);
	}
	struct stat st;	
	if(fstat(fd, &st) < 0){
		printf("%s cannot fstat\n", argv[1]);
		close(fd);
		exit(0);
	}
	if(st.type != T_DIR){
		printf("find needs a directory, not a file\n");
		close(fd);
		exit(0);
	}
	find(argv[1],argv[2]);
	exit(0);
}
