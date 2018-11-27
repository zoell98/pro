#include <stdio.h>
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
#include <unistd.h>
#include <time.h>
#include <mysql/mysql.h>

#define SER_ADDR "127.0.0.1"
#define MSG_LOGIN	    1
#define MSG_REG			2
#define MSG_CHANGPD		3
#define MSG_GROUP	    4
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
int cli_login();
void cli_reg();
void cli_changpd();
void print_err(int line,char *str,int err_num);

int socket_fd;
int connect_fd;

char id[10],passwd[20],name[20];

int ban_flag;
