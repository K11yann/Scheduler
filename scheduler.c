/**
 * Tony Givargis
 * Copyright (C), 2023
 * University of California, Irvine
 *
 * CS 238P - Operating Systems
 * scheduler.c
 */

#undef _FORTIFY_SOURCE

#include <unistd.h>
#include <signal.h>
#include <setjmp.h>
#include "system.h"
#include "scheduler.h"

/**
 * Needs:
 *   setjmp()
 *   longjmp()
 */

/* research the above Needed API and design accordingly */

// gcc -c -S -O3（不link，只编译） ,会生成scheduler.s

# define S2 STACK 1048576

struct {

}

// *head, *thread 是唯一两个global

//clean up the linked list
static void destroy(void) {
    struct thread *t, t_;
    t = head;
    while (t) {
        t_ = t;
        t = t -> link;
        free(t_ -> stack.memory_);
        free(t_);
    }
}

// 初始化 to create a new thread
int schedule_create(fnc, arg) {
    t = malloc.....
    t -> status = INIT;
    t -> code.fnc = fnc;
    t -> code.arg = arg;
    // p is pagesize
    t -> stack.memory = malloc(st_STACK+p);
    t -> stack.memory = memory_align(t -> stack.memory, p);
    t -> link = head;
    head = t;
}

static struct thread * candidate(void) {
    // INIT和SPEEPING 都可以是candidate

    // traverse our list of threads
    //  if t.status is (INIT or SLEEPING)
    //  return id
    //  NULL no terminate is a candidate
}

// call by main after all the threads created
void scheduler_execute(void) {
    setjmp(&ctx); // address of ctx
    schedule(); // go find one thread and run it; if schedule scucess, it will go back setjmp,but 
    destroy();
}

// you r the only one runnung thread
void scheduler_yield(void) {
    if(!setjmp(thread->ctx)){
        thread->status= SLEEPING;
        longjmp(ctx, 1);

    }
}