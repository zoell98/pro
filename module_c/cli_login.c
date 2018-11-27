#include "../include/client_head.h"

void killthread()
{
	pthread_exit(0);
}

void get_banflag()
{
	struct msg *sm,*rm;
	sm = (struct msg*)malloc(3000);
	rm = (struct msg*)malloc(3000);

	sm->type = MSG_GETBAN;
	sm->data_len = 0;
	//printf("		send:%s\n",sm->data);
	if( send(socket_fd,(void*)sm,sizeof(struct msg)+sm->data_len,0) == -1 )
	{
		print_err(__LINE__,"send group message",errno);
		exit(-1);
	}
	if( recv(socket_fd,(void*)rm,3000,0) == -1 )
	{
		print_err(__LINE__,"recv ACK",errno);
		exit(-1);
	}
	ban_flag = atoi(rm->data);
}
int login()
{
	struct msg *sm,*rm;
	int flag_correct=0;
	sm = (struct msg*)malloc(3000);
	rm = (struct msg*)malloc(3000);
	while(1)
	{
		memset(id,0,sizeof(id));
		memset(passwd,0,sizeof(id));
		printf("\tPlease input the user ID:(press 'end' to exit!)\n");
		scanf("%s",id);
		if(strcmp(id,"end") == 0)
		{
			return 0;
		}
		printf("\tPlease input the user PASSWORD:\n");
		scanf("%s",passwd);
		char message[50]={0};
		strcpy(message,id);
		strcat(message," ");
		strcat(message,passwd);
		sm->type = MSG_LOGIN;
		sm->data_len = strlen(message);
		memcpy(sm->data,message,sm->data_len);
		if( send(socket_fd,(void*)sm,sizeof(struct msg)+sm->data_len,0) == -1 )
		{
			print_err(__LINE__,"send id and passwd",errno);
			exit(-1);
		}
		if( recv(socket_fd,(void*)rm,3000,0) == -1 )
		{
			print_err(__LINE__,"recv ACK",errno);
			exit(-1);
		}
		if(strcmp(rm->data,"incorrect") != 0)
		{
			printf("\tPASSWORD correct!\n");
			strcpy(name,rm->data);
			return 1;
		}
	}
}
void *recv_data()
{
	signal(SIGQUIT,killthread);
	struct msg *rm;
	rm = (struct msg*)malloc(3000);
	while(1)
	{
		if( recv(socket_fd,(void*)rm,3000,0) == -1 )
		{
			print_err(__LINE__,"recv ACK",errno);
			exit(-1);
		}
		else
			printf("%s\n",rm->data);
	}
}
int group_chat()
{
	char message[100];

	struct msg *sm,*rm;
	sm = (struct msg*)malloc(3000);
	rm = (struct msg*)malloc(3000);
	pthread_t tid;
	int ret;
	ret = pthread_create(&tid, NULL, (void*)recv_data, NULL);
	if(ret != 0)
	{
		printf("\tCreate pthread error!\n");
		exit(-1);
	}
	while(1)
	{
		memset(message,0,100);
		scanf("%s",message);
		if(strcmp(message,"#quit") == 0)
		{
			int pthread_kill_ret;
			pthread_kill_ret = pthread_kill(tid,SIGQUIT);
			return 0;
		}
		if(ban_flag == 1)
		{
			memset(sm->data,0,3000);
			sm->type = MSG_GROUP;
			sprintf(sm->data,"%s say:%s",name,message);
			sm->data_len = strlen(sm->data);
			//printf("		send:%s\n",sm->data);
			if( send(socket_fd,(void*)sm,sizeof(struct msg)+sm->data_len,0) == -1 )
			{
				print_err(__LINE__,"send group message",errno);
				exit(-1);
			}
		}
		else
			printf("\tYOU HAVE BEEN BANED!\n");
	}
	return 0;
}
int alone_chat(int target_id)
{
	char message[100];
	struct msg *sm,*rm;
	sm = (struct msg*)malloc(3000);
	rm = (struct msg*)malloc(3000);
	pthread_t tid;
	int ret;
	ret = pthread_create(&tid, NULL, (void*)recv_data, NULL);
	if(ret != 0)
	{
		printf("\tCreate pthread error!\n");
		exit(-1);
	}
	sm->recv_id = target_id;
	while(1)
	{
		memset(message,0,100);
		scanf("%s",message);
		if(strcmp(message,"#quit") == 0)
		{
			return 0;
		}
		memset(sm->data,0,3000);
		sm->type = MSG_ALONE;
		sprintf(sm->data,"%s say:%s",name,message);
		sm->data_len = strlen(sm->data);
		//printf("		send:%s\n",sm->data);
		if( send(socket_fd,(void*)sm,sizeof(struct msg)+sm->data_len,0) == -1 )
		{
			print_err(__LINE__,"send group message",errno);
			exit(-1);
		}
	}
	return 0;
}
void kick()
{
	struct msg *sm,*rm;
	char kick_id[10];
	sm = (struct msg*)malloc(3000);
	rm = (struct msg*)malloc(3000);
	printf("\tPlease input kick member's ID:\n");
	scanf("%s",kick_id);
	sm->type = MSG_KICK;
	memcpy(sm->data,kick_id,strlen(kick_id));
	sm->data_len = strlen(sm->data);
	if( send(socket_fd,(void*)sm,sizeof(struct msg)+sm->data_len,0) == -1 )
	{
		print_err(__LINE__,"kick member",errno);
		exit(-1);
	}
	if( recv(socket_fd,(void*)rm,3000,0) == -1 )
	{
		print_err(__LINE__,"recv ACK",errno);
		exit(-1);
	}
	if(strcmp(rm->data,"OK") == 0)
		printf("\tKICK SUCCESS!\n");
	else if(strcmp(rm->data,"noright") == 0)
		printf("\tYou don't have right!\n");
	else
		printf("\tKICK FAILED!\n");
}
void ban()
{
	struct msg *sm,*rm;
	char ban_id[10];
	sm = (struct msg*)malloc(3000);
	rm = (struct msg*)malloc(3000);
	printf("\tPlease input BAN member's ID:\n");
	scanf("%s",ban_id);
	sm->type = MSG_BAN;
	memcpy(sm->data,ban_id,strlen(ban_id));
	sm->data_len = strlen(sm->data);
	if( send(socket_fd,(void*)sm,sizeof(struct msg)+sm->data_len,0) == -1 )
	{
		print_err(__LINE__,"ban member",errno);
		exit(-1);
	}
	if( recv(socket_fd,(void*)rm,3000,0) == -1 )
	{
		print_err(__LINE__,"recv ACK",errno);
		exit(-1);
	}
	if(strcmp(rm->data,"OK") == 0)
		printf("\tBAN SUCCESS!\n");
	else if(strcmp(rm->data,"noright") == 0)
		printf("\tYou don't have right!\n");
	else
		printf("\tBAN FAILED!\n");
}
void unban()
{
	struct msg *sm,*rm;
	char unban_id[10];
	sm = (struct msg*)malloc(3000);
	rm = (struct msg*)malloc(3000);
	printf("\tPlease input UNBAN member's ID:\n");
	scanf("%s",unban_id);
	sm->type = MSG_UNBAN;
	memcpy(sm->data,unban_id,strlen(unban_id));
	sm->data_len = strlen(sm->data);
	if( send(socket_fd,(void*)sm,sizeof(struct msg)+sm->data_len,0) == -1 )
	{
		print_err(__LINE__,"unben member",errno);
		exit(-1);
	}
	if( recv(socket_fd,(void*)rm,3000,0) == -1 )
	{
		print_err(__LINE__,"recv ACK",errno);
		exit(-1);
	}
	if(strcmp(rm->data,"OK") == 0)
		printf("\tUNBAN SUCCESS!\n");
	else if(strcmp(rm->data,"noright") == 0)
		printf("\tyou don't have right!\n");
	else
		printf("\tUNBAN FAILED!\n");
}
void upload()
{
	struct msg *sm,*rm;

	sm = (struct msg*)malloc(3000);
	rm = (struct msg*)malloc(3000);
	
	sm->type = MSG_UPLOAD;
	memset(sm->data,0,3000);
	sm->data_len = 0;
	if( send(socket_fd,(void*)sm,sizeof(struct msg)+sm->data_len,0) == -1 )
	{
		print_err(__LINE__,"send name",errno);
		exit(0);
	}

	char filename[50]={0};
	printf("\tPlease input file name:\n");
	scanf("%s",filename);

	int i,j;
	char f_name[50]={0};
	for(i=strlen(filename)-1;i>=0;i--)
		if(filename[i] == '/')
			break;
	printf("i=%d\n",i);
	for(j=0,i=i+1;filename[i];i++)
		f_name[j++] = filename[i];
	f_name[j]=0;
	printf("f_name:%s\n",f_name);
	sm->type = MSG_FILENAME;
	sm->data_len = strlen(f_name);
	strcpy(sm->data,f_name);
	printf("sm->data:%s\n",sm->data);
	if( send(socket_fd,(void*)sm,sizeof(struct msg)+sm->data_len,0) == -1 )
	{
		print_err(__LINE__,"send name",errno);
		return;
	}
	if( recv(socket_fd,(void*)rm,3000,0) == -1 )
	{
		print_err(__LINE__,"recv ACK",errno);
		return;
	}
	FILE *fp;
    if( (fp = fopen(filename,"r")) == NULL)
    {
            printf("\tFILE OPEN ERROR!\n");
            return;
    }
	while( !feof(fp) )
	{
		char buff[100] = {0};
		int len = 0;
		len = fread(buff,sizeof(buff)-1,1,fp);
		sm->type = MSG_CONTENT;
		sm->data_len = sizeof(buff);
		strcpy(sm->data,buff);
		if( send(socket_fd,(void*)sm,sizeof(struct msg)+100,0) == -1)
		{
			print_err(__LINE__,"send name",errno);
			exit(0);
		}
		if( recv(socket_fd,(void*)rm,3000,0) == -1 )
		{
			print_err(__LINE__,"recv ACK",errno);
			exit(0);
		}
	}
	sm->type = MSG_FILEDONE;
	sm->data_len = 0;
	if( send(socket_fd,(void*)sm,sizeof(struct msg)+sm->data_len,0) == -1)
	{
		print_err(__LINE__,"send name",errno);
		exit(0);
	}
	if( recv(socket_fd,(void*)rm,3000,0) == -1 )
	{
		print_err(__LINE__,"recv ACK",errno);
		exit(0);
	}
	fclose(fp);
}
void download()
{
	struct msg *sm,*rm;

	sm = (struct msg*)malloc(3000);
	rm = (struct msg*)malloc(3000);

	int len;
	
	sm->type = MSG_DOWNLOAD;
	sm->data_len = 0;
	memset(sm->data,0,3000);
	if( send(socket_fd,(void*)sm,sizeof(struct msg)+sm->data_len,0) == -1 )
	{
		print_err(__LINE__,"send name",errno);
		exit(0);
	}
	if( (len = recv(socket_fd,(void*)rm,3000,0)) == -1 )
	{
		print_err(__LINE__,"recv ACK",errno);
		exit(0);
	}
	if(len > 0)
		printf("file list:\n%s\n",rm->data);

	char filename[50]={0};
	printf("\tPlease input download file name:\n");
	scanf("%s",filename);
	if( strstr(rm->data,filename) == NULL )
	{
		printf("\tNo such file!\n");
		return ;
	}
	sm->type = MSG_CONFFILE;
	strcpy(sm->data,filename);
	sm->data_len = strlen(filename);
	if( send(socket_fd,(void*)sm,sizeof(struct msg)+sm->data_len,0) == -1 )
	{
		print_err(__LINE__,"send name",errno);
		exit(0);
	}
	FILE *fp;
	while(1)
	{
		len = recv(socket_fd,(void*)rm,sizeof(struct msg)+100,0);
        printf("%d\n",len);
        printf("%d\n",rm->type);
		if(len > 0 && rm->type == MSG_FILENAME)
		{
			char file[50];
			memset(file,0,50);
			strcpy(file,rm->data);
			strcat(file,"(client)");
			printf("%s\n",file);
			if( (fp=fopen(file,"w")) == NULL)
			{
			    printf("\tFILE ERROR!\n");
			    exit(0);
			}
			sm->type = MSG_ACK;
			sm->data_len = 0;
			if(send(socket_fd,(void*)sm,3000,0) == -1)
			{
				print_err(__LINE__,"send msg",errno);
				break;
			}
		}
		else if(len > 0 && rm->type == MSG_CONTENT)
		{	
			printf("%s",rm->data);
			fprintf(fp,"%s",rm->data);
			sm->type = MSG_ACK;
			sm->data_len = 0;
			if(send(socket_fd,(void*)sm,3000,0) == -1)
			{
				print_err(__LINE__,"send msg",errno);
				break;
			}
		}
        else if(len > 0 && rm->type == MSG_FILEDONE)
        {
            sm->type = MSG_ACK;
			sm->data_len = 0;
			if(send(socket_fd,(void*)sm,3000,0) == -1)
			{
				print_err(__LINE__,"send msg",errno);
				break;
			}
			printf("\tWrite done !\n");
            break;
        }
    }
    fclose(fp);
}
void file()
{
	int select;
	system("clear");
	printf("\t-----1.UPLOAD-----\n");
	printf("\t-----2.DOWNLOAD---\n");
	printf("\t-----3.EXIT-------\n");
	scanf("%d",&select);
	if( select == 1)
		upload();
	else if(select == 2)
		download();
	else
		return;
}
void logout()
{
	struct msg *sm,*rm;
	sm = (struct msg*)malloc(3000);
	rm = (struct msg*)malloc(3000);
	sm->type = MSG_DONE;
	memset(sm->data,0,3000);
	sm->data_len = 0;
	if( send(socket_fd,(void*)sm,sizeof(struct msg)+sm->data_len,0) == -1 )
	{
		print_err(__LINE__,"logout",errno);
		exit(-1);
	}
}
int cli_login()
{
	int flag_login = 0;
	flag_login = login();
	if(flag_login == 0)
		return 0;
	else
	{
		printf("\tUSER LOGIN SUCCESS!\n");
		printf("\tWelcome User :%s\n",name);
	}
	get_banflag();
	printf("\tBan_flag = %d\n",ban_flag);
	/*LOGIN and USE*/
	int select=0;
	while(select!=7)
	{
          printf("\t┍┉┉┉┉┉┉┉┉┉┉┉┉┉┉┉┉┉┉┉┉┉┉┉┉┉┉┉┉┉┉┉┉┉┉┉┑\n");
		  printf("\t┋               MENU                ┋\n");
          printf("\t┍┉┉┉┉┉┉┉┉┉┉┉┉┉┉┉┉┉┉┉┉┉┉┉┉┉┉┉┉┉┉┉┉┉┉┉┑");
		printf("\n\t┋             1. 群聊               ┋\n");
		printf("\n\t┋             2. 私聊               ┋\n");
		printf("\n\t┋             3. 踢人               ┋\n");
		printf("\n\t┋             4. 禁言               ┋\n");
		printf("\n\t┋             5. 解禁               ┋\n");
		printf("\n\t┋             6. 文件               ┋\n");
		printf("\n\t┋             7. 注销               ┋\n");
          printf("\t┕┉┉┉┉┉┉┉┉┉┉┉┉┉┉┉┉┉┉┉┉┉┉┉┉┉┉┉┉┉┉┉┉┉┉┉┙\n");
		printf("\nPlease choose the function: ");
		scanf("%d",&select);
        system("clear");
		if(select == 1)
		{
			printf("\tPlease input the message:(press '#quit' to exit!)\n");
			group_chat();
		}
		else if(select == 2)
		{
			int target_id;
			printf("\tPlease input the recv's id:\n");
			scanf("%d",&target_id);
			printf("\tPlease input the alone message:(press '#quit' to exit!)\n");
			alone_chat(target_id);
		}
		else if(select == 3)
		{
			kick();
		}
		else if(select == 4)
			ban();
		else if(select == 5)
			unban();
		else if(select == 6)
			file();
		else if(select == 7)
			logout();
		else if(select > 7 || select < 1)
			printf("\n\tInput error! Please try again!\n");
	}
	return 1;
}