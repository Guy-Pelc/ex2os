#ifndef THREAD_H
#define THREAD_H

#include <setjmp.h>

#define STACK_SIZE 4096

enum Status {READY,BLOCKED,RUNNING};

class Thread
{
private:
	char stack[STACK_SIZE] = {0};
	
	
public:
	Status status;
	int tid = 0;
	sigjmp_buf env = {0};
	Thread(int tid);
	Thread(int tid, void (*f)(void));
	~Thread();
};

#endif
