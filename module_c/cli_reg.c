#include "../include/client_head.h"
void cli_reg()
{
	char id[10],user[10],passwd[20];
	struct msg *sm,*rm;
	int flag_correct=0;
	sm = (struct msg*)malloc(3000);
	rm = (struct msg*)malloc(3000);
	while(1)
	{
		memset(id,0,sizeof(id));
		memset(passwd,0,sizeof(id));
		printf("Please input the user ID:(press 'end' to exit!)\n");
		scanf("%s",id);
		if( strcmp(id,"end") == 0)
			break;
		printf("Please input the user NAME:\n");
		scanf("%s",user);
		printf("Please input the user PASSWORD:\n");
		scanf("%s",passwd);
		char message[50]={0};
		sprintf(message,"%s %s %s",id,user,passwd);
		sm->type = MSG_REG;
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
		if(strcmp(rm->data,"correct") == 0)
		{
			printf("注册成功！\n");
			break;
		}
		else
		{
			printf("注册失败！(用户名重复或数据库错误)\n");
			printf("请重新输入:\n");
		}
	}
	
}