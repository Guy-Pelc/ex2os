#ifndef UTHREADS_HELPER_H
#define UTHREADS_HELPER_H

#include "thread.h"
void s_timer_handler(int signum);

timeval calc_wake_up_timeval(int usecs_to_sleep);

void remove_from_ready_pthreads(int tid);

void inc_tid();

void swap();

void timer_handler();

int get_first_free_tid();

void print_threads();

Thread* get_next_ready_thread();

Thread* get_thread_by_id(int id);
void start_s_timer();

void wake_thread(int id);

void set_vtimer();

void stop_s_timer();

void exit_program();
// void print_s_threads();
#endif