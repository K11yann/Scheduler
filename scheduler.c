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

#define SZ_STACK 4096

struct thread{
    jmp_buf ctx;
    struct {
        /* unsigned gurentee length */
        char *memory; 
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

struct thread *head, *current; /*currently running thread*/
jmp_buf ctx;

/* *head, *thread 是唯一两个global */

/*clean up the linked list */
void destroy(void) {
    struct thread *t, *next;
    t = head;
    do {
        next = t->link;
        free(t->stack.memory_); 
        free(t);
        t = next;
    } while (t != head);
    head = NULL;
    current = NULL;
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
    t->stack.memory_ = malloc(SZ_STACK + PAGE_SIZE);
    if (t -> stack.memory_ == NULL) {
        free(t);
        return -1;
    }
    t -> stack.memory = memory_align(t -> stack.memory_, PAGE_SIZE);
    if (head == NULL) {
        head = t;
        current = t;
        t -> link = t;
    } else {
        t -> link = current -> link;
        current -> link = t;
        current = t;
    }
    return 0;
}

static struct thread *candidate(void) {
    struct thread *t = current -> link;
    while (t != current) {
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
    uint64_t rsp;
    if (t == NULL) {
        return;
    }
    current = t;
    if (current -> status == INIT) {
        rsp = (uint64_t)current->stack.memory + SZ_STACK;
        __asm__ volatile("mov %[rs], %%rsp \n" : [rs] "+r"(rsp)::);
        current -> status = RUNNING;
        current -> code.fnc(current -> code.arg);
        current -> status = TERMINATED;
        longjmp(ctx, 1);
    } else {
        current -> status = RUNNING;
        longjmp(current -> ctx, 1);
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
    if(current && !setjmp(current->ctx)){
        current->status= SLEEPING;
        /* restore */
        longjmp(ctx, 1);

    }
}

