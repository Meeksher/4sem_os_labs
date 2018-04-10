#include <stdio.h>
#include <iostream>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include <string>
using namespace std;


struct thread_params{
	int flag;
	int symb;
	pthread_mutex_t * omutex;
	int id;
};

void * func(void * params)
{
	thread_params * par = (thread_params*) params;
	struct timespec deltatime;
	
	string temp;
	
	while(par->flag)
	{
		int mutex_code;
		do{
            clock_gettime(CLOCK_REALTIME, &deltatime);
            time_t delta = deltatime.tv_sec;

            deltatime.tv_sec += 3;
			mutex_code = pthread_mutex_timedlock(par->omutex, &deltatime);
            
            clock_gettime(CLOCK_REALTIME, &deltatime);

            temp = "delta is: " + to_string(deltatime.tv_sec - delta) + "\n";
            cout << temp;

			if(!par->flag) goto exit;
			
		} while(mutex_code);
		
		for(int i = 0; i < 3; i++)
		{
			cout << par->symb << flush;
			sleep(1);
		}
		pthread_mutex_unlock(par->omutex);
		sleep(2);
	}
	
	exit: 
	temp = "Thread " + to_string(par->id) + " exit.\n";
	cout << temp;
	return (void*) new int (0);
}

int main ()
{
	pthread_t id1, id2;
	void * status_exit;
	pthread_mutex_t mutex;
	pthread_mutex_init(&mutex, 0);
	
    pthread_mutex_lock(&mutex);

	thread_params params1 = {1, 1, &mutex, 0}, params2 = {1, 2, &mutex, 1};

	pthread_create(&id1, 0, func, &params1);
	pthread_create(&id2, 0, func, &params2); 
	getchar();
	params1.flag = params2.flag = 0;
	
	pthread_join(id1, &status_exit);
	cout << "Thread 1 exit code: " << *((int *) status_exit) << endl;
	pthread_join(id2, &status_exit);
	cout << "Thread 2 exit code: " << *((int *) status_exit) << endl;
	
	pthread_mutex_destroy(&mutex);
	
	return 0;
}
