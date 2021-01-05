#include<stdio.h>
#include<sys/types.h>
#include<netinet/in.h>
#include<string.h>
#include<sys/socket.h>
#include<unistd.h>
#include<stdlib.h>
#include<time.h>
#include<errno.h>
#include<fcntl.h>
#include <arpa/inet.h>
#define name_size 256
#define buffer_size 2048
struct header
{
	unsigned short int destination_port;
      	unsigned short int source_port;
	unsigned int checksum;
	unsigned short int datalength;
	unsigned int seqnum;
	unsigned int acknum;
	
};
//20 byts on header
typedef struct header header;
struct timeval t={1,0};

int main(int argc,char* argv[])
{
	int socketfd=0;
	socketfd=socket(AF_INET , SOCK_DGRAM ,IPPROTO_UDP);
	if(socketfd!=1)
		printf("socket success\n");
	// connect to socket
	struct sockaddr_in info, clientinfo;
	bzero(&info,sizeof(info));//init string
	info.sin_family=AF_INET;
	info.sin_addr.s_addr=inet_addr("140.117.0.1");
	info.sin_port=htons(atoi(argv[1]));
	header server;
	server.source_port=atoi(argv[1]);
	server.seqnum=0;
	server.acknum=0;
	int on=1;
	if((setsockopt(socketfd,SOL_SOCKET,SO_REUSEADDR,&on,sizeof(on)))<0)
	{
		perror("setsockopt failed");
		exit(EXIT_FAILURE);
	}
	bzero(&clientinfo ,sizeof(clientinfo));
	if(bind(socketfd,(struct sockaddr*)&info,sizeof(info))==-1)
		perror("bind");
	else
		printf("bind success\n");
	// connect to a port
	// accept connection request
	
	clientinfo.sin_family=AF_INET;

	while(1)
	{
		socklen_t addrlen=sizeof(clientinfo);
		int clientFD;
		printf("plz transfer file name:\n");
		char received_file[name_size];
		memset(received_file,'\0',sizeof(received_file));
		// request file
	//	fcntl(socketfd, F_SETFL, O_NONBLOCK);
		clientFD=recvfrom(socketfd,received_file,name_size,0,(struct sockaddr*)&clientinfo,&addrlen);
		printf("[+]name received! file name=%s\n",received_file);
		FILE *f;
		f=fopen(received_file,"r");
		if(f==NULL)
		{
			printf("File can not open!!\n");
			continue;
		}
		else
		{
			char a,buffer[buffer_size+1],ackmsg[10];
			memset(buffer,'\0',sizeof(buffer));
			memset(buffer,'0',8);//syn and ack num
			fseek(f,0,SEEK_END);
			int fsize=ftell(f);
			server.seqnum=0;
			printf("size=%d\n",fsize);
			fseek(f,0,SEEK_SET);
			char Fsize[20],cp[20],i=0;//cp=copy
			memset(Fsize,'\0',sizeof(Fsize));
			memset(cp,'\0',sizeof(cp));
			sprintf(Fsize,"%d",fsize);
/*			while(fsize!=0)
			{
				cp[i++]=fsize%10+48;
				fsize/=10;
			}
			while(i!=-1)
			{
				Fsize[i]=cp[strlen(cp)-i];
				i--;
			}
			for(i=0;i<strlen(Fsize);i++)
				printf("%c",Fsize[i]);
			printf("\n");*/
			clientFD=sendto(socketfd,Fsize,sizeof(Fsize),0,(struct sockaddr*)&clientinfo,addrlen);
			int len=0,recent_len=0;
			while(1)
			{
				size_t flag=fread(&a,1,1,f);
				len++;
				if(flag==0)//eof
				{
					printf("File end!\n");
					break;
				}

				buffer[recent_len++]=a ^ 'S';
				if(a=='S'||recent_len==buffer_size-1)
				{
					if(recent_len==buffer_size-1)
					{
						buffer[recent_len]='S';
					//	sleep(10);
					}
					server.seqnum++;
					recent_len=0;
					printf("sending... strlen=%ld len=%d seqnum=%d\n",strlen(buffer),len,server.seqnum);
					int i=0;
				//	for(i=0;i<strlen(buffer);i++)
				//		printf("%c",buffer[i]);
					printf("\n");
					if(strlen(buffer)>1500)
						sleep(10000);
					clientFD=sendto(socketfd,buffer,strlen(buffer),0,(struct sockaddr*)&clientinfo,addrlen);
					perror("sendto");
					memset(ackmsg,'\0',sizeof(ackmsg));
					clientFD=recvfrom(socketfd,ackmsg,10,0,(struct sockaddr*)&clientinfo,&addrlen);
					perror("recvfrom");
					server.acknum=atoi(ackmsg);
					printf("ACK=%d\n",server.acknum);
					usleep(7500);
					if(server.seqnum!=server.acknum)
					{
						printf("no!\n");
						sleep(100);
					}
					memset(buffer,'\0',sizeof(buffer));
				}
			//	int socketopt=setsockopt(socketfd,SOL_SOCKET,SO_RCVTIMEO,&t,sizeof(t));
			//	clientFD=recvfrom(socketfd,ackmsg,10,0,(struct sockaddr*)&clientinfo,&addrlen);
			//	perror("recvfrom");
			/*	while(clientFD==-1 && errno==EAGAIN)//timeout(or again)
				{
					perror("recvfrom");
					sleep(5);
					printf("timeout!!\n");
					clientFD=sendto(socketfd,buffer,strlen(buffer)+1,MSG_DONTWAIT,(struct sockaddr*)&clientinfo,addrlen);
					perror("sendto");
					printf("resending...\n");
					socketopt=setsockopt(socketfd,SOL_SOCKET,SO_RCVTIMEO,&t,sizeof(t));
					printf("rcvACK again...\n");
				}*/
			}
			printf("last send,len=%d\n",len);
			clientFD=sendto(socketfd,buffer,sizeof(buffer),0,(struct sockaddr*)&clientinfo,addrlen);
			perror("sendto");
			clientFD=recvfrom(socketfd,ackmsg,10,0,(struct sockaddr*)&clientinfo,&addrlen);
			server.acknum=atoi(ackmsg);
			printf("ACKnum=%d\n",server.acknum);
			char eof=EOF;
			printf("sending EOF...\n");
			clientFD=sendto(socketfd,&eof,sizeof(eof),0,(struct sockaddr*)&clientinfo,addrlen);
			clientFD=recvfrom(socketfd,ackmsg,10,0,(struct sockaddr*)&clientinfo,&addrlen);
			printf("last rcving ACK%s\n",ackmsg);
		}
	}
	// conneect receive / transfer data
	return 0;
}
