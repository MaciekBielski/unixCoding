/* Topic: context allocation and switching
 *
 * This program presents how to change context to the new allocated one. It is
 * more than just setjmp/longjmp - those two are for stack unwinding, whereas
 * their jumping points need to remain valid, which means their stack cannot be
 * invalidated */

#include <stdio.h>
#include <string.h>
#include <ucontext.h>
#include <unistd.h>

static ucontext_t ctxA, ctxB, tmp;

static void fiberA();
static void fiberB();

int main()
{
    char stackA[24];
    char stackB[24];

    getcontext( &tmp);          // the only one that is not configured
    getcontext( &ctxA );
    ctxA.uc_stack.ss_sp = stackA;
    ctxA.uc_stack.ss_size = sizeof(stackA);
    ctxA.uc_link = &ctxB;
    makecontext( &ctxA, fiberA, 0);
    //
    getcontext( &ctxB );
    ctxB.uc_stack.ss_sp = stackB;
    ctxB.uc_stack.ss_size = sizeof(stackB);
    ctxB.uc_link = &tmp;
    makecontext( &ctxB, fiberB, 0);
    setcontext(&ctxA);
    puts("--bye--");
    return 0;
}

static void fiberA()
{
    char fakeData[8];
    fakeData[7]='1';    
    puts("\t--fiberA--");
    sleep(1);
}

static void fiberB()
{
    char fakeData[8];
    fakeData[7]='1';    //prevent optimization
    puts("\t--fiberB--");
    sleep(1);
}
