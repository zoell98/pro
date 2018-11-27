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

pthread_t pth_id[50];
int pth_len = 0;
int mem_len = 0;
pthread_mutex_t lock;
struct group
{
	int acp_fd;
	int gro_id;
};
struct group member[50];
char lord_id[10]={0};
#define SER_ADDR "127.0.0.1"
#define MSG_LOGIN       1
#define MSG_REG         2
#define MSG_CHANGPD     3
#define MSG_GROUP       4
#define MSG_ALONE       5
#define MSG_DONE        6
#define MSG_ACK         7
#define MSG_KICK 		8
#define MSG_BAN			9
#define MSG_UNBAN		10
#define MSG_GETBAN		11
#define MSG_UPLOAD		12
#define MSG_DOWNLOAD	13
#define MSG_FILENAME	14
#define MSG_CONTENT		15
#define MSG_FILEDONE	16
#define MSG_CONFFILE	17
#define MSG_SEARCH		18

struct msg
{
	int type;
	int data_len;
	int recv_id;
	char data[2048];
};

char file_list[500] = "mian.c ";

MYSQL *conn; /*这是一个数据库连接*/
void print_err(int line,char *str,int err_num)
{
	printf("In line:%d ,%s error ,error type: %d\n",line,str,err_num);
}
void sigfun()
{
	int i;
	for(i = 0; i < pth_len; i++)
		if( pthread_join(pth_id[i],NULL) )
			printf("error join thread.\n");
	close(socket_fd);
	mysql_close(conn);
	exit(0);
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
int search_name(char id[],char name[])
{
	int res; /*执行sql語句后的返回标志*/
	MYSQL_RES *res_ptr; /*指向查询结果的指针*/
	MYSQL_FIELD *field; /*字段结构指针*/
	MYSQL_ROW result_row; /*按行返回的查询信息*/
	int row, column; /*查询返回的行数和列数*/
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
				printf("%s\n",result_row[1]);
				strcpy(name,result_row[1]);
				return 1;
			}
		}
		else
			return 0;
	}
}
int search_lord(char lord_id[])
{
	int res; /*执行sql語句后的返回标志*/
	MYSQL_RES *res_ptr; /*指向查询结果的指针*/
	MYSQL_FIELD *field; /*字段结构指针*/
	MYSQL_ROW result_row; /*按行返回的查询信息*/
	int row, column; /*查询返回的行数和列数*/
	char sql[50]="select * from members where LORD = 1";
	res = mysql_query(conn,sql);
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
				printf("%s\n",result_row[1]);
				strcpy(lord_id,result_row[1]);
				return 1;
			}
		}
		else
			return 0;
	}
}
int kick_member(char kickid[])
{
	int res; /*执行sql語句后的返回标志*/
	MYSQL_RES *res_ptr; /*指向查询结果的指针*/
	MYSQL_FIELD *field; /*字段结构指针*/
	MYSQL_ROW result_row; /*按行返回的查询信息*/
	int row, column; /*查询返回的行数和列数*/
	char sql[50]="delete from members where ID = ";
	strcat(sql,kickid);
	res = mysql_query(conn, sql);
	if (res)   /*执行失败*/
	{
		printf("Error： mysql_query !\n");
		/*关闭连接*/
		mysql_close(conn);
		return 0;
	}
	else     /*现在就代表执行成功了*/
	{
		return 1;
	}
}
int ban(char banid[])
{
	int res; /*执行sql語句后的返回标志*/
	MYSQL_RES *res_ptr; /*指向查询结果的指针*/
	MYSQL_FIELD *field; /*字段结构指针*/
	MYSQL_ROW result_row; /*按行返回的查询信息*/
	int row, column; /*查询返回的行数和列数*/
	char sql[50]="update members set FLAG = 0 where ID = ";
	strcat(sql,banid);
	res = mysql_query(conn, sql);
	if (res)   /*执行失败*/
	{
		printf("Error： mysql_query !\n");
		/*关闭连接*/
		mysql_close(conn);
		return 0;
	}
	else     /*现在就代表执行成功了*/
	{
		return 1;
	}
}
int unban(char unbanid[])
{
	int res; /*执行sql語句后的返回标志*/
	MYSQL_RES *res_ptr; /*指向查询结果的指针*/
	MYSQL_FIELD *field; /*字段结构指针*/
	MYSQL_ROW result_row; /*按行返回的查询信息*/
	int row, column; /*查询返回的行数和列数*/
	char sql[50]="update members set FLAG = 1 where ID = ";
	strcat(sql,unbanid);
	res = mysql_query(conn,sql);
	if (res)   /*执行失败*/
	{
		printf("Error： mysql_query !\n");
		/*关闭连接*/
		mysql_close(conn);
		return 0;
	}
	else     /*现在就代表执行成功了*/
	{
		return 1;
	}
}
int get_ban(char id[],char flag[])
{
	int res; /*执行sql語句后的返回标志*/
	MYSQL_RES *res_ptr; /*指向查询结果的指针*/
	MYSQL_FIELD *field; /*字段结构指针*/
	MYSQL_ROW result_row; /*按行返回的查询信息*/
	int row, column; /*查询返回的行数和列数*/
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
				printf("flag = %s\n",result_row[4]);
				strcpy(flag,result_row[4]);
				return 1;
			}
		}
		else
			return 0;
	}
}
int get_file(int accept)
{
	char name[50];
	int len;
	struct msg *rm,*sm;
	rm = (struct msg*)malloc(3000);
	sm = (struct msg*)malloc(3000);
	FILE *fp;
	while(1)
	{
		len = recv(accept,(void*)rm,sizeof(struct msg)+500,0);
		//printf("	get_file===accept_fd:%d,rm->type:%d,rm->data:%s\n",accept,rm->type,rm->data);
		if(len > 0 && rm->type == MSG_FILENAME)
		{
			memset(name,0,50);
			strcpy(name,rm->data);

			strcat(file_list,name);
			strcat(file_list," ");

			printf("file name:%s\n",name);
			if( (fp=fopen(name,"w")) == NULL)
			{
			    printf("FILE ERROR!\n");
			    exit(0);
			}
			sm->type = MSG_ACK;
			sm->data_len = 0;
			if(send(accept,(void*)sm,3000,0) == -1)
			{
				print_err(__LINE__,"send msg",errno);
				break;
			}
		}
		else if(len > 0 && rm->type == MSG_CONTENT)
		{	
			//printf("%s",rm->data);
			fprintf(fp,"%s",rm->data);
			sm->type = MSG_ACK;
			sm->data_len = 0;
			if(send(accept,(void*)sm,3000,0) == -1)
			{
				print_err(__LINE__,"send msg",errno);
				break;
			}
		}
        else if(len > 0 && rm->type == MSG_FILEDONE)
        {
            sm->type = MSG_ACK;
			sm->data_len = 0;
			if(send(accept,(void*)sm,3000,0) == -1)
			{
				print_err(__LINE__,"send msg",errno);
				break;
			}
			printf("write done !\n");
            break;
        }
    }
    fclose(fp);
	return 0;
}
int push_file(int accept)
{
	struct msg *sm,*rm;

	sm = (struct msg*)malloc(3000);
	rm = (struct msg*)malloc(3000);
	
	sm->type = MSG_ACK;
	sm->data_len = strlen(file_list);
	strcpy(sm->data,file_list);
	if( send(accept,(void*)sm,sizeof(struct msg)+sm->data_len,0) == -1 )
	{
		print_err(__LINE__,"send file list",errno);
		exit(0);
	}
	if( recv(accept,(void*)rm,3000,0) == -1 )
	{
		print_err(__LINE__,"recv conform",errno);
		return 0;
	}
	char f_name[50]={0};
	if(rm->type == MSG_CONFFILE)
		strcpy(f_name,rm->data);
	sm->type = MSG_FILENAME;
	strcpy(sm->data,f_name);
	sm->data_len = strlen(f_name);
	memcpy(sm->data,f_name,sm->data_len);

	if( send(accept,(void*)sm,sizeof(struct msg)+sm->data_len,0) == -1 )
	{
		print_err(__LINE__,"send name",errno);
		return 0;
	}
	if( recv(accept,(void*)rm,3000,0) == -1 )
	{
		print_err(__LINE__,"recv ACK",errno);
		return 0;
	}
	FILE *fp;
    if( (fp = fopen(f_name,"r")) == NULL)
    {
            printf("FILE OPEN ERROR!\n");
            return 0;
    }
	while( !feof(fp) )
	{
		char buff[100] = {0};
		int len = 0;
		len = fread(buff,sizeof(buff)-1,1,fp);
		sm->type = MSG_CONTENT;
		sm->data_len = sizeof(buff);
		strcpy(sm->data,buff);
		if( send(accept,(void*)sm,sizeof(struct msg)+100,0) == -1)
		{
			print_err(__LINE__,"send name",errno);
			exit(0);
		}
		if( recv(accept,(void*)rm,3000,0) == -1 )
		{
			print_err(__LINE__,"recv ACK",errno);
			exit(0);
		}
	}
	sm->type = MSG_FILEDONE;
	sm->data_len = 0;
	if( send(accept,(void*)sm,sizeof(struct msg)+sm->data_len,0) == -1)
	{
		print_err(__LINE__,"send file done",errno);
		return 0;
	}
	if( recv(accept,(void*)rm,3000,0) == -1 )
	{
		print_err(__LINE__,"recv ACK",errno);
		return 0;
	}
	fclose(fp);
	return 1;
}
void *deal_data()
{
		int len;
		struct msg *sm,*rm;
		sm = (struct msg*)malloc(3000);
		rm = (struct msg*)malloc(3000);
		char ID[10] = {0};
		char NAME[20] = {0};
		int accept = accept_fd;
		while(1)
		{
			len = recv(accept,(void*)rm,3000,0);
			printf("	accept_fd:%d,rm->type:%d,rm->data:%s\n",accept,rm->type,rm->data);
			if(len > 0 && rm->type == MSG_LOGIN)
			{
				sm->type = MSG_ACK;
				if( comp_passwd(rm->data) == 1)
				{
					pthread_mutex_lock(&lock);
					member[mem_len].gro_id = 0;
					int i;
					for(i=0;rm->data[i] != ' ';i++)
					{
						ID[i] = rm->data[i];
						member[mem_len].gro_id = member[mem_len].gro_id * 10 + rm->data[i]-'0';
					}
					ID[i] = 0;
					if(search_name(ID,NAME) == 0)
					{
						printf("search id:%s...no such user!\n",ID);
						exit(-1);
					}
					printf("ID:%s\n",ID);
					member[mem_len].acp_fd = accept;
					mem_len++;
					pthread_mutex_unlock(&lock);
					//printf("id=%d\n",member[mem_len-1].gro_id);
					//printf("accept_fd = %d\n",member[mem_len-1].acp_fd);
					strcpy(sm->data,NAME);
				}
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
				if(send(accept,(void*)sm,2048+sm->data_len,0) == -1)
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
				if(send(accept,(void*)sm,2048+sm->data_len,0) == -1)
				{
					print_err(__LINE__,"send msg",errno);
					break;
				}
			}
			else if(len > 0 && rm->type == MSG_GROUP)
			{
				usleep(50);
				printf("ID:%s\n",ID);
				printf("NAME:%s\n",NAME);
				memset(sm->data,0,2200);
				strcpy(sm->data,rm->data);
				printf("%s\n",rm->data);
				sm->type = MSG_ACK;
				sm->data_len = strlen(sm->data);
				for(int i=0;i < mem_len;i++)
				{
					if(send(member[i].acp_fd,(void*)sm,2048+sm->data_len,0) == -1)
					{
						printf("Send message to %d error!\n",member[i].gro_id);
					}
				}
			}
			else if(len > 0 && rm->type == MSG_ALONE)
			{
				usleep(50);
				printf("ID:%s\n",ID);
				printf("NAME:%s\n",NAME);
				memset(sm->data,0,2200);
				printf("%s\n",rm->data);
				strcpy(sm->data,rm->data);
				sm->type = MSG_ACK;
				sm->data_len = strlen(sm->data);
				int i;
				for(i=0;i < mem_len;i++)
				{
					if(member[i].gro_id == rm->recv_id)
					{
						if(send(member[i].acp_fd,(void*)sm,2048+sm->data_len,0) == -1)
							printf("Send message to %d error!\n",member[i].gro_id);
						break;
					}
				}
				if( i >= mem_len)
				{
					memset(sm->data,0,2200);
					strcpy(sm->data,"send error!(may be no such user)");
					sm->data_len = strlen(sm->data);
					if(send(member[i].acp_fd,(void*)sm,2048+sm->data_len,0) == -1)
							printf("Send message to %d error!\n",member[i].gro_id);
				}
			}
			else if(len > 0 && rm->type == MSG_KICK)
			{
				sm->type = MSG_ACK;
				char buff[20]={0};
				search_name(rm->data,buff);
				printf("lord_id=%s,ID=%s\n",lord_id,ID);
				if(strcmp(lord_id,NAME) != 0)
					strcpy(sm->data,"noright");
				else if(strcmp(buff,"") == 0)
					strcpy(sm->data,"faild");
				else if(kick_member(rm->data) == 1)
					strcpy(sm->data,"OK");
				sm->data_len = strlen(sm->data);
				if(send(accept,(void*)sm,2048+sm->data_len,0) == -1)
				{
					print_err(__LINE__,"send msg",errno);
					break;
				}
			}
			else if(len > 0 && rm->type == MSG_BAN)
			{
				sm->type = MSG_ACK;
				char buff[20]={0};
				search_name(rm->data,buff);
				//printf("lord_id=%s,ID=%s\n",lord_id,ID);
				if(strcmp(lord_id,NAME) != 0)
					strcpy(sm->data,"noright");
				else if(strcmp(buff,"") == 0)
					strcpy(sm->data,"faild");

				else if(ban(rm->data) == 1)
					strcpy(sm->data,"OK");

				sm->data_len = strlen(sm->data);
				if(send(accept,(void*)sm,2048+sm->data_len,0) == -1)
				{
					print_err(__LINE__,"send msg",errno);
					break;
				}
			}
			else if(len > 0 && rm->type == MSG_UNBAN)
			{
				sm->type = MSG_ACK;
				char buff[20]={0};
				search_name(rm->data,buff);
				//printf("lord_id=%s,ID=%s\n",lord_id,ID);
				if(strcmp(lord_id,NAME) != 0)
					strcpy(sm->data,"noright");
				else if(strcmp(buff,"") == 0)
					strcpy(sm->data,"faild");
				else if(unban(rm->data) == 1)
					strcpy(sm->data,"OK");
				sm->data_len = strlen(sm->data);
				if(send(accept,(void*)sm,2048+sm->data_len,0) == -1)
				{
					print_err(__LINE__,"send msg",errno);
					break;
				}
			}
			else if(len > 0 && rm->type == MSG_GETBAN)
			{
				char flag[10]={0};
				get_ban(ID,flag);
				sm->type = MSG_ACK;
				strcpy(sm->data,flag);
				sm->data_len = strlen(sm->data);
				if(send(accept,(void*)sm,2048+sm->data_len,0) == -1)
				{
					print_err(__LINE__,"send msg",errno);
					break;
				}
			}
			else if(len > 0 && rm->type == MSG_UPLOAD)
			{
				get_file(accept);
			}
			else if(len > 0 && rm->type == MSG_DOWNLOAD)
			{
				push_file(accept);
			}
			else if(len > 0 && rm->type == MSG_SEARCH)
			{
				sm->type = MSG_ACK;
				memset(sm->data,0,3000);
				printf("mem_len:%d\n",mem_len);
				sm->data[0] = mem_len;
				printf("sm->data:%d\n",(int)sm->data[0])
				sm->data_len = strlen(sm->data);
				if(send(accept,(void*)sm,2048+sm->data_len,0) == -1)
				{
					printf("Send message to number!\n");
				}
			}
			else if(len > 0 && rm->type == MSG_DONE)
			{
				int i;
				for(i=0;i<mem_len;i++)
					if(accept == member[i].acp_fd)
						break;
				for(;i<mem_len-1;i++)
					member[i] = member[i+1];
				mem_len--;
				break;
			}
			else
				break;
		}
		free(sm);
		free(rm);
		close(accept);
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
	search_lord(lord_id);

	signal(SIGINT,sigfun);

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
			int ret;
			ret = pthread_create(&pth_id[pth_len++],NULL,(void*)deal_data,NULL);
			if(ret != 0)
			{
				printf("Create pthread error!\n");
				exit(-1);
			}
			if(pth_len > 49)
			{
				printf("pthread not enough!\n");
				exit(-1);
			}
		}
	}
	close(socket_fd);
	mysql_close(conn);
	return 0;
}