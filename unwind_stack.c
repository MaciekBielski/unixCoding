/* The program checks whether the same jmp_buf structure may be used for many
 * jumps
 */
 
#include<stdio.h>
#include<setjmp.h>
#include<unistd.h>

static jmp_buf jmpBuf;

static void taskB();
static void taskA();

int main()
{
    taskA();
}

static void taskA()
{
    int a=5;
    while(1)
    {
        printf("--taskA: %d\n", a);
        sleep(1);
        if(setjmp(jmpBuf))
        {
            /* return from taskB */
            puts("\t--return--");
        }
        else
            taskB();    
    }
}

static void taskB()
{
    int a=3;
    printf("--taskB: %d\n", a);
    sleep(1);
    longjmp(jmpBuf,8);
}
