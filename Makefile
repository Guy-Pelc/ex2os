all:
	g++ -c 				 thread.cpp		uthreads.cpp sleeping_threads_list.cpp -std=c++11 -Wall 
	ar rvs libuthreads.a thread.o	 	uthreads.o	 sleeping_threads_list.o

tar:
	tar -cvf ex2.tar blackbox.h Makefile README sleeping_threads_list.cpp sleeping_threads_list.h thread.cpp thread.h uthreads.cpp uthreads_helper.h
