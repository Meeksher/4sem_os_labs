#include <stdio.h>
#include <iostream>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>
#include <string>
#include <semaphore.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <fstream>
#include <sys/socket.h>
#include <sys/types.h>
#include <queue>
#include <cstring>
#include <sys/un.h>
#define MAX_BUF 1024
using namespace std;


int flag_rcv, flag_response, flag_waitconnect;
int sock_server;
pthread_t id_recieve, id_response, id_waitconnect; 




void * rsp_func(void * params)
{
	while(flag_response)
	{
		sleep(1);

        string message = "Message #" + to_string(rand() %10);

		if(send(sock_server, message.c_str(), message.length(), MSG_NOSIGNAL) > 0) cout << "Response \"" << message << "\" sent." << endl;
		else perror("send");

		if(errno == EPIPE) { cout << "Connection has lost." << endl; return (void*) new int (0); }
	}


    return (void*) new int (0);
}

void * rcv_func(void * params)
{
	while(flag_rcv)
	{
		char buf[MAX_BUF];

		int rcv = recv(sock_server, buf, MAX_BUF, MSG_NOSIGNAL);
		if(rcv > 0) cout << "Recieved: " << buf << ", " << rcv << " bytes." << endl;
		if(errno == EPIPE)  return (void*) new int (0);
		sleep(1);
	}


    return (void*) new int (0);
}

void * connect_func(void * params)
{	
	const char name_sock_l[] = "/tmp/socknamel8listener";

	struct sockaddr_un sun;
	memset(&sun,0,sizeof sun);
	sun.sun_family = AF_UNIX;
	//точно ли sizeof правилен?
	strncpy(sun.sun_path, name_sock_l, sizeof(sun.sun_path) - 1);

	while(flag_waitconnect)
	{
		cout << "Trying to connect.." << endl;
		int connect_res = connect(sock_server,(struct sockaddr *)&sun, sizeof(sun));

		if(connect_res  == 0)
		{
			cout << "Connected." << endl;

			if(pthread_create(&id_response, 0, rsp_func, 0) == 0) cout << "Thread Responsing started." << endl;
			else perror("pthread_create");
			if(pthread_create(&id_recieve, 0, rcv_func, 0) == 0) cout << "Thread Recieving started." << endl;
			else perror("pthread_create");

			return (void*) new int (0);
		}
		sleep(1);
	}


    return (void*) new int (0);
}


int main()
{
	const char name_sock_l[] = "/tmp/socknamel8listener";


	flag_rcv = flag_response = flag_waitconnect = 1;

	sock_server = socket(AF_UNIX, SOCK_NONBLOCK | SOCK_STREAM, 0);

	if(sock_server > 0) cout << "sock_server initiliazied." << endl;
	else perror("socket");


	if(pthread_create(&id_waitconnect, 0, connect_func, 0) == 0) cout << "Thread Connection started." << endl;
	else perror("pthread_create");


	getchar();

	flag_rcv  = flag_response = flag_waitconnect = 0;

	pthread_join(id_recieve, 0);
	pthread_join(id_response, 0);
	pthread_join(id_waitconnect, 0);

	cout << "Joined all." << endl;

	if(close(sock_server) == 0) cout << "sock_server closed." << endl;
	else perror("close");


	return 0;
}