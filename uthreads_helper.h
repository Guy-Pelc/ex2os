#ifndef UTHREADS_HELPER_H
#define UTHREADS_HELPER_H

#include "thread.h"
void s_timer_handler(int signum);

int calc_wake_up_timeval(int usecs_to_sleep,timeval* wake_up_timeval);

void remove_from_ready_pthreads(int tid);

void inc_tid();

void swap();

void timer_handler();

int get_first_free_tid();

void print_threads();

Thread* get_next_ready_thread();

Thread* get_thread_by_id(int id);
int start_s_timer();

void wake_thread(int id);

int set_vtimer();

int stop_s_timer();

void exit_program();
// void print_s_threads();
#endif