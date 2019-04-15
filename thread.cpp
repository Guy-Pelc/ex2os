#include <iostream>
#include <signal.h>
#include "thread.h"
#include "blackbox.h"

using namespace std;
Thread::~Thread()
{
	cout<<"destructor with tid "<<tid<<endl;
}

Thread::Thread(int tid, void (*f)(void))
{
	status = READY;
	cout<<"hi from thread function constructor"<<endl;
	this->tid = tid;

	//set stack and pc to f
	address_t sp, pc;
	sigsetjmp(env,1);
	sp = (address_t)stack + STACK_SIZE - sizeof(address_t);
	pc = (address_t)f;
	(env->__jmpbuf)[JB_SP] = translate_address(sp);
	(env->__jmpbuf)[JB_PC] = translate_address(pc);
	sigemptyset(&env->__saved_mask);   

}

Thread::Thread(int tid)
{
	status = RUNNING;
	cout<<"hi from thread main constructor"<<endl;
	this->tid = tid;
}

/*
int main()
{
	Thread* t = new Thread(2,3);

	cout<<t->a<<t->b<<endl;
	return 0;
}
*/