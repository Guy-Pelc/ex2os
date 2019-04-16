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
			cout<<"f loop";
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

void h()
{
	cout<<"h blocking itself"<<endl;
	// uthread_sleep(3*pow(10,6));
	while(1);
}
int main()
{
	cout<<"hello world"<<endl;
	uthread_init(999999);

	uthread_spawn(&h);

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
}
int main2()
{
	cout<<"hello world"<<endl;
	uthread_init(999999);

	uthread_spawn(&f);
	uthread_spawn(&g);
	uthread_spawn(&f);

	for (int i;i<pow(10,7);++i);

	// uthread_terminate(1);
	uthread_block(1);
	for (int i;i<pow(10,7);++i);

	uthread_resume(1);

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

	return 0;
}
int main1()
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