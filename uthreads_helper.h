#ifndef UTHREADS_HELPER_H
#define UTHREADS_HELPER_H

#include "thread.h"

void remove_from_ready_pthreads(int tid);

void inc_tid();

void swap();

void timer_handler();

int get_first_free_tid();

void print_threads();

Thread* get_next_ready_thread();

Thread* get_thread_by_id(int id);

#endif