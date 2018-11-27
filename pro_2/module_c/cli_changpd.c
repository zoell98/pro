#include "../include/client_head.h"
void cli_changpd()
{
	struct msg *sm,*rm;
	int flag_correct=0;
	sm = (struct msg*)malloc(3000);
	rm = (struct msg*)malloc(3000);
	char new_passwd[20]={0};
	while(1)
	{
		memset(id,0,sizeof(id));
		memset(passwd,0,sizeof(id));
		printf("Please input the user ID:(press 'end' to exit!)\n");
		scanf("%s",id);
		if(strcmp(id,"end") == 0)
		{
			break;
		}
		printf("Please input the user PASSWORD:\n");
		scanf("%s",passwd);
		printf("Please input the user NEW PASSWORD:\n");
		scanf("%s",new_passwd);
		char message[50]={0};
		sprintf(message,"%s %s %s",id,passwd,new_passwd);
		sm->type = MSG_CHANGPD;
		sm->data_len = strlen(message);
		memcpy(sm->data,message,sm->data_len);
		if( send(socket_fd,(void*)sm,sizeof(struct msg)+sm->data_len,0) == -1 )
		{
			print_err(__LINE__,"send id  passwd and new passwd",errno);
			exit(-1);
		}
		if( recv(socket_fd,(void*)rm,3000,0) == -1 )
		{
			print_err(__LINE__,"recv ACK",errno);
			exit(-1);
		}
		if(strcmp(rm->data,"success") == 0)
		{
			printf("Change password success!\n");
			break;
		}
		else
			printf("Change password failed!\n");
	}
}