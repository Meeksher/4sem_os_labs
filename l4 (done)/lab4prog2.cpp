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
#define MAX_BUF 1024
using namespace std;

//program #2
//output: *
struct thread_params {
    int flag;
    std::ofstream * outfile;
    sem_t * semaphore;
};

void * func(void * params) {
    thread_params * par = (thread_params*) params;
    
    char ch = '*';
    while(par->flag)
    {
        cout << "sem_wait_start" << endl;
        sem_wait(par->semaphore);
        cout << "sem_wait_end" << endl;

        for(int i = 0; i < 3; i++)
        {
            *(par->outfile) << ch;
	    (par->outfile)->flush();
            sleep(1);
        }
        sem_post(par->semaphore);
	cout << "sem_posted" << endl << endl;
        sleep(2);
    }


    return (void*) new int (0);
}

int main ()
{
	pthread_t id;
	void * status_exit;
	sem_t * semaphore;
	string sem_name =  "/lab4_sem";
	std::ofstream outfile("file.txt", fstream::app);

	if((semaphore = sem_open(sem_name.c_str(), O_CREAT, 0777, 1)) == SEM_FAILED)
	{
	    perror("sem_open");
	    return 1;
	}
	
	cout << "[prog2] Semaphore taken."  << semaphore << endl;
	
	thread_params params1 = {1, &outfile, semaphore};
	pthread_create(&id, 0, func, &params1);
	
	getchar();
	
	params1.flag = 0;
	
	pthread_join(id, &status_exit);
	
	cout << "[prog2] Thread 1 exit code: " << *((int *) status_exit) << endl;
	outfile.close();
	sem_close(semaphore);
	sem_unlink(sem_name.c_str()); //он удаляется сам по себе, если все процессы закрыли его
	
	return 0;
}
