#include <stdio.h>
#include <iostream>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>
#include <string>
#include <fcntl.h>
#define MAX_BUF 1024
using namespace std;
/*
pipe - пуст.
Т.е. write завершился аварийно.
Тогда read() бесконечен.

Решение - флаг O_NONBLOCK.
И читать про pipe2. Его нет на макос. Есть другая - fcntl().
*/

struct thread_params {
    int flag;
    int * filedes;
};

void * func1(void * params) {
    thread_params * par = (thread_params*) params;
    
    while(par->flag)
    {
        string message = "mes id " + to_string(rand() %10);
        //write(par->filedes[1], message.c_str(), message.length());
        sleep(2);
    }

    cout << "Thread-write closed." << endl;    

    return (void*) new int (0);
}

void * func2(void * params) {
    thread_params * par = (thread_params*) params;
    int flags = fcntl(par->filedes[0], F_GETFL);
    flags |= O_NONBLOCK;
    fcntl(par->filedes[0], F_SETFL, flags);


    char buf[MAX_BUF];
    while(par->flag)
    {
        int r_c = read(par->filedes[0], buf, MAX_BUF -1);
        if(r_c > 0)
        {
            buf[MAX_BUF] = '\0';
            cout << buf << endl;
        }
        sleep(1);
    }
    
    cout << "Thread-read closed." << endl;
    
    return (void*) new int (0);
}

int main ()
{
	pthread_t id1, id2;
	void * status_exit;
	int filedes[2];
	pipe(filedes);
	
	thread_params params1 = {1, filedes}, params2 = {1, filedes};
	
	pthread_create(&id1, 0, func1, &params1);
	pthread_create(&id2, 0, func2, &params2);
	getchar();
	params1.flag = params2.flag = 0;
	
	pthread_join(id1, &status_exit);
	cout << "Thread 1 exit code: " << *((int *) status_exit) << endl;
	pthread_join(id2, &status_exit);
	cout << "Thread 2 exit code: " << *((int *) status_exit) << endl;
	close(filedes[0]);close(filedes[1]);
	
	return 0;
}
