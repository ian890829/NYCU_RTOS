/*
*********************************************************************************************************
*                                                uC/OS-II
*                                          The Real-Time Kernel
*
*                           (c) Copyright 1992-2002, Jean J. Labrosse, Weston, FL
*                                           All Rights Reserved
*
*                                               EXAMPLE #1
*********************************************************************************************************
*/

#include "includes.h"

/*
*********************************************************************************************************
*                                               CONSTANTS
*********************************************************************************************************
*/

#define  TASK_STK_SIZE                 512       /* Size of each task's stacks (# of WORDs)            */
#define  N_TASKS                        3      /* Number of identical tasks                          */

typedef struct
{
    int compTime;
    int period;
}TaskInfo;



/*
*********************************************************************************************************
*                                               VARIABLES
*********************************************************************************************************
*/

OS_STK        TaskStk[N_TASKS][TASK_STK_SIZE];        /* Tasks stacks                                  */
OS_STK        TaskStartStk[TASK_STK_SIZE];
//char          TaskData[N_TASKS];                      /* Parameters to pass to each task               */
OS_EVENT     *RandomSem;
TaskInfo info[3]={{1,3},{3,6},{4,9}};
int line=1;
char CtxSwMsg[20][50];
int CtxSwMsgCnt = 0;

/*
*********************************************************************************************************
*                                           FUNCTION PROTOTYPES
*********************************************************************************************************
*/

        void  Task(void *data);                       /* Function prototypes of tasks                  */
        void  TaskStart(void *data);                  /* Function prototypes of Startup task           */
static  void  TaskStartCreateTasks(void);
static  void  TaskStartDispInit(void);
static  void  TaskStartDisp(void);
        void  printMessage();

/*$PAGE*/
/*
*********************************************************************************************************
*                                                MAIN
*********************************************************************************************************
*/

void  main (void)
{
    PC_DispClrScr(DISP_FGND_WHITE + DISP_BGND_BLACK);      /* Clear the screen                         */

    OSInit();                                              /* Initialize uC/OS-II                      */

    PC_DOSSaveReturn();                                    /* Save environment to return to DOS        */
    PC_VectSet(uCOS, OSCtxSw);                             /* Install uC/OS-II's context switch vector */
    PC_ElapsedInit();
    RandomSem   = OSSemCreate(1);                          /* Random number semaphore                  */

    OSTaskCreate(TaskStart, (void *)0, &TaskStartStk[TASK_STK_SIZE - 1], 0);
    OSStart();                                             /* Start multitasking                       */
}


/*
*********************************************************************************************************
*                                              STARTUP TASK
*********************************************************************************************************
*/
void  TaskStart (void *pdata)
{
#if OS_CRITICAL_METHOD == 3                                /* Allocate storage for CPU status register */
    OS_CPU_SR  cpu_sr;
#endif
    char       s[100];
    INT16S     key;


    pdata = pdata;                                         /* Prevent compiler warning                 */

    //TaskStartDispInit();                                   /* Initialize the display                   */

    OS_ENTER_CRITICAL();
    PC_VectSet(0x08, OSTickISR);                           /* Install uC/OS-II's clock tick ISR        */
    PC_SetTickRate(OS_TICKS_PER_SEC);                      /* Reprogram tick rate                      */
    OS_EXIT_CRITICAL();

    //OSStatInit();                                          /* Initialize uC/OS-II's statistics         */
    OSTimeSet(0);
    TaskStartCreateTasks();                                /* Create all the application tasks         */

    for (;;) {
                                
       printMessage();

        if (PC_GetKey(&key) == TRUE) {                     /* See if key has been pressed              */
            if (key == 0x1B) {                             /* Yes, see if it's the ESCAPE key          */
                PC_DOSReturn();                            /* Return to DOS                            */
            }
        }

        OSCtxSwCtr = 0;                                    /* Clear context switch counter             */
        OSTimeDly(300);                         /* Wait one second                          */
    }
}



static  void  TaskStartCreateTasks (void)
{
    INT8U  i;


    for (i = 0; i < N_TASKS; i++) {                        /* Create N_TASKS identical tasks           */
         OSTaskCreate(Task, (void *)&info[i], &TaskStk[i][TASK_STK_SIZE - 1], i + 1);
     }
}

/*
*********************************************************************************************************
*                                                  TASKS
*********************************************************************************************************
*/

void Task(void *pdata) {
    int start;
    int end;
    int toDelay;

    TaskInfo *taskinfo=(TaskInfo *)pdata;

    OS_ENTER_CRITICAL();
    OSTCBCur->compTime = taskinfo->compTime;
    OSTCBCur->period = taskinfo->period;
    OS_EXIT_CRITICAL();
    //printf("task%d: compTime:%d period:%d",(int)OSTCBCur->OSTCBPrio,OSTCBCur->compTime,OSTCBCur->period);
    start=OSTimeGet();
    while(1)
    {
        while(OSTCBCur->compTime>0)
        {
            //computing
        };
        //PC_DispStr(0, line++, (INT8U *)"going through while loop ", DISP_FGND_WHITE + DISP_BGND_BLACK);
        end=OSTimeGet();
        toDelay=(OSTCBCur->period)-(end-start);
        //printf("task%d: starttime:%d end:%d\n",(int)OSTCBCur->OSTCBPrio,start,end);
        if (toDelay<0)
        {
            printf("%s","DeadLine Violation\n");
            printMessage();
            while(1);
        }
        //printf("%d",toDelay);
        start=start+(OSTCBCur->period);
        OS_ENTER_CRITICAL();
        OSTCBCur->compTime=taskinfo->compTime;
        OS_EXIT_CRITICAL();
        OSTimeDly(toDelay);
    }
}

void printMessage()
{
    static int i = 0;

    for(; i < CtxSwMsgCnt; i++)
        printf("%s",CtxSwMsg[i]);
    
    if (i > 20)
        while(1); //Stuck if message array is full
}
