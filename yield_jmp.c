/* Topic: setjmp() and longjmp() */
#include<stdio.h>
#include<setjmp.h>
#include<unistd.h>

static jmp_buf dispJmp;

static void taskB();
static void taskA();
static void yeldFoo();
void (*tasks[])(void) ={ taskA, taskB };

int main()
{
   yeldFoo(); 
   yeldFoo(); 
   yeldFoo(); 
   yeldFoo(); 
}

static void yeldFoo()
{
    //non-reentrant version
    static int i=0;
    if(setjmp(dispJmp))
        i++;
    else
        tasks[i%2]();
}

static void taskA()
{
    int a=5;
    printf("--taskA: %d\n", a);
    sleep(1);
    longjmp(dispJmp,1);
}

static void taskB()
{
    int a=3;
    printf("--taskB: %d\n", a);
    sleep(1);
    longjmp(dispJmp,1);
}
