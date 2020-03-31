#include <stdio.h>
#include "wrap.h"
#include <sys/epoll.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include "decode.h"
#include "dirent.h"
#include <signal.h>
class process
{
private:
    
public:
    void send_header(int cfd,int code,char *info,char *filetype,int length);
    void send_file(int cfd, char * filepath,int close_flag,struct epoll_event *ev,int epfd);
    void request_http(char *msg ,struct epoll_event *ev,int epfd);
    process();
    ~process();
};
//请求头处理函数
void process::send_header(int cfd,int code,char *info,char *filetype,int length)
{
	//状态行Get url Http/1.1  Http/1.1 200 OK
	char buf[1024]="";
	int len =0;
	len = sprintf(buf,"HTTP/1.1 %d %s\r\n",code,info);
	send(cfd,buf,len,0);
	//消息头
	len = sprintf(buf,"Content-Type:%s\r\n",filetype);
	send(cfd,buf,len,0);
	if(length > 0)
	{
		len = sprintf(buf,"Content-Length:%d\r\n",length);
		send(cfd,buf,len,0);
	}
	//空行
	send(cfd,"\r\n",2,0);

}
//发送文件处理函数
void process::send_file(int cfd, char * filepath,int close_flag,struct epoll_event *ev,int epfd)
{
	int fd = open(filepath,O_RDONLY);
	if(fd < 0)
	{
		perror("");
		return ;
	}
	char buf[1024]="";
	while(1)
	{
		int count = read(fd,buf,sizeof(buf));
		if(count <=0)
		{
			break;
		}
		int n = write(cfd,buf,count);
		printf("write = %d\n",n);
	}
	close(fd);
	if(close_flag == 1)
	{

		epoll_ctl(epfd,EPOLL_CTL_DEL,cfd,ev);
		close(cfd);
	}
}

//请求处理函数
void process::request_http(char *msg ,struct epoll_event *ev,int epfd)
{
	signal(SIGPIPE,SIG_IGN);
	int cfd = ev->data.fd;
	//printf("[%s]\n",msg);
	char method[256];
	char content[256];
	//GET /abc HTTP/1.1
	sscanf(msg,"%[^ ] %[^ ]",method,content);
	printf("[%s]   [%s]\n",method,content);
	if( strcasecmp(method,"get") == 0)
	{
		//  GET  /a.txt
		char *strfile = content+1;//前面有一个斜杠
		strdecode(strfile,strfile);//转码
		if(*strfile == 0)//如果没有请求 ，默认请求当前目录
		{
			strfile= (char*)"./";

		}
		struct stat s;
		if(stat(strfile,&s)== -1)//文件不存在
		{
			printf("文件不存在\n");
			//发生错误信息头部
			send_header(cfd,404, (char*)"NOT FOUND",get_mime_type( (char*)"*.html"),0);
			//发生error.html
			send_file( cfd, (char*)"error.html" ,1,ev, epfd);

		}
		else //文件存在
		{
			//普通文件
			if(S_ISREG(s.st_mode))
			{
				printf("普通文件\n");
				//发生信息头部
				send_header(cfd,200, (char*)"OK",get_mime_type(strfile),s.st_size);
				//发生文件
				send_file( cfd,strfile ,1,ev, epfd);

			}
			else if(S_ISDIR(s.st_mode))//目录
			{	
					printf("目录\n");
					//发生信息头部
					send_header(cfd,200, (char*)"OK",get_mime_type( (char*)"*.html"),0);
					//发生dir_header.html
					send_file( cfd, (char*)"dir_header.html" ,0,ev, epfd);
					//发生列表  组包
					struct dirent **list=NULL;
					int ndir = scandir(strfile,&list,NULL,alphasort);
					int i=0;
					printf("001\n");
					for(i=0;i<ndir;i++)
					{
						printf("002  \n");
						printf("%p\n",list[i]);
						printf("[%s]\n",list[i]->d_name);
						char listbuf[256]="";
						int n =0;
						if(list[i]->d_type == DT_REG)
						{
							n = sprintf(listbuf,"<li><a href=%s>%s</a></li>",list[i]->d_name,list[i]->d_name);
						}
						else if(list[i]->d_type == DT_DIR)
						{

							n = sprintf(listbuf,"<li><a href=%s/>%s/</a></li>",list[i]->d_name,list[i]->d_name);
						}
						send(cfd,listbuf,n,0);
						printf("0021\n");
						free(list[i]);
						
						printf("003\n");
					}
					free(list);


					//发生dir_tail.html
					send_file( cfd, (char*)"dir_tail.html" ,1,ev, epfd);
			}
		}
	}
}

process::process()
{
}

process::~process()
{
}
