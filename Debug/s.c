#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <unistd.h>
#include <mysql/mysql.h>
#pragma comment(lib, "libmysql.lib")

#define MAX_MYSQL_LEN 1024
/*数据库连接用宏*/
#define HOST "localhost"
#define USERNAME "root"
#define PASSWORD "zzl"
#define DATABASE "group_chat"
int socket_fd;
int accept_fd;
#define SER_ADDR "127.0.0.1"
#define MSG_LOGIN       1
#define MSG_REG         2
#define MSG_CHANGPD     3
#define MSG_GROUP       4
#define MSG_ALONE       5
#define MSG_DONE        6
#define MSG_ACK         7
struct msg
{
	int type;
	int data_len;
	char data[2048];
};
MYSQL *conn; /*这是一个数据库连接*/
void print_err(int line,char *str,int err_num)
{
	printf("In line:%d ,%s error ,error type: %d\n",line,str,err_num);
}
int connect_mysql()
{
		/*初始化mysql连接my_connection*/
		conn=mysql_init(NULL);
		/*建立mysql连接*/
		if (NULL != mysql_real_connect(conn, HOST, USERNAME, PASSWORD,DATABASE, 0, NULL, CLIENT_FOUND_ROWS))  /*连接成功*/
		{
			printf("数据库查询query_sql连接成功！\n");
		}
		else
		{
			printf("数据库连接失败");
		}
}
int comp_passwd(char buff[])
{

		int res; /*执行sql語句后的返回标志*/
		MYSQL_RES *res_ptr; /*指向查询结果的指针*/
		MYSQL_FIELD *field; /*字段结构指针*/
		MYSQL_ROW result_row; /*按行返回的查询信息*/
		int row, column; /*查询返回的行数和列数*/
		int i, j;
		char id[10]={0};
		char passwd[20]={0};
		for(i=0;buff[i];i++)
			if(buff[i] == ' ')
				break;
		strncpy(id,buff,i);
		id[i]=0;
		j=0;
		for(i=i+1;buff[i];i++)
			passwd[j++] = buff[i];
		passwd[j]=0;
		char sql[50]="select * from members where ID = ";
		strcat(sql,id);
		res = mysql_query(conn, sql);
		if (res)   /*执行失败*/
		{
			printf("Error： mysql_query !\n");
			/*关闭连接*/
			mysql_close(conn);
		}
		else     /*现在就代表执行成功了*/
		{
			/*将查询的結果给res_ptr*/
			res_ptr = mysql_store_result(conn);
			/*如果结果不为空，就把结果print*/
			if (res_ptr)
			{
				/*取得結果的行数和*/
				column = mysql_num_fields(res_ptr);
				row = mysql_num_rows(res_ptr);
				if(row < 1)
					return 0;
				else
				{
					result_row = mysql_fetch_row(res_ptr);
					printf("%s\n",result_row[2]);
					if( strcmp(result_row[2],passwd) == 0)
						return 1;
					else
						return 0;
				}
			}
			else
				return 0;
		}
}
int reg_count(char buff[])
{
	int res; /*执行sql語句后的返回标志*/
	MYSQL_RES *res_ptr; /*指向查询结果的指针*/
	MYSQL_FIELD *field; /*字段结构指针*/
	MYSQL_ROW result_row; /*按行返回的查询信息*/
	int row, column; /*查询返回的行数和列数*/
	int i, j, flag=0;
	char id[10]={0};
	char name[20]={0};
	char passwd[20] = {0};
	for(i=0;buff[i];i++)
		if(buff[i] == ' ')
			break;
	strncpy(id,buff,i);
	id[i]=0;
	j=0;
	for(i=i+1;buff[i]!=' ';i++)
		name[j++] = buff[i];
	name[j] = 0;
	j=0;
	for(i=i+1;buff[i];i++)
		passwd[j++] = buff[i];
	passwd[j] = 0;
	char sql[50]="select * from members where ID = ";
	strcat(sql,id);
	res = mysql_query(conn, sql);
	if (res)
	{
		printf("Error： mysql_query !\n");
		mysql_close(conn);
	}
	else
	{
		res_ptr = mysql_store_result(conn);
		if (res_ptr)
		{
			column = mysql_num_fields(res_ptr);
			row = mysql_num_rows(res_ptr);
			if(row < 1)
				flag = 0;
			else
			{
				flag = 1;
			}
		}
		else
			flag = 0;
	}
	if( flag == 1)
		return 0;
	sprintf(sql,"INSERT INTO members values ('%s','%s','%s','0','1')",id,name,passwd);
	res = mysql_query(conn,sql);
    if(res)
    {
        printf("Error : mysql_query !\n");
        mysql_close(conn);
    }
    else
    	return 1;
}
int change_passwd(char buff[])
{
	char id_passwd[50]={0},new_passwd[50]={0},id[10]={0};
	strcpy(id_passwd,buff);
	int i,j;
	for(i = strlen(id_passwd)-1;i >= 0;i--)
	{
		if(id_passwd[i] == ' ')
			break;
	}
	id_passwd[i] = 0;
	printf("id_passwd = %s\n",id_passwd);
	if(comp_passwd(id_passwd) == 0)
		return 0;
	j=0;
	for(i = i+1;buff[i];i++)
		new_passwd[j++] = buff[i];
	new_passwd[j] = 0;
	j=0;
	for(i=0;buff[i] != ' ';i++)
	{
		id[j++] = buff[i];
	}
	id[j] = 0;
	printf("id = %s\n",id);
	printf("new_passwd = %s\n",new_passwd);
	int res;
	char sql[100] = {0};
	sprintf(sql,"update members set PASSWD = '%s' where ID = '%s'",new_passwd,id);
	res = mysql_query(conn,sql);
    if(res)
    {
        printf("Error : mysql_query !\n");
        return 0;
        mysql_close(conn);
    }
    else
    	return 1;
}
int main()
{
	struct sockaddr_in servaddr;
	/*socket*/
	if( ( socket_fd = socket(AF_INET,SOCK_STREAM,0) ) == -1 )
	{
		print_err(__LINE__,"socket",errno);
		exit(0);
	}
	printf("socket successfully!\n");
	/*init*/
	memset(&servaddr,0,sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = inet_addr(SER_ADDR);
	servaddr.sin_port = htons(8000);

	/*bind*/
	if( bind(socket_fd,(struct sockaddr*)&servaddr,sizeof(servaddr)) == -1 )
	{
		print_err(__LINE__,"bind",errno);
		exit(0);
	}
	printf("bind successfully!\n");
	/*listen*/
	if( listen(socket_fd,4) == -1)
	{
		print_err(__LINE__,"listen",errno);
		exit(0);
	}
	printf("listen successfully!\n");

	connect_mysql();

	while(1)
	{
		/*accept*/
		if( ( accept_fd = accept(socket_fd,NULL,0) ) == -1 )
		{
			print_err(__LINE__,"accept",errno);
			continue;
		}
		if(accept_fd > 0)
		{
			printf("accept successfully!\n");
			pid_t pid;
			pid = fork();
			if(pid == 0)
			{
				int len;
				struct msg *sm,*rm;
				sm = (struct msg*)malloc(2200);
				rm = (struct msg*)malloc(2200);
				while(1)
				{
					len = recv(accept_fd,(void*)rm,sizeof(struct msg)+100,0);
					if(len > 0 && rm->type == MSG_LOGIN)
					{
						sm->type = MSG_ACK;
						if( comp_passwd(rm->data) == 1)
							strcpy(sm->data,"correct");
						else
							strcpy(sm->data,"incorrect");
						printf("%s\n",sm->data);
						sm->data_len = strlen(sm->data);
						if(send(accept_fd,(void*)sm,2048+sm->data_len,0) == -1)
						{
							print_err(__LINE__,"send msg",errno);
							break;
						}
					}
					else if(len > 0 && rm->type == MSG_REG)
					{   
						sm->type = MSG_ACK;
						if(reg_count(rm->data) == 1)
							strcpy(sm->data,"correct");
						else
							strcpy(sm->data,"incorrect");
						sm->data_len = strlen(sm->data);
						if(send(accept_fd,(void*)sm,2048+sm->data_len,0) == -1)
						{
							print_err(__LINE__,"send msg",errno);
							break;
						}
					}
					else if(len > 0 && rm->type == MSG_CHANGPD)
					{
						sm->type = MSG_ACK;
						if( change_passwd(rm->data) == 1)
							strcpy(sm->data,"success");
						else
							strcpy(sm->data,"unsuccess");
						printf("%s\n",sm->data);
						sm->data_len = strlen(sm->data);
						if(send(accept_fd,(void*)sm,2048+sm->data_len,0) == -1)
						{
							print_err(__LINE__,"send msg",errno);
							break;
						}
					}
					/*else if(len > 0 && rm->type == MSG_EXCEPTION)
					{
						printf("write error !\n");
						break;
					}*/
					else
						break;
				}
				free(sm);
				free(rm);
				close(accept_fd);
				exit(0);
			}
			else if(pid > 0);
			else
				perror("fork");
		}
	}
	close(socket_fd);
	mysql_close(conn);
	return 0;
}
