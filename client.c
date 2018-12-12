#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<unistd.h>
#include<pthread.h>
#include<signal.h>
//void *_receive();
//void *type();
int sockfd;
pthread_cond_t cond=PTHREAD_COND_INITIALIZER;
pthread_mutex_t mutex=PTHREAD_MUTEX_INITIALIZER;
int J=0;
void *_receive()
{
        char Buffer[2000];
        int len=0;
	char filename[20],line[2000];
	FILE *fp;
        memset(Buffer,0,sizeof(Buffer));
        while(len=recv(sockfd,Buffer,2000,0)>0)
	{
                if(strcmp(Buffer,"Do you want to receive file?yes/no")==0)
		{
			printf("Do you want to receive file?yes/no\n");
			memset(Buffer,0,sizeof(Buffer));
			recv(sockfd,Buffer,2000,0);
			if(strcmp(Buffer,"no")==0)
			{
				memset(Buffer,0,sizeof(Buffer));
				printf("rejected the request!\n");
				continue;
			}
			recv(sockfd,filename,20,0);
			fp=fopen(filename,"w");
			while(recv(sockfd,line,2000,0)>0)
			{
				if(strcmp(line,"EOF")==0)
					break;
				fprintf(fp,"%s",line);
				memset(line,0,sizeof(line));
			}
			printf("Received the file!\n");
			fclose(fp);
		}
		else if(strcmp(Buffer,"no")==0)
		{
			pthread_cond_signal(&cond);
		}
		else if(strcmp(Buffer,"yes")==0)
		{
			J=1;
			pthread_cond_signal(&cond);
		}
		else
			printf("%s",Buffer);
                memset(Buffer,0,sizeof(Buffer));
        }
        pthread_exit(NULL);
}

void *type()
{
        char buffer[2000],name[20];
	char filename[20];
	char line[2000];
	FILE *fp;
        printf("Enter client name: ");
        fgets(name,20,stdin);
        name[strlen(name)-1]='\0';
        send(sockfd,name,strlen(name),0);
        memset(buffer,0,sizeof(buffer));
        while(1)
        {
                printf("Enter a Message: \n");
                fgets(buffer,1000,stdin);
                buffer[strlen(buffer)-1]='\0';
		if(strcmp(buffer,"sendfile")==0)
		{
			send(sockfd,buffer,2000,0);
			printf("Enter the client to send file: ");
			fgets(name,20,stdin);
			name[strlen(name)-1]='\0';
			send(sockfd,name,strlen(name),0);
			printf("Enter file name: ");
			fgets(filename,20,stdin);
			filename[strlen(filename)-1]='\0';
			send(sockfd,filename,strlen(filename),0);
			
			pthread_cond_wait(&cond,&mutex);
			if(J==0)
			{
				printf("The client rejected\n");
				continue;
			}
			else
			{	
				J=0;
				fp=fopen(filename,"r");
				while(fgets(line,2000,fp)!=NULL)
				{
					send(sockfd,line,strlen(line),0);
					memset(line,0,sizeof(line));
				}
				sleep(2);
				send(sockfd,"EOF",strlen("EOF"),0);
			
				printf("Sent file!\n");
			}
		}
		else
                	send(sockfd,buffer,strlen(buffer),0);
                memset(buffer,0,sizeof(buffer));
        }
        pthread_exit(NULL);
}

int main()
{
	int i;
	void *status;
	struct sockaddr_in servaddr;
	
	sockfd=socket(AF_INET,SOCK_STREAM,0);
	if(sockfd==-1)
	{
		perror("Could not create socket");
	}

	printf("Created Socket\n");
	bzero(&servaddr,sizeof(servaddr));
	servaddr.sin_family=AF_INET;
	servaddr.sin_port=htons(8080);
	servaddr.sin_addr.s_addr=inet_addr("127.0.0.1");
	
	connect(sockfd,(struct sockaddr *)&servaddr,sizeof(servaddr));

	pthread_t r,t;
	pthread_create(&r,NULL,(void *)(&_receive),NULL);
	pthread_create(&t,NULL,(void *)(&type),NULL);
//	
	pthread_join(r,&status);
	pthread_join(t,&status);
 	
	close(sockfd);
	return 0;
}
