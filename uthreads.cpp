#include <iostream>
#include "thread.h"
#include "uthreads.h"
#include "sys/time.h"
#include "uthreads_helper.h"
#include <signal.h>
#include <deque>
#include "sleeping_threads_list.h"
#include "s_helper.h"


using namespace std;


//assume pointer to thread with tid=i is at threads[i]
Thread *threads[MAX_THREAD_NUM] = {0};

Thread* running_pthread;
deque<Thread*> ready_pthreads;

int running_tid;

int _quantum_usecs;



void inc_running_tid()
{
	running_tid++;
	if (running_tid == MAX_THREAD_NUM)
	{
		running_tid = 0;
	}
	return;
}

int uthread_get_tid()
{
	return running_pthread->tid;
}

/** on failiure returns NULL, success returns pointer to thread*/
Thread* get_thread_by_id(int id)
{
	for (int i=0;i<MAX_THREAD_NUM;i++)
	{
		if (threads[i]->tid == id) {return threads[i];}
	}
	return nullptr;
}

/* calling block(0) should exit program with error and not return
** missing implementation for thread blocking itself */
int uthread_block(int tid)
{
	cout<<"uthread_block: "<<tid<<endl;
	Thread *t = get_thread_by_id(tid);

	if (t->tid == 0)
	{
		cout<<"error, trying to block main thread"<<endl;
		return -1;
	}

	t->status = BLOCKED;
	remove_from_ready_pthreads(tid);


	return 0;
}

//remove from ready queue (might not be there if it is running/blocked)
void remove_from_ready_pthreads(int tid)
{
	for (unsigned int i=0;i<ready_pthreads.size();++i)
	{
		if (ready_pthreads[i]->tid == tid)
		{
			ready_pthreads.erase(ready_pthreads.begin() + i);
		}
	}
	return;
}

/* missing implementation for terminating tid=0, by tid=0 or by any other thread*/
int uthread_terminate(int tid)
{
	cout<<"uthread_terminate"<<endl;
	// for (int i=0;i<ready_pthreads)
	remove_from_ready_pthreads(tid);


	//in case thread terminates itself
	if (tid == running_pthread->tid)
	{
		running_pthread = nullptr;
	}

	delete threads[tid];
	threads[tid] = nullptr;

 
	
	print_threads();	

	return 0;
}

/** pops first ready thread in queue and returns it*/
Thread* get_next_ready_pthread()
{
	cout<<"get_next_ready_pthread"<<endl;
	fflush(stdout);
	//v2
	Thread* next_pthread = ready_pthreads[0];
	ready_pthreads.pop_front();
	return next_pthread;

	//nonfair version:
	/*
		inc_running_tid();
		while (threads[running_tid] == 0)
		{
			inc_running_tid();
		}

		if (threads[running_tid] -> status != BLOCKED)
		{
			return threads[running_tid]	;
		}

		return get_next_ready_pthread();
		*/
}


int uthread_resume(int tid)
{
	cout<<"uthread_resume: "<<tid<<endl;
	ready_pthreads.push_back(threads[tid]);
	return 0;
}

void swap()
{
	cout<<"in swap"<<endl;
	int res;
	// if running thread terminates itself, save nothing
	if (running_pthread == nullptr) {res = 0;}
	// if running thread blocks itself, save state
	else if (running_pthread->status == BLOCKED)
		res = sigsetjmp(running_pthread->env,1);
	// save state and push to end of ready line
	else
	{
		running_pthread->status = READY;	
		ready_pthreads.push_back(running_pthread);
		res = sigsetjmp(running_pthread->env,1);
	}


	
	if (res == 0) 
	{
		Thread* next_pthread = get_next_ready_pthread();

		cout<<"now changing running_tid to "<<next_pthread->tid<<endl;
		
		
		next_pthread->status = RUNNING;
		running_pthread = next_pthread;
		running_tid = next_pthread->tid;

		print_threads();
		siglongjmp(threads[running_tid]->env,threads[running_tid]->tid);
	}
	return;
	
}

void timer_handler(int signum)
{
	cout<<"timer_handler"<<endl;
	swap();
	
	return;
}

int get_first_free_tid()
{
	int i = 0;
	while (threads[i]!=0)
	{
		i++;
	}
	return i;
}

void print_threads()
{
	for (int i = 0; i<2; ++i)
	{
		Thread* cur_thread = threads[i];
		cout<<"threads["<<i<<"]="<<cur_thread;
		if (cur_thread != 0)
		{
			cout<<"  status: "<<cur_thread->status<<endl;
		}
		else
		{
			cout<<endl;
		}
	}
	cout<<"ready_pthreads:";
	for (unsigned int i = 0; i<ready_pthreads.size();++i)
	{
		cout<<ready_pthreads[i]->tid<<",";
	}
	cout<<endl;
		// cout<<"threads[0,1,2,3] = "<<threads[0]<<","<<threads[1]<<","<<threads[2]<<","<<threads[3]<<endl;
		// cout<<"status = "<<threads[0]->status<<","<<threads[1]->status<<","<<threads[2]->status<<","<<threads[3]->status<<endl;
}
int uthread_spawn(void (*f)(void))
{
	cout<<"uthread_spawn"<<endl;
	int tid = get_first_free_tid();
	threads[tid] = new Thread(tid,f);
	ready_pthreads.push_back(threads[tid]);

	print_threads();

	return 0;
}

/** currently supports up to 999,999 microsecs */
int uthread_init(int quantum_usecs)
{
	cout<<"uthreads_init"<<endl;
	_quantum_usecs = quantum_usecs;
	//init main thread
	threads[0] = new Thread(0);
	running_tid = 0;
	running_pthread = threads[0];

	//set hanlder
	struct sigaction sa = {0};
	sa.sa_handler = &timer_handler;
	sigaction(SIGVTALRM, &sa, NULL);
	
	//set sleep handler
	struct sigaction sa2 = {0};
	sa2.sa_handler = &s_timer_handler;
	sigaction(SIGALRM,&sa2,nullptr);

	//set timer
	itimerval tv = {0};

	//TODO CORRECT THIS FOR ABOVE 1M MICROSECS

	// tv.it_interval.tv_sec = 1;
	// tv.it_value.tv_sec = 1;

	tv.it_interval.tv_usec = quantum_usecs;
	tv.it_value.tv_usec = quantum_usecs;

	//start timer
	setitimer(ITIMER_VIRTUAL, &tv, NULL);
	
	return 0;
}

