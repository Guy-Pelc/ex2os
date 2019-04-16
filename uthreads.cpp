#include <iostream>
#include "thread.h"
#include "uthreads.h"
#include "sys/time.h"
#include "uthreads_helper.h"
#include <signal.h>
#include <deque>
#include "sleeping_threads_list.h"
// #include "s_helper.h"


using namespace std;


//assume pointer to thread with tid=i is at threads[i]
Thread *threads[MAX_THREAD_NUM] = {0};

Thread* running_pthread;
deque<Thread*> ready_pthreads;
SleepingThreadsList sleeping_threads;

int running_tid;

int _quantum_usecs;

int total_quantums;



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
	// cout<<"get_thread_by_id"<<endl;
	for (int i=0;i<MAX_THREAD_NUM;i++)
	{
		if (threads[i] == nullptr) {continue;}
		if (threads[i]->tid == id) {return threads[i];}
	}
	return nullptr;
}


int uthread_block(int tid)
{
	cout<<"uthread_block: "<<tid<<endl;
	Thread *pthread_to_block = get_thread_by_id(tid);
	//check thread with tid exists:
	if (pthread_to_block == nullptr)
	{
		cerr<<"thread library error: blocking thread with tid that does not exist"<<endl;
		return -1;
	}

	if (pthread_to_block->tid == 0)
	{
		cerr<<"thread library error:trying to block main thread"<<endl;
		return -1;
	}

	pthread_to_block->block();

	remove_from_ready_pthreads(tid);

	//blocking itself - should return only after unblocked.
	if (pthread_to_block->tid == running_pthread->tid)
	{
		cout<<"blocking itself"<<endl;
		set_vtimer();
		swap();
	}


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


//release memory and exit.
void exit_program(int exit_code)
{
	//release memory
	for (int i = 0; i<MAX_THREAD_NUM; ++i)
	{
		if (threads[i]!=nullptr)
		{				
			delete threads[i];
		}
	}
	//exit
	exit(exit_code);
}

int uthread_terminate(int tid)
{
	cout<<"uthread_terminate "<<tid<<endl;

	Thread* thread_to_terminate = get_thread_by_id(tid);
	if (thread_to_terminate == nullptr)
	{
		cerr<<"thread library error: trying to terminate a thread with invalid tid"<<endl;
		return -1;
	}

	if (tid == 0)
	{
		exit_program(0);
	}

	remove_from_ready_pthreads(tid);
	//remove from sleeping threads:
	bool is_empty = sleeping_threads.erase(tid);

	//make sure this is necessary, if not- DELETE!
	if (is_empty){stop_s_timer();}



	int this_tid = running_pthread->tid;
	
	delete threads[tid];
	threads[tid] = nullptr;

	//in case thread terminates itself
	if (this_tid == tid) {
		running_pthread = nullptr;
		set_vtimer();
		swap();
	}

	print_threads();	
	return 0;
}


/** pops first ready thread in queue and returns it*/
Thread* get_next_ready_pthread()
{
	// cout<<"get_next_ready_pthread"<<endl;
	fflush(stdout);
	//v2
	Thread* next_pthread = ready_pthreads[0];
	ready_pthreads.pop_front();
	return next_pthread;

}


int uthread_resume(int tid)
{
	cout<<"uthread_resume: "<<tid<<endl;

	Thread* thread_to_resume = get_thread_by_id(tid);
	if (thread_to_resume == nullptr)
	{
		cerr<<"thread library error: trying to resume thread with invalid tid."<<endl;
		return -1;
	}
	//resuming a READY or RUNNING thread has no effect
	if ((thread_to_resume->status == READY) or(thread_to_resume->status == RUNNING)) {return 0;}
	
	thread_to_resume->resume();

	// not trivial, is false for sleeping thread
	if (thread_to_resume->status == READY) {
		ready_pthreads.push_back(thread_to_resume);
	}
	
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
		
		next_pthread->quantums++;
		total_quantums++;
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

/* returns first free id up to MAX_THREAD_NUM-1 , or -1 if there isn't any*/
int get_first_free_tid()
{
	for (int i=0; i<MAX_THREAD_NUM;++i)
	{
		if (threads[i] == 0) {return i;}
	}
	return -1;
}

void print_threads()
{	
	cout<<"thread:status= ";
	for (int i = 0; i<MAX_THREAD_NUM; ++i)
	{

		Thread* cur_thread = threads[i];
		if (threads[i] != NULL)
		{
			cout<<i<<":"<<cur_thread->status<<", ";	
		}
		
		// if (cur_thread != 0)
		// {
		// 	cout<<"  status: "<<cur_thread->status<<endl;
		// }
		// else
		// {
		// 	cout<<endl;
		// }
	}
	cout<<endl;
	cout<<"ready_pthreads:";
	for (unsigned int i = 0; i<ready_pthreads.size();++i)
	{
		cout<<ready_pthreads[i]->tid<<",";
	}
	cout<<endl;
	cout<<"thread:quantums= ";
	for (unsigned int i = 0; i<MAX_THREAD_NUM;++i)
	{	
		if (threads[i] != NULL)
		{
			cout<<i<<":"<<uthread_get_quantums(i)<<", ";
		}
	}
	cout<<endl;
	cout<<"total_quantums: "<<total_quantums<<endl;
	cout<<endl;
		// cout<<"threads[0,1,2,3] = "<<threads[0]<<","<<threads[1]<<","<<threads[2]<<","<<threads[3]<<endl;
		// cout<<"status = "<<threads[0]->status<<","<<threads[1]->status<<","<<threads[2]->status<<","<<threads[3]->status<<endl;
}
int uthread_get_total_quantums() {return total_quantums;}
int uthread_get_quantums(int tid)
{
	return get_thread_by_id(tid)->quantums;
}
int uthread_spawn(void (*f)(void))
{

	cout<<"uthread_spawn"<<endl;
	int tid = get_first_free_tid();
	if (tid == -1)
		{
			cout<<"failiure (not error): cannot exceed MAX_THREAD_NUM"<<endl;
			return -1;
		}
	threads[tid] = new Thread(tid,f);
	if (threads[tid] == 0)
	{
		cerr<<"system error: failed to allocate memory to thread"<<endl;
		return -1;
	}
	ready_pthreads.push_back(threads[tid]);

	print_threads();

	return tid;
}

/** currently supports up to 999,999 microsecs */
int uthread_init(int quantum_usecs)
{
	if (quantum_usecs<=0)
	{
		cerr<<"thread library error: trying to init uthread library with non-positive quantum_usecs value"<<endl;
		return -1;
	}
	cout<<"uthreads_init"<<endl;
	_quantum_usecs = quantum_usecs;

	//init main thread
	threads[0] = new Thread(0);
	if (threads[0] == 0)
	{
		cerr<<"system error: failed to allocate memory to thread"<<endl;
		return -1;
	}
	running_tid = 0;
	running_pthread = threads[0];
	running_pthread->quantums++;
	total_quantums++;

	//set hanlder
	struct sigaction sa = {0};
	sa.sa_handler = &timer_handler;
	sigaction(SIGVTALRM, &sa, NULL);
	
	//set sleep handler
	struct sigaction sa2 = {0};
	sa2.sa_handler = &s_timer_handler;
	sigaction(SIGALRM,&sa2,nullptr);

	set_vtimer();
	
	return 0;
}

/** currently supports only up to 999,999 microsecs*/
void set_vtimer()
{
		//set timer
	itimerval tv = {0};


	tv.it_interval.tv_usec = _quantum_usecs % 1000000;
	tv.it_interval.tv_sec = _quantum_usecs / 1000000;
	tv.it_value.tv_usec = _quantum_usecs % 1000000;
	tv.it_value.tv_sec = _quantum_usecs / 1000000;

	//start timer
	setitimer(ITIMER_VIRTUAL, &tv, NULL);
	return;
}

int uthread_sleep(unsigned int usec)
{
	if (running_pthread->tid == 0)
	{
		cerr<<"thread library error: main can not be blocked"<<endl;
		return -1;
	}


	cout<<endl<<"sleep"<<endl<<endl;
	running_pthread->sleep();
	sleeping_threads.add(running_pthread->tid,calc_wake_up_timeval(usec));
	start_s_timer();


	// print_s_threads();
	return 0;
}	

void s_timer_handler(int signum)
{
	cout<<endl<<"s_timer_handler,"<<endl;
	wake_thread(sleeping_threads.peek()->id);
	sleeping_threads.pop();
	if (sleeping_threads.peek() != nullptr)
	{
		start_s_timer();
	}
}


void wake_thread(int id)
{
	cout<<"wake thread: "<<id<<endl<<endl;
	Thread* pthread_to_block = get_thread_by_id(id);
	// in case thread doesn'pthread_to_block exist - was terminated before wake
	if (pthread_to_block == nullptr) {cout<<"thread doesnt exist, returning"<<endl; return;}

	cout<<"ehre 325"<<endl;
	pthread_to_block->status = READY;
	ready_pthreads.push_back(pthread_to_block);
	return;
}

timeval calc_wake_up_timeval(int usecs_to_sleep) {
	// cout<<"calc_wake_up_timeval"<<usecs_to_sleep<<endl;
	timeval now, time_to_sleep, wake_up_timeval;
	gettimeofday(&now, nullptr);
	time_to_sleep.tv_sec = usecs_to_sleep / 1000000;
	time_to_sleep.tv_usec = usecs_to_sleep % 1000000;
	timeradd(&now, &time_to_sleep, &wake_up_timeval);

	// cout<<endl;
	return wake_up_timeval;
}
void stop_s_timer()
{
	cout<<"stop_s_timer"<<endl;
	itimerval tv = {0};
	setitimer(ITIMER_REAL, &tv,nullptr);
	return;
}


void start_s_timer()
{
	cout<<"start_s_timer"<<endl;

	timeval now, timer_val;
	gettimeofday(&now,nullptr);


	timersub(&(sleeping_threads.peek()->awaken_tv),&now,&timer_val);
	itimerval tv = {0};
	tv.it_value = timer_val;
	
	setitimer(ITIMER_REAL,&tv,nullptr);

}
