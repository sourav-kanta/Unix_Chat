#include<stdio.h>
#include<sys/types.h>
#include<netinet/in.h>
#include<sys/socket.h>
#include<string.h>
#include<unistd.h>
#include<errno.h>
#include<arpa/inet.h>
#include<stdlib.h>

#define PORT 4444

int check=1;

void error_exit(char *msg)
{
        perror(msg);
        exit(1);
}

void error_warn(char *msg)
{
        perror(msg);
        return;
}

void * send_msg(void *param)
{
	int fd=*((int *)param);
	char input[30],output[30];
        while(1 && check)
        {
                fgets(input,30,stdin);
                if(strcmp(input,"exit\n")==0)
                {
                        printf("\nClosing connection.\n");
                        break;
                }
                input[strlen(input)-1]=0;
                if(write(fd,(void *)input,30)==-1)
                        error_warn("Error in writing to socket ");
                /*puts("\n");*/
        }
        return NULL;

}

void * receive(void *param)
{
	int fd=*((int *)param);
	char input[30],output[30];
        while(1 && check)
        {
                int r=read(fd,(void *)output,30);
		if(r==-1)
                        error_warn("Error in reading from socket ");
		if(r==0){
			check=0;
			break;
		}
                printf("Received : ");
                puts(output);
                /*puts("\n");*/
        }
        return NULL;
	
}

void do_comm(int fd)
{
	char uname[30];
	printf("Enter username : ");
	scanf("%s",uname);
	write(fd,uname,30);
	pthread_t rcv_thread;
	if(pthread_create(&rcv_thread,NULL,receive,&fd))
		error_exit("Error creating thread ");
	send_msg(&fd);
	if(pthread_cancel(rcv_thread))
		error_warn("Error quitting receive thread ");;
	if(close(fd)==-1)
		error_warn("Error closing connection to client ");
	return;	 
}

int main(int argc,char *argv[])
{
	int client_fd;
	int port;
	struct sockaddr_in dest_addr;
	if(argc<2)
		port=PORT;
	else
		port=atoi(argv[1]);
	client_fd=socket(AF_INET,SOCK_STREAM,0);
	if(client_fd==-1)
		error_exit("Error initializing socket ");
	dest_addr.sin_family=AF_INET;
	dest_addr.sin_port=htons(port);
	dest_addr.sin_addr.s_addr=inet_addr("10.31.50.173");
	printf("Connecting to server at port : %d\n",port);
	if(connect(client_fd,(struct sockaddr *) &dest_addr,sizeof(dest_addr))==-1)
		error_exit("Error connecting to destination ");
	printf("Successfully established connection \n");
	do_comm(client_fd);
	return 0;
}		
