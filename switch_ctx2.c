/* Topic: context allocation and switching 2
 *
 * 1) what happens when you returns from swapcontext?
 * if uc_link is not null then in will return to the place swapcontext,
 * uc_link keeps only an address of something that is not defined yet, but will
 * be later
 */

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

    getcontext( &ctxA );
    ctxA.uc_stack.ss_sp = stackA;
    ctxA.uc_stack.ss_size = sizeof(stackA);
    ctxA.uc_link = &ctxB;           
    makecontext( &ctxA, fiberA, 0);
    //
    swapcontext(&ctxB, &ctxA);
    puts("--returned to ctxB--");
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
