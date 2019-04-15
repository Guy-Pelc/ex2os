#include <setjmp.h>

#define STACK_SIZE 4096

class Thread
{
private:
	char stack[STACK_SIZE] = {0};
	
	
public:
	int tid = 0;
	sigjmp_buf env = {0};
	Thread(int tid);
	Thread(int tid, void (*f)(void));
};
