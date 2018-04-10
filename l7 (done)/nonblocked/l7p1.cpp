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
#include <sys/mman.h>
#include <mqueue.h>
#include <sys/stat.h>
#define MAX_BUF 1024
using namespace std;

//program #1
struct thread_params {
    int flag;
    mqd_t mqdes;
    
};

void * func(void * params) {
    cout << "Thread started." << endl << endl;
    thread_params * par = (thread_params*) params;
    
    while(par->flag)
    {
        string message = "Message #" + to_string(rand() %10);
        cout << "Trying to send a message.." << endl;

	int send_ans = mq_send(par->mqdes, message.c_str(), message.length() + 1, 0);
        if(send_ans == -1) 
		if(errno == EAGAIN) cout << "Queue is full." << endl << endl;
		else perror("mq_send");
	else cout << "Sent: " << message << endl << endl;
        
        sleep(1);
    }


    return (void*) new int (0);
}

int main ()
{
	//размер буфера макс 8192
	pthread_t id;
	void * status_exit;
	string queue_name = "/queuelab7";
	cout << "Trying to open (or create) the Message Queue.." << endl;
	struct mq_attr attr;
	attr.mq_msgsize = 100;
	attr.mq_maxmsg = 10;

	mqd_t mqdes = mq_open(queue_name.c_str(), O_CREAT | O_WRONLY | O_NONBLOCK, 0777, &attr);

	if(mqdes < 0) { perror("mq_open"); return 1;}

        cout << "MQ has been opened." << endl;
    
	thread_params params = {1, mqdes};
	pthread_create(&id, 0, func, &params);
	
	getchar();
	
	params.flag = 0;
	cout << "Wait for thread's closing." << endl;
	pthread_join(id, &status_exit);
	
	cout << "Thread exit code: " << *((int *) status_exit) << endl;
	mq_close(mqdes);
	mq_unlink(queue_name.c_str());
	
	return 0;
}
