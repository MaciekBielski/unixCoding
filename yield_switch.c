/* Topic: implementing 'yield' using tricky feature of switch construction,
 * http://www.chiark.greenend.org.uk/~sgtatham/coroutines.html
 * 
 * This code presents how to yield from any function by using 3 simple macros.
 * When the function is invoked again. That solution is easier than keeping
 * an obscure state machine code, especially for a reader. And the more
 * complicated objects we have, the more the advantage of 'yield' is visible.
 * No need to keep the state of all arguments each time.
 * Coding standard is violated but algorithmic structure is very much revealed.
 * The whole trick works thanks to little-know feature of 'switch' construction.
 *
 * Drawback:
 * Solution relies on static variables so it fails to be reentrant or
 * thread-safe.
 * Solution:
 * Additional pointer to a context structure passed each time to crBegin, all
 * locals and static variables are then loaded from that structure.
 *
 * Commented code is the primary version. Working one is reentrant.
 */

#include<stdio.h>

//THIS IS GOING TO BE EXPRESSED BY MACROS
//int foo(void) {
//    static int i, state = 0;
//    switch (state)
//    {
//        case 0: /* start of function */
//            /* i - incremented always except for 1st time */
//            /* except for return for-loop body is executed only for i<6 */
//            for (i; i < 6; i++)
//            {
//               state = 1234; /* so we will come back to "case 1" */
//                printf("\t%d ", i);
//                return i;
//        case 1234:; /* resume control straight after the return */
//            }
//    }
//}


#define crBegin( context ) switch( context->crState ){ case 0:
#define crYield( context ) do{ context->crState=__LINE__; return ( context->val );  \
    case __LINE__:; }while(0)
#define crFinish }

typedef struct
{
    int crState;            //default 0
    int i;                  
    int val;                //default 0
} FooContext;

static int foo(FooContext* context)
{
    crBegin(context);           //this is a macro pointer is passed by name
    for( context->i=0; context->i<6; (context->i)++)
    {
        context->val = context->i * context->i;
        crYield(context);
    }
    crFinish;
}

int main()
{
    FooContext fcOuter = { .crState=0, .val=0 };
    FooContext fcInner = { .crState=0, .val=0 };
    /* let's proof reentrancy */
    for(int k=0; k<3; k++)
    {
        printf("%d\n", foo(&fcOuter));
        for(int l=0; l<2; l++)
            printf("\t%d\n", foo(&fcInner));
    }
    return 0;
}
