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
#include <sys/stat.h>
#define MAX_BUF 1024
using namespace std;

//program #2
struct thread_params {
    int flag, fd;
    
};

void * func(void * params) {
    thread_params * par = (thread_params*) params;
    
    while(par->flag)
    {
	char buf[1024];
	int readcount = read(par->fd, (void*) buf, 1024);
        if(readcount < 0) { perror("read"); break; }
	else if (readcount == 0) continue;
	else cout << "Read: " << buf << endl;
        sleep(1);
    }


    return (void*) new int (0);
}

int main ()
{
	pthread_t id;
	void * status_exit;
	string fifo_name = "/tmp/l6pipe";
	int fd;
	
	if(mkfifo(fifo_name.c_str(), 0777) < 0) { perror("mkfifo");  }
	if( (fd = open(fifo_name.c_str(), O_RDONLY)) <= 0) { perror("open fifo"); return 1; }
	


	thread_params params = {1, fd};
	pthread_create(&id, 0, func, &params);
	
	getchar();
	
	params.flag = 0;
	
	pthread_join(id, &status_exit);
	
	cout << "Thread exit code: " << *((int *) status_exit) << endl;
	close(fd);
	unlink(fifo_name.c_str());
	
	return 0;
}
