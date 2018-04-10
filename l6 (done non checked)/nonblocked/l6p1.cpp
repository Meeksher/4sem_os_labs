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
#define MAX_BUF 1024
using namespace std;

//program #1
struct thread_params {
    int flag, fd;
    string fifo_name;
    
};

void * func(void * params) {
    thread_params * par = (thread_params*) params;
    
    while(par->flag)
    {
        string message = "Message #" + to_string(rand() %10);
        
        struct stat buf;
        int file_stat = stat(par->fifo_name.c_str(), &buf);

        if(file_stat < 0) { cout << "Pipe closed." << endl; break; }


        write(par->fd, message.c_str(), message.length());
        cout << "Wrote: " << message << endl;
        
        sleep(1);
    }

    return (void*) new int (0);
}

int main () 
{
	//не блокирующий open; в случае если закрылась одна - другая НЕ падает
	pthread_t id;
	void * status_exit;
	string fifo_name = "/tmp/l6pipe";
	
	if(mkfifo(fifo_name.c_str(), 0777) < 0) { perror("mkfifo");  }
	
    int fd = 0;

	while(fd <= 0)
    {
        cout << "Trying to sync.." << endl;
        fd = open(fifo_name.c_str(), O_WRONLY | O_NONBLOCK);
        if(fd <= 0) perror("open");

        sleep(1);
    }

    cout << "Connected." << endl;
    

	thread_params params = {1, fd, fifo_name};
	pthread_create(&id, 0, func, &params);
	
	getchar();
	
	params.flag = 0;
	
	pthread_join(id, &status_exit);
	
	cout << "Thread exit code: " << *((int *) status_exit) << endl;
	close(fd);
	unlink(fifo_name.c_str());
	
	return 0;
}
