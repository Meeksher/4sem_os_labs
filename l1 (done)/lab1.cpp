#include <pthread.h>
#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
using namespace std;

struct thread_args
{
	int num;
	int flag;
};

void* func(void * arg)
{
	thread_args *args = (thread_args*) arg;

	while(args->flag)
	{
		cout << args->num << flush;
		sleep(1);
	}
	
	return (void*) new int(0);
}



int main()
{
	thread_args args1 = {1, 1}, args2 = {2, 1};
	pthread_t id1, id2;
	void * cancel_status;

	pthread_create(&id1,0,func,&args1);
	pthread_create(&id2,0,func,&args2);
	getchar();
	args1.flag = args2.flag = 0;
	
	pthread_join(id1, &cancel_status);
	cout << "Thread 1 cancelled with: " << *( (int *) cancel_status) << endl;
	pthread_join(id2, &cancel_status);
	cout << "Thread 2 cancelled with: " << *( (int *) cancel_status) << endl;
	
	return 0;
}
