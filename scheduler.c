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

/* gcc -c -S -O3（不link，只编译） ,会生成scheduler.s */

# define SZ_STACK 1048576

struct thread{
    jmp_buf ctx;
    struct {
        /* unsigned gurentee length */
        char * memory; 
        /* actually allocated */
        char *memory_;
    } stack;
    struct {
        void *arg;
        scheduler_fnc_t fnc;
    } code;
    enum {
        INIT,
        RUNNING,
        SLEEPING, 
        TERMINATED
    } status;
    struct thread *link;
};

struct thread *head, *thread; /*currently running thread*/
jmp_buf ctx;

/* *head, *thread 是唯一两个global */

/*clean up the linked list */
void destroy(void) {
    struct thread *t, *t_;
    t = head -> link;
    while (!t) {
        t_ = t;
        free(t_ -> stack.memory_);
        free(t_);
        t = t -> link;
    }
}

/* 初始化 to create a new thread */
int scheduler_create(scheduler_fnc_t fnc, void *arg) {
    size_t PAGE_SIZE = page_size();
    struct thread *t = malloc(sizeof(struct thread));
    /* initialize failed */
    if (!t) {
        TRACE("scheduler_create malloc failed");
        return -1;
    }
    t -> status = INIT;
    t -> code.fnc = fnc;
    t -> code.arg = arg;
    t -> stack.memory_ = malloc(SZ_STACK+ PAGE_SIZE);
    if (t -> stack.memory_ == NULL) {
        free(t);
        return -1;
    }
    t -> stack.memory = memory_align(t -> stack.memory_, PAGE_SIZE);
    t -> link = head;
    head = t;
    return 0;
}

static struct thread * candidate(void) {
    struct thread *t;
    t = head;
    while (t) {
        if (t -> status == INIT || t -> status == SLEEPING) {
            return t;
        }
        t = t -> link;
    }
    /* cannot find a candidate */
    return NULL;
}

void schedule(void) {
    struct thread *t = candidate();
    if (t == NULL) {
        return;
    }
    if (t -> status == INIT) {
        t -> stack.memory_ = (t -> stack.memory) + SZ_STACK;
        __asm__ volatile (
            "mov %0, %%rsp\n"  
            :                   
            : "r"(t -> stack.memory_)               
        );
        t -> status = RUNNING;
        t -> code.fnc(t -> code.arg);
        t -> status = TERMINATED;
        longjmp(ctx, 1);
    } else {
        t -> status = RUNNING;
        longjmp(t -> ctx, 1);
    }
}

/* call by main after all the threads created */
void scheduler_execute(void) {
    setjmp(ctx); /* address of ctx */
    /* go find one thread and run it; if schedule scucess, it will go back setjmp,but */
    schedule(); 
    destroy();
}

/* you r the only one runnung thread */
void scheduler_yield(void) {
    if(thread && !setjmp(thread->ctx)){
        thread->status= SLEEPING;
        /* restore */
        longjmp(ctx, 1);

    }
}

