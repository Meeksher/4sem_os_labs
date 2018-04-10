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

//сделать синхронизацию "пишет - ждет" без потери данных; нач значения семов = 0
//program #1
struct thread_params {
    int flag;
    sem_t * sem_read, *sem_write;
    int *mem_addr;
};

void * func(void * params) {
    thread_params * par = (thread_params*) params;
    
    while(par->flag)
    {
        sem_wait(par->sem_write);

        int val = rand() % 100;
        cout << "Generated value: " << val << endl;

        *par->mem_addr = val;

        sem_post(par->sem_read);
        
	    
        sleep(2);
    }

exit:

    return (void*) new int (0);
}

int main ()
{
	pthread_t id;
	void * status_exit;
	int  *mem_addr;
	sem_t * sem_read, *sem_write;
	int fd, memory_size = sizeof(int);
	string sem_read_name =  "/l5_read", sem_write_name = "/l5_write";
	string memory_name = "lab5";
	

	if((sem_read = sem_open(sem_read_name.c_str(), O_CREAT, 0777, 0)) == SEM_FAILED)
	{
	    perror("sem_read"); return 1;
	}
	if((sem_write = sem_open(sem_write_name.c_str(), O_CREAT, 0777, 0)) == SEM_FAILED)
	{
	    perror("sem_write"); return 1;
	}
	
	if( (fd = shm_open(memory_name.c_str(), O_CREAT|O_RDWR, 0777)) == -1)
	{
	    perror("shm_open");
	    return 1;
	}
	
	if( ftruncate(fd, memory_size) == -1)
	{
	    perror("ftruncate");
	    return 1;
	}
	
	mem_addr = (int*) mmap(0, memory_size, PROT_WRITE|PROT_READ, MAP_SHARED, fd, 0);
	
	if( mem_addr == (int*) -1)
	{
	    perror("mmap");
	    return 1;
	}
	// memory allocated and ready for using

	thread_params params = {1, sem_read, sem_write, mem_addr};
	pthread_create(&id, 0, func, &params);
	
	getchar();
	
	params.flag = 0;
	
	pthread_join(id, &status_exit);
	
	cout << "Thread exit code: " << *((int *) status_exit) << endl;
	
	sem_close(sem_read);
	sem_unlink(sem_read_name.c_str());
	sem_close(sem_write);
	sem_unlink(sem_write_name.c_str());
	
	munmap(mem_addr, memory_size);
	close(fd);
	shm_unlink(memory_name.c_str());
	
	return 0;
}
