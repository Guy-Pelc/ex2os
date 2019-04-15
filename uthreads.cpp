#include <iostream>
#include "thread.h"
#include "uthreads.h"
#include "sys/time.h"
#include "signal.h"
using namespace std;



Thread *threads[MAX_THREAD_NUM] = {0};
int running_tid;

int _quantum_usecs;

int say_hi()
{
	cout<<"hi from uthreads"<<endl;
	return 0;
}




void inc_tid()
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
/** assumes two threads only */
void swap()
{
	cout<<"in swap"<<endl;
	
	
	// running_tid = 1;
	// siglongjmp(threads[1]->env,1);
	
	
	int res = sigsetjmp(threads[running_tid]->env,1);
	if (res == 0)
	{
		inc_tid();
		while (threads[running_tid] == 0)
		{
			inc_tid();
		}

		cout<<"now changing running_tid to "<<running_tid<<endl;
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

int uthread_spawn(void (*f)(void))
{
	cout<<"uthread_spawn"<<endl;
	int tid = get_first_free_tid();
	threads[tid] = new Thread(tid,f);

	cout<<"threads[0,1,2,3] = "<<threads[0]<<","<<threads[1]<<","<<threads[2]<<","<<threads[3]<<endl;

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

