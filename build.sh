gcc pt_s.c -o pt_s -lpthread -lmysqlclient
cd module_c
bash client.sh
cd ..
gcc client_main.c -o main -I ./include -L ./module_c -lclient -lpthread
gnome-terminal -t "client" -x bash -c "cd ~/pro_2;exec bash;"
sudo ./pt_s


