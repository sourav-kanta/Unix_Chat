#include<stdio.h>
#include<sys/socket.h>
#include<sys/types.h>
#include<netinet/in.h>
#include<errno.h>
#include<arpa/inet.h>
#include<unistd.h>
#include<string.h>
#include<stdlib.h>
#include<pthread.h>
#include<sqlite3.h>

#define PORT 4444
#define BACKLOG 5

//int check=1;
sqlite3 *db;

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

sqlite3* openDB()
{
	sqlite3 *db;
   	int rc;

  	 rc = sqlite3_open("test.db", &db);
	 if(rc)
     		error_exit("Error opening database");
	 else
        	printf("Opened database successfully\n");
	return db;
}

void closeDB(sqlite3* db)
{
	int er=sqlite3_close(db);
	if(er)
		error_warn("Error closing database");
	else
		printf("Closed database successfully\n");
	return;	
}

void createFDTable(sqlite3 *db)
{
	char *sql="CREATE TABLE IF NOT EXISTS USERS("\
		"FD INT PRIMARY KEY,"\
		"USERNM TEXT);";
	int err=sqlite3_exec(db,sql,NULL,0,NULL);
	if(err!=SQLITE_OK)
		error_exit("Error creating table");
	else
		printf("Table creation successful\n");
}

void insertFDTable(sqlite3 *db,int fd,char *unm)
{
	char sql[200];
	sprintf(sql,"DELETE FROM USERS WHERE USERNM='%s';",unm);
	int err=sqlite3_exec(db,sql,NULL,0,NULL);
	if(err!=SQLITE_OK)
		error_warn("User overwrite unsuccessful\n");
	sprintf(sql,"INSERT INTO USERS VALUES ('%d','%s');",fd,unm);
	err=sqlite3_exec(db,sql,NULL,0,NULL);
	if(err!=SQLITE_OK)
		error_warn("User entry in database unsuccessful");
	else 
		printf("User entered with values %d and %s\n",fd,unm);
}

static int select_all_cb(void *args,int argc,char **argv,char **colnm)
{
	int i=0;
	for(i=0;i<argc;i++)
	{
		printf("%s\t%s\n",colnm[i],argv[i]); 
	}
	write(*((int *)args),argv[1],30);
	printf("\n");
	return 0;
}

void selectAllUser(sqlite3 *db,int fd)
{
	printf("\nPrinting all users : \n");
	char *sql="SELECT * FROM USERS;";
	int err=sqlite3_exec(db,sql,select_all_cb,&fd,NULL);
	if(err!=SQLITE_OK)
		error_warn("Error selecting users");
}

void deleteAllUser(sqlite3 *db)
{
	char *sql="DELETE FROM USERS;";
        int err=sqlite3_exec(db,sql,NULL,0,NULL);
        if(err!=SQLITE_OK)
                error_warn("Error deleteing users");
}

static int select_cb(void *args,int argc,char **argv,char **colnm)
{
        int i=0;
        for(i=0;i<argc;i++)
        {
 //               printf("%s\t%s\n",colnm[i],argv[i]);
        }
   //     printf("\n");
	if(write(atoi(argv[0]),args,30)==-1)
		error_warn("Error writing");
        return 0;
}

void selectUser(sqlite3 *db,char *unm,char *msg)
{
        //printf("\nPrinting user @%s : \n",unm);
        char sql[200];
	sprintf(sql,"SELECT * FROM USERS WHERE USERNM=\"%s\";",unm);
        int err=sqlite3_exec(db,sql,select_cb,(void *)msg,NULL);
        if(err!=SQLITE_OK)
                error_warn("Error selecting user");
}

void deleteUser(sqlite3 *db,char *unm)
{
        char sql[200];
	sprintf(sql,"DELETE FROM USERS WHeRE USERNM=\"%s\";",unm);
        int err=sqlite3_exec(db,sql,NULL,0,NULL);
        if(err!=SQLITE_OK)
                error_warn("Error deleteing user");
}

void * forward(void *param)
{
        int fd=*((int *)param);
	//fd=5;
        char input[30],output[30];
	//printf("Forwarding for FD : %d\n",fd);
	int check=1;  
        while(1 && check)
        {
                int r=read(fd,(void *)output,30);
		if(strcmp(output,"read_list")==0){
			selectAllUser(db,fd);
			continue;
		}
                if(r==-1)
                        error_warn("Error in reading from socket ");
                if(r==0)
                {
			printf("Broken connection\n");
                        check=0;
                        break;
                }
		printf("%s\n",output);
		char *user=strtok(output,"@");
		char *msg=strtok(NULL,"@");
		selectUser(db,user,msg);
                //printf("Received : ");
                //puts(output);
                /*puts("\n");*/
        }
	free(param);
	close(fd);
	pthread_exit(0);
        return NULL;

}

void do_comm(int fd)
{
        pthread_t fwd_thread,send_thread;
	printf("%d \n",fd);
	int *hfd=malloc(sizeof(int));
	*hfd=fd;
        if(pthread_create(&fwd_thread,NULL,forward,(void *)hfd))
                error_exit("Error creating thread ");
  //      if(pthread_cancel(rcv_thread))
  //              error_warn("Error quitting receive thread ");;
  //      if(close(fd)==-1)
  //              error_warn("Error closing connection to client ");
        return;
}



int main(int argc, char *argv[])
{
	db=openDB();
	createFDTable(db);
	

	//insertFDTable(db,2,"skanta");
	//insertFDTable(db,3,"skanta");
	deleteAllUser(db);
	//deleteUser(db,"asaha");
	//selectAllUser(db);
	//selectUser(db,"skanta");
	
        int serv_fd,comm_fd;
        int port;
        socklen_t addr_sz;
        if(argc<2)
                port=PORT;
        else
                port=atoi(argv[1]);
        struct sockaddr_in serv_addr,peer_addr;
        serv_fd=socket(AF_INET,SOCK_STREAM,0);
        if(serv_fd==-1)
                error_exit("Error in creating socket ");
        memset(&serv_addr,0,sizeof(serv_addr));
        serv_addr.sin_family=AF_INET;
        serv_addr.sin_port=htons(port);
        serv_addr.sin_addr.s_addr=htonl(INADDR_ANY);
        if(bind(serv_fd,(struct sockaddr *) &serv_addr,sizeof(serv_addr))==-1)
                error_exit("Error binding ");
        if(listen(serv_fd,BACKLOG) == -1)
                error_exit("Error listening ");
        printf("Server listening on port : %d\n",port);
        //single connection accepted
        addr_sz=sizeof(struct sockaddr);
	while(1)
	{
		//selectAllUser(db);
        	comm_fd=accept(serv_fd,(struct sockaddr *) &peer_addr,&addr_sz);
        	if(comm_fd==-1)
                	error_exit("Error accepting ");
        	else
                	printf("Received connection.\n");
		char user[30];
		if(read(comm_fd,(void *)user,30)==-1)
			error_warn("Error reading client name");
		insertFDTable(db,comm_fd,user);
		do_comm(comm_fd);
	}
	closeDB(db);	
}
