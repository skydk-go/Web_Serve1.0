#include <stdio.h>
#include "socket_pack.h"
#include <sys/epoll.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include<functional>
#include <sys/stat.h>
#include "decode.h"
#include "dirent.h"
#include <signal.h>
#include"threadpool.h"
#include"process.h"
using namespace std;
#define  PORT 8000

int main(int argc, char const *argv[])
{
	typedef std::function<void(void)> request_task;
	threadpool<request_task>* pool = NULL;//创建线程池
    pool=new threadpool<request_task>;
	process process1;

	//切换工作目录
	char * curdir = getenv("PWD");
	char mydir[256]="";
	strcpy(mydir,curdir);
	strcat(mydir,"/web-http");
	chdir(mydir);
    
	int lfd = tcp4bind(PORT,NULL);
	Listen(lfd,128);
	int epfd = epoll_create(1);
	if(epfd < 0)
	{
		perror("");
		exit(0);
	}
	struct epoll_event ev,evs[1024];
	ev.events = EPOLLIN;
	ev.data.fd = lfd;
	epoll_ctl(epfd,EPOLL_CTL_ADD,lfd,&ev);
	while(1)
	{
		int n = epoll_wait(epfd,evs,1024,-1);
		if(n <0)
		{
			perror("");
			exit(0);
		}
		else
		{
			for(int i=0;i<n;i++)
			{
				if(evs[i].data.fd == lfd && evs[i].events&EPOLLIN)
				{
					struct sockaddr_in cliaddr;
					socklen_t len = sizeof(cliaddr);
					char ip[16]="";
					int cfd = Accept(lfd,(struct sockaddr*)&cliaddr,&len);	
					printf("client ip=%s port=%d\n",inet_ntop(AF_INET,&cliaddr.sin_addr.s_addr,ip,16),
						ntohs(cliaddr.sin_port));
					int flag = fcntl(cfd,F_GETFL);
					flag |= O_NONBLOCK;
					fcntl(cfd,F_SETFL,flag);

					ev.events = EPOLLIN;
					ev.data.fd = cfd;
					epoll_ctl(epfd,EPOLL_CTL_ADD,cfd,&ev);

				}
				else if( evs[i].events&EPOLLIN)
				{
					char msg[1500]="";
					int count  = Read(evs[i].data.fd,msg,sizeof(msg));
					if(count < 0)
					{
						perror("");
						close(evs[i].data.fd);
						epoll_ctl(epfd,EPOLL_CTL_DEL,evs[i].data.fd,&evs[i]);
					}
					else if(count == 0)
					{
						printf("client close\n");
						close(evs[i].data.fd);
						epoll_ctl(epfd,EPOLL_CTL_DEL,evs[i].data.fd,&evs[i]);

					}
					else
					{
						pool->append(std::bind(&process::request_http,&process1,msg,&evs[i],epfd));
						
					}


				}

			}


		}


	}


	return 0;
}
