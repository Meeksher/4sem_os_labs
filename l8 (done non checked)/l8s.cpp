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
#include <list>
#define MAX_BUF 1024
using namespace std;


int flag_rcv, flag_perform, flag_response, flag_waitconnect;
int sock_listen;
pthread_t id_waitconnect; 

struct thread_params {
	int socket;
	queue<char*> * q_perform, * q_response;
};

struct socket_threads {
	pthread_t id_recieve, id_perform, id_response;
	thread_params * params;
};

std::list<socket_threads> threads_l;

void * rcv_func(void * params)
{
	thread_params * par = (thread_params*) params;

	while(flag_rcv)
	{
		char buf[MAX_BUF];


		int rcv = recv(par->socket, buf, MAX_BUF, MSG_NOSIGNAL);

		if(errno == EPIPE) { return (void*) new int (0); } 

		if(rcv > 0)
		{		
			cout << "Recieved: " << buf << ", " << rcv << " bytes." << endl;
			par->q_perform->push(buf);
		} 
		else sleep(1);
	}


    return (void*) new int (0);
}

void * perform_func(void * params)
{
	thread_params * par = (thread_params*) params;

	while(flag_perform)
	{
		if(par->q_perform->size() == 0){
			sleep(1);
			continue;
		}

		if(errno == EPIPE) { return (void*) new int (0); } 

		char * msg = par->q_perform->front();
		par->q_perform->pop();

		// gen answer
		msg[1] = '1';
		msg[2] = '2';
		par->q_response->push(msg);
	}


    return (void*) new int (0);
}

void * response_func(void * params)
{
	thread_params * par = (thread_params*) params;

	while(flag_response)
	{
		if(par->q_response->size() == 0){
			sleep(1);
			continue;
		}

		char * msg = par->q_response->front();
		par->q_response->pop();

		if(send(par->socket, msg, strlen(msg), MSG_NOSIGNAL) > 0) cout << "Response \"" << msg << "\" sent." << endl;
		else perror("send");

		if(errno == EPIPE) { cout << "One of connections has lost." << endl;  return (void*) new int (0); } 
	}
}

void * listening_func(void * params) {
	while(flag_waitconnect)
	{
		sleep(1);
		int sock_client = accept(sock_listen, 0, 0);
		if(sock_client > 0) cout << "Connected new device." << endl;
		else continue;


		queue<char*> * q_perform = new queue<char*>(), * q_response = new queue<char*>();

		thread_params * new_params =  new thread_params {sock_client, q_perform, q_response};

		socket_threads threads;
		threads.params = new_params;

		if(pthread_create(&threads.id_recieve, 0, rcv_func, new_params) == 0) cout << "Thread Recieving started." << endl;
		else perror("pthread_create");
		if(pthread_create(&threads.id_perform, 0, perform_func, new_params) == 0) cout << "Thread Performing started." << endl;
		else perror("pthread_create");
		if(pthread_create(&threads.id_response, 0, response_func, new_params) == 0) cout << "Thread Responsing started." << endl;
		else perror("pthread_create");

		threads_l.push_back(threads);
	}

    return (void*) new int (0);
}

int main()
{
	const char name_sock_l[] = "/tmp/socknamel8listener";
	unlink(name_sock_l);


	flag_rcv = flag_perform = flag_response = flag_waitconnect = 1;

	sock_listen = socket(AF_UNIX, SOCK_NONBLOCK | SOCK_STREAM, 0);

	if(sock_listen > 0) cout << "sock_listen initiliazied." << endl;
	else perror("socket");

    struct sockaddr_un sun;
    memset(&sun,0,sizeof sun);

    sun.sun_family = AF_UNIX;

    strncpy(sun.sun_path, name_sock_l, sizeof(sun.sun_path) - 1);

    if( bind(sock_listen, (struct sockaddr *)&sun, sizeof(sun)) == 0) cout << "sock_listen binded." << endl;
    else perror("bind");

    if( listen(sock_listen, 2) == 0) cout << "sock_listen in listening mode." << endl;
    else perror("listen");


	if(pthread_create(&id_waitconnect, 0, listening_func, 0) == 0) cout << "Thread Listening started." << endl;
	else perror("pthread_create");


	getchar();

	flag_rcv = flag_perform = flag_response = flag_waitconnect = 0;

	for ( socket_threads thread : threads_l)
	{
		if(close(thread.params->socket) == 0) cout << "regular sock_client closed." << endl;
		else perror("close");

		pthread_join(thread.id_recieve, 0);
		pthread_join(thread.id_perform, 0);
		pthread_join(thread.id_response, 0);
	}
	
	pthread_join(id_waitconnect, 0);

	cout << "Joined all." << endl;

	if(close(sock_listen) == 0) cout << "sock_listen closed." << endl;
	else perror("close");

	unlink(name_sock_l);

	return 0;
}