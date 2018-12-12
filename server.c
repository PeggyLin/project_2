#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<unistd.h>
#include<pthread.h>
void *server_handler(int n);
char name[5][20];
int connfd[5];
int OP=0;
pthread_cond_t cond=PTHREAD_COND_INITIALIZER;
pthread_mutex_t mutex=PTHREAD_MUTEX_INITIALIZER;
int main()
{
	memset(connfd,-1,sizeof(connfd));
	int listenfd,n=0, i=0, new_sock;
	socklen_t clilen;
	struct sockaddr_in cliaddr, servaddr;

	//create socket
	listenfd=socket(AF_INET,SOCK_STREAM,0);

	if(listenfd==-1)
	{
		printf("Could not create socket\n");
	}
	puts("Socket Created");

	//prepare the sockaddr_in structure
	bzero(&servaddr,sizeof(servaddr));
	servaddr.sin_family=AF_INET;
	servaddr.sin_addr.s_addr=INADDR_ANY;
	servaddr.sin_port = htons(8080);

	//Bind
	if(bind(listenfd,(struct sockaddr *)&servaddr,sizeof(servaddr))<0)
	{
		//print error message
		perror("bind failed");
		return 1;
	}
	puts("bind success");

	listen(listenfd, 5);

	//accept and incoming connection
	puts("Waiting for connections");
	clilen=sizeof(cliaddr);

	while(1)
	{
		for(i=0;i<5;i++)
		{
			if(connfd[i]==-1)
				break;
		}
		connfd[i]=accept(listenfd,(struct sockaddr *)&cliaddr,&clilen);
		puts("Connection accepted");

		pthread_create(malloc(sizeof(pthread_t)),NULL,(void *)(&server_handler),(void *)i);
	}
/*	if(connfd[i]<0)
	{
		perror("Accept Failed");
		return 1;
	}
	close(listenfd);
*/	return 0;
}

void *server_handler(int n)
{
	printf("Start\n");
	int sock=connfd[n];
	int read_size,i=0;
	int f=0;
	char buffer[2000],talkto[20];
	char client_message[2000];

	memset(client_message,0,sizeof(client_message));
	recv(connfd[n],name[n],20,0);
	printf("Client %s is connected\n",name[n]);
	
	while(1)
	{
		memset(talkto,0,sizeof(talkto));
		read_size=recv(sock,client_message,2000,0);
		if(read_size>0)
		{
			if(strcmp(client_message,"check")==0)
			{
				send(sock,"online: \n",strlen("online: \n"),0);
				for(i=0;i<5;i++)
				{
					if(connfd[i]!=-1)
					{
						memset(buffer,0,sizeof(buffer));
						sprintf(buffer,"%s\n",name[i]);
						send(sock,buffer,strlen(buffer),0);
					}
				}
			}
			else if(strcmp(client_message,"talk")==0)
			{
				f=0;
				recv(sock,talkto,20,0);
				for(i=0;i<5;i++)
				{
					if(connfd[i]!=-1)
					{
						if(strcmp(name[i],talkto)==0)
						{
							f=1;
							break;
						}
					}
				}
				if(f==0)
				{
					send(sock,"The client not exist!",strlen("The client not exist!"),0);
					memset(client_message,0,sizeof(client_message));
					continue;
				}
				memset(client_message,0,sizeof(client_message));
				recv(sock,client_message,2000,0);
				while(strcmp(client_message,"end")!=0)
				{
					memset(buffer,0,sizeof(buffer));
					sprintf(buffer,"%s say:%s\n",name[n],client_message);
					send(connfd[i],buffer,sizeof(buffer),0);
					memset(client_message,0,sizeof(client_message));
					recv(sock,client_message,2000,0);
				}
			}
			else if(strcmp(client_message,"sendfile")==0)
			{
				char filename[20];
				memset(filename,0,sizeof(filename));
				recv(sock,talkto,20,0);
				for(i=0;i<5;i++)
				{
					if(connfd[i]!=-1)
					{
						if(strcmp(name[i],talkto)==0)
							break;
					}
				}
				recv(sock,filename,20,0);
				send(connfd[i],"Do you want to receive file?yes/no",strlen("Do you want to receive file?yes/no"),0);
				memset(client_message,0,sizeof(client_message));
				pthread_cond_wait(&cond,&mutex);
				if(OP==0)
				{
					send(sock,"no",strlen("no"),0);
					memset(client_message,0,sizeof(client_message));
					continue;
				}
				else
				{
					OP=0;
					char line[2000];
					send(sock,"yes",strlen("yes"),0);
					sleep(1);
					send(connfd[i],filename,strlen(filename),0);
					sleep(1);
					while(recv(sock,line,2000,0)>0)
					{
						if(strcmp(line,"EOF")==0)
						{
							sleep(2);
							send(connfd[i],line,strlen(line),0);
							break;
						}
						else
							send(connfd[i],line,strlen(line),0);
						
						memset(line,0,sizeof(line));
					}
					printf("already sent file\n");
				}
			}
			else if(strcmp(client_message,"no")==0)
			{
				send(sock,"no",strlen("no"),0);
				pthread_cond_signal(&cond);
			}
			else if(strcmp(client_message,"yes")==0)
			{
				send(sock,"yes",strlen("yes"),0);
				OP=1;
				pthread_cond_signal(&cond);
			}
			else
			{
				for(i=0;i<5;i++)
				{
					if(connfd[i]!=-1)
					{
						memset(buffer,0,sizeof(buffer));
						sprintf(buffer,"%s:%s\n",name[n],client_message);
						send(connfd[i],buffer,2000,0);
					}
				}
			}
			memset(client_message,0,sizeof(client_message));
		}
		else
			break;
	}
//	if(read_size==0)
//	{
		connfd[n]=-1;
		printf("Client %s disconnected\n",name[n]);
		memset(name[n],0,sizeof(name[n]));
		fflush(stdout);
//	}
//	else if(read_size==-1)
//	{
//		perror("recv failed");
//	}
	pthread_exit(NULL);
}
//
