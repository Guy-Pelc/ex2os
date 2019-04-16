#ifndef THREAD_H
#define THREAD_H

#include <setjmp.h>

#define STACK_SIZE 4096

enum Status {READY,BLOCKED,RUNNING};

class Thread
{
private:
	char stack[STACK_SIZE] = {0};
	bool is_blocked = false;
	bool is_sleeping = false;

public:
	bool get_is_blocked() {return is_blocked;}
	bool get_is_sleeping() {return is_sleeping;}
	

	int quantums = 0;
	Status status;
	int tid = 0;
	sigjmp_buf env = {0};
	void resume() {
		is_blocked = false; 
		if (!is_sleeping) {
			status = READY;
		}
		return;
	}
	void block() {is_blocked = true; status = BLOCKED; return;}
	void sleep() {is_sleeping = true; status = BLOCKED;return;}
	void wake() {
		is_sleeping = false;
		if (!is_blocked) {
			status = READY;
		}
		return;
	}
	Thread(int tid);
	Thread(int tid, void (*f)(void));
	~Thread();
};

#endif
