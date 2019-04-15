#include <iostream>
#include "thread.h"
#include "uthreads.h"
#include "sys/time.h"
#include "signal.h"
#include "uthread_helper.h"
using namespace std;



Thread *threads[MAX_THREAD_NUM] = {0};
int running_tid;

int _quantum_usecs;

int say_hi()
{
	cout<<"hi from uthreads"<<endl;
	return 0;
}




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
	return running_tid;
}

/** on failiure returns NULL, success returns pointer to thread*/
Thread* get_thread_by_id(int id)
{
	for (int i=0;i<MAX_THREAD_NUM;i++)
	{
		if (threads[i]->tid == id) {return threads[i];}
	}
	return NULL;
}

/* calling block(0) should exit program with error and not return*/
int uthread_block(int tid)
{
	Thread *t = get_thread_by_id(tid);

	if (t->tid == 0)
	{
		cout<<"error, trying to block main thread"<<endl;
		return -1;
	}

	t->status = BLOCKED;




	return 0;
}

/* tested support only terminating from thread 0,
	missing test for terminating itself, including when tid=0*/
int uthread_terminate(int tid)
{
	delete threads[tid];
	threads[tid] = 0;
	
	print_threads();	

	return 0;
}

/* this recursive algorithm will terminate, since main thread cannot be blocked.*/
Thread* get_next_ready_thread()
{
		inc_running_tid();
		while (threads[running_tid] == 0)
		{
			inc_running_tid();
		}

		if (threads[running_tid] -> status != BLOCKED)
		{
			return threads[running_tid]	;
		}

		return get_next_ready_thread();
}
/** currently swaps by incrementing order */
void swap()
{
	cout<<"in swap"<<endl;
	
	
	// running_tid = 1;
	// siglongjmp(threads[1]->env,1);
	// Thread* cur_thread = get_thread_by_id(running_tid);
	threads[running_tid]->status = READY;	

	int res = sigsetjmp(threads[running_tid]->env,1);
	if (res == 0) 
	{
		Thread* next_threadp = get_next_ready_thread();


		cout<<"now changing running_tid to "<<running_tid<<endl;
		
		
		threads[running_tid]->status = RUNNING;	
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
	for (int i = 0; i<4; i++)
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
		// cout<<"threads[0,1,2,3] = "<<threads[0]<<","<<threads[1]<<","<<threads[2]<<","<<threads[3]<<endl;
		// cout<<"status = "<<threads[0]->status<<","<<threads[1]->status<<","<<threads[2]->status<<","<<threads[3]->status<<endl;
}
int uthread_spawn(void (*f)(void))
{
	cout<<"uthread_spawn"<<endl;
	int tid = get_first_free_tid();
	threads[tid] = new Thread(tid,f);

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

	//set hanlder
	struct sigaction sa = {0};
	sa.sa_handler = &timer_handler;
	sigaction(SIGVTALRM, &sa, NULL);

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

