#include<stdio.h>
#include<sys/types.h>
#include<netinet/in.h>
#include<string.h>
#include<sys/socket.h>
#include<unistd.h>
#include<stdlib.h>
#include <arpa/inet.h>
#define name_len 100
#define buffer_size 2048
struct header
{
	unsigned short int seqnum;
	unsigned short int acknum;
	unsigned short int checksum;
};
typedef struct header header;
int main(int argc,char* argv[])
{
	int socketfd=0;
	socketfd=socket(AF_INET , SOCK_DGRAM ,IPPROTO_UDP);
	if(socketfd!=1)
		printf("socket success\n");
	// connect to socket
	struct sockaddr_in info,serverinfo;
	bzero(&info,sizeof(info));//init string
	info.sin_family=AF_INET;
	info.sin_addr.s_addr=inet_addr(argv[2]);
	info.sin_port=htons(atoi(argv[1]));
	
	int err=connect(socketfd,(struct sockaddr*)&info,sizeof(info));
	if(err==-1)
		perror("connect");
	else
		printf("connect success\n");
	bzero(&serverinfo,sizeof(serverinfo));
	serverinfo.sin_family=AF_INET;
	serverinfo.sin_port=htons(80);
	socklen_t addrlen= sizeof(info);
	while(1)
	{
		int serverFD,filesize;
		header h;
		char request[name_len];
		printf("I want the file:");
		scanf("%s",request);
		serverFD=sendto(socketfd,request,sizeof(request),0,(struct sockaddr*)&info,addrlen);
		char  buffer[buffer_size];
		serverFD=recvfrom(socketfd,request,sizeof(request),0,(struct sockaddr*)&info,&addrlen);
		filesize=atoi(request);
		printf("fsize=%d\n",filesize);
		FILE *o;
		o=fopen("output.mp4","w");
		if(o==NULL)
			printf("error\n");
		int len=0;h.seqnum=0;
		while(1)
		{
			memset(buffer,'\0',sizeof(buffer));
			buffer[0]='\0';
			addrlen=sizeof(serverinfo);
			serverFD=recvfrom(socketfd,buffer,buffer_size,0,(struct sockaddr*)&serverinfo,&addrlen);
			if(serverFD==-1)
				perror("recvfrom");
			printf("data received! len=%ld\n",strlen(buffer));
			h.seqnum++;
			char seq_char[10];memset(seq_char,'\0',sizeof(seq_char));
			sprintf(seq_char,"%d",h.seqnum);
			printf("sendACKNUM:%s\n",seq_char);
			serverFD=sendto(socketfd,seq_char,strlen(seq_char),0,(struct sockaddr*)&serverinfo,addrlen);
			if(buffer[0]==EOF&&strlen(buffer)==1)
			{
				printf("END,len=%d\n",len);
				sprintf(seq_char,"%d",len);
			//	serverFD=sendto(socketfd,seq_char,strlen(seq_char),0,(struct sockaddr*)&serverinfo,addrlen);
				break;
			}
			else
			{
				int S=strlen(buffer);
				if(strlen(buffer)==buffer_size-1)
					S--;
				else if(strlen(buffer)>buffer_size)
					S=buffer_size-2;
				len=len+S+1;
				printf("hi strlen=%d now len=%d\n",S,len);
				int i=0;
				for(i=0;i<=S;i++)
				{
					char a;
					a=buffer[i] ^ 'S';
			//		printf("%c",buffer[i]);
					fwrite(&a,1,1,o);
				}
				printf("\n");
			}
		}
		fclose(o);
		printf("loop is out!\n");
	}
	return 0;
}
