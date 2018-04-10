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


int flag_rcv, flag_perform, flag_response, flag_waitconnect;
int sock_listen, sock_client;
queue<char*> q_perform, q_response;
pthread_t id_recieve, id_perform, id_response, id_waitconnect; 


void * rcv_func(void * params)
{
	while(flag_rcv)
	{
		char buf[MAX_BUF];

		//по идее создается новый сокет и сюда передается, например;
		int rcv = recv(sock_client, buf, MAX_BUF, 0);

		if(rcv > 0)
		{		
			cout << "Recieved: " << buf << ", " << rcv << " bytes." << endl;
			q_perform.push(buf);
		} 

		sleep(1);
	}


    return (void*) new int (0);
}

void * perform_func(void * params)
{
	while(flag_perform)
	{
		if(q_perform.size() == 0){
			sleep(1);
			continue;
		}

		char * msg = q_perform.front();
		q_perform.pop();

		// gen answer
		msg[1] = '1';
		msg[2] = '2';
		q_response.push(msg);
		sleep(1);
	}


    return (void*) new int (0);
}

void * response_func(void * params)
{
	while(flag_response)
	{
		if(q_response.size() == 0){
			sleep(1);
			continue;
		}

		char * msg = q_response.front();
		q_response.pop();

		if(send(sock_client, msg, strlen(msg), 0) > 0) cout << "Response \"" << msg << "\" sent." << endl;
		else perror("send");

		sleep(1);
	}
}

void * listening_func(void * params) {
	while(flag_waitconnect)
	{
		sock_client = accept(sock_listen, 0, 0);

		if(sock_client > 0) cout << "Connected new device." << endl;
		else perror("accept");

		if(sock_client > 0)
		{
			if(pthread_create(&id_recieve, 0, rcv_func, 0) == 0) cout << "Thread Recieving started." << endl;
			else perror("pthread_create");
			if(pthread_create(&id_perform, 0, perform_func, 0) == 0) cout << "Thread Performing started." << endl;
			else perror("pthread_create");
			if(pthread_create(&id_response, 0, response_func, 0) == 0) cout << "Thread Responsing started." << endl;
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
	unlink(name_sock_l);


	flag_rcv = flag_perform = flag_response = flag_waitconnect = 1;

	//инициализация слушающего сокета
	sock_listen = socket(AF_UNIX, SOCK_NONBLOCK | SOCK_STREAM, 0);

	if(sock_listen > 0) cout << "sock_listen initiliazied." << endl;
	else perror("socket");

    struct sockaddr_un sun;
    memset(&sun,0,sizeof sun);

    sun.sun_family = AF_UNIX;
    //точно ли sizeof правилен?
    strncpy(sun.sun_path, name_sock_l, sizeof(sun.sun_path) - 1);

    if( bind(sock_listen, (struct sockaddr *)&sun, sizeof(sun)) == 0) cout << "sock_listen binded." << endl;
    else perror("bind");

    if( listen(sock_listen, 2) == 0) cout << "sock_listen in listening mode." << endl;
    else perror("listen");


	if(pthread_create(&id_waitconnect, 0, listening_func, 0) == 0) cout << "Thread Listening started." << endl;
	else perror("pthread_create");


	getchar();

	flag_rcv = flag_perform = flag_response = flag_waitconnect = 0;

	pthread_join(id_recieve, 0);
	pthread_join(id_perform, 0);
	pthread_join(id_response, 0);
	pthread_join(id_waitconnect, 0);

	cout << "Joined all." << endl;

	if(close(sock_client) == 0) cout << "sock_client closed." << endl;
	else perror("close");

	if(close(sock_listen) == 0) cout << "sock_listen closed." << endl;
	else perror("close");

	return 0;
}