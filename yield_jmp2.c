/* Topic: setjmp() and longjmp()
 *
 * inspired by: https://en.wikipedia.org/wiki/Setjmp.h
 * nice example of taking turns cooperative tasks
 */

#include<stdio.h>
#include<setjmp.h>
#include<unistd.h>

static jmp_buf callerJmp;
static jmp_buf calleeJmp;

static void yieldFoo();
static void yieldCaller();
static void yieldCallee();

int main()
{
    yieldFoo();
    yieldFoo();
    yieldFoo();
    yieldFoo();
    yieldFoo();
}

static void yieldFoo()
{
    /* overestimation space needed for main to run */
    char nothing[1000];
    nothing[999] =1;    //preventing optimization
    yieldCaller();
}

static void yieldCaller()
{
    if(!setjmp(callerJmp))
        yieldCallee(); //never returns from here
    for(;;)
    {
        puts("==yielded==");
        if(!setjmp(callerJmp))
            longjmp(calleeJmp,3);
        sleep(1);
    }
}

static void yieldCallee()
{
    for(;;)
    {
        puts("\t--callee FH--");
        if(!setjmp(calleeJmp))
            longjmp(callerJmp,5);
        puts("\t--callee BH--");
        if(!setjmp(calleeJmp))
            longjmp(callerJmp,5);
    }
}
