#include "./include/client_head.h"


void print_err(int line,char *str,int err_num)
{
	printf("In line:%d ,%s error ,error type: %d\n",line,str,err_num);
}
int app_connect()
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

	/*connect*/
	if( (connect_fd = connect(socket_fd,(struct sockaddr*)&servaddr,sizeof(servaddr))) == -1 )
	{
		print_err(__LINE__,"connect",errno);
		exit(0);
	}
	printf("connect successfully!\n");
	return 0;
}
int main()
{
	app_connect();
	printf("\nWelcome to GROUP CHAT system!\n");
	int select=0;
	while(select!=4)
	{
          printf("\t┍┉┉┉┉┉┉┉┉┉┉┉┉┉┉┉┉┉┉┉┉┉┉┉┉┉┉┉┉┉┉┉┉┉┉┉┑\n");
		  printf("\t┋               MENU                ┋\n");
          printf("\t┍┉┉┉┉┉┉┉┉┉┉┉┉┉┉┉┉┉┉┉┉┉┉┉┉┉┉┉┉┉┉┉┉┉┉┉┑");
		printf("\n\t┋1. 	用户 登录 		    ┋\n");
		printf("\n\t┋2.	用户 更改密码               ┋\n");
		printf("\n\t┋3.	用户 注册                   ┋\n");
		printf("\n\t┋4.	用户 退出                   ┋\n");
          printf("\t┕┉┉┉┉┉┉┉┉┉┉┉┉┉┉┉┉┉┉┉┉┉┉┉┉┉┉┉┉┉┉┉┉┉┉┉┙\n");
		printf("\nPlease choose the function: ");
		scanf("%d",&select);
        system("clear");
		if(select == 1)
			cli_login();
		else if(select == 2)
			cli_changpd();
		else if(select == 3)
			cli_reg();
		else if(select > 4 || select < 1)
			printf("\n\tInput error! Please try again!\n");
	}
	printf("\n\tThank you for using!\n");
	close(socket_fd);
	close(connect_fd);
	return 0;
}