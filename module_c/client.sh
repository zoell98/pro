gcc -c cli_login.c
gcc -c cli_reg.c
gcc -c cli_changpd.c
ar rsv libclient.a cli_login.o cli_reg.o cli_changpd.o

