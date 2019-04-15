#include "uthreads.h"
#include "thread.h"
#include <iostream>
#include <math.h>

using namespace std;

void f()
{
	int i = 0;
	while(1)
	{
		i++;
		if(i==pow(10,7))
		{
			i = 0;
			cout<<"f loop"<<endl;
			cout<<"tid = "<<uthread_get_tid()<<endl;
		}
	}
	return;
}

void g()
{
		int i = 0;
	while(1)
	{
		i++;
		if(i==pow(10,7))
		{
			i = 0;
			cout<<"g loop"<<endl;
			cout<<"tid = "<<uthread_get_tid()<<endl;

		}
	}
	return;
}
int main()
{
	cout<<"main_uthreads"<<endl;
	uthread_init(999999);


	uthread_spawn(&f);
	uthread_spawn(&g);
	uthread_spawn(&f);

	for(int j=0;j<2.5*pow(10,7);j++);
	uthread_terminate(1);
	uthread_spawn(&g);

	uthread_block(3);

	int i = 0;
	while(1)
	{
		i++;
		if(i==pow(10,7))
		{
			i = 0;
			cout<<"main loop"<<endl;
		}
	}
	cout<<"end_main"<<endl;
	return 0;
}