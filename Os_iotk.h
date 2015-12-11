/****************************************************************************
*****************************************************************************
**
** File Name
** ---------
**
** Os_iotk.h
**
*****************************************************************************
*****************************************************************************
**
** Description
** -----------
**
** OS related #defines, typedefs & prototypes
**
**
*****************************************************************************
*****************************************************************************
**
** Source Change Indices
** ---------------------
**
** Porting: <none>0----<major>         Customization: <none>0----<major>
**
*****************************************************************************
*****************************************************************************
**                                                                         **
** Copyright (c) 2015 Rockwell Automation Technologies, Inc.               **
** All rights reserved.                                                    **
**                                                                         **
** All rights reserved, except as specifically licensed in writing.        **
** The following work constitutes example program code and is intended     **
** merely to illustrate useful programming techniques.  The user is        **
** responsible for applying the code correctly.  The code is provided      **
** AS IS without warranty and is in no way guaranteed to be error-free.    **
** This example code is for use only in and for ControlLogix products, but **
** is specifically excluded from use in developing ANY communications      **
** source code or development tools for sale or distribution.  Please      **
** consult the licensing documents which apply to this code for more       **
** information.                                                            **
**                                                                         **
*****************************************************************************
*****************************************************************************
*/

/****************************************************************************
*****************************************************************************
**
** Change Log
** ----------
**
** Latest Revision:
**  
**     Rev 1.00   02 Aug 2011 DDM  Created from SOE
**     Rev 1.01   20 Jul 2015 DDM  Use INLINE macro
**     Rev 1.02   05 Nov 2015 DDM  Add OS_ASSERT_APEXFAULT
**     Rev 1.03   11 Dec 2015 TNH  Add extern "C" to provide linkage to C++
**                                 code for OS_Assert();
**
**
*****************************************************************************
*****************************************************************************
*/

#ifndef OS_IOTK_H
#define OS_IOTK_H

#include <exception.h>
#include <ccblkfn.h>
#include "Defs.h"
#include <cycles.h>

/*---------------------------------------------------------------------------
**
** Public structs and defines
**
**---------------------------------------------------------------------------
*/

#define CURRENT_PROC      -1   /* Special process_id flag meaning current_proc */
#define NO_GAP            -1   /* -1 indicates not a gap in stack */
#define MAX_PROCS         12   /* Max number of procs ever in system.        */

/* Defines from dis-assembly of OS_Schedule function to calculate initial process stacks */
#define OS_SCHEDULE_LINK        4       /* OS_Schedule link instuction value */
#define OS_SCHEDULE_REGS        35      /* Number of registers OS_Schedule pushes on stack */
#define C_PRESTACK              12      /* Stack size reserved by compiler of all C calls */

/* Message Queue defines */
#define NUM_QUEUES         5   /* Define maximum number of message queues in use */
#define NO_QUEUE           NUM_QUEUES /* No Q in use is set to max queues allowing */

/* Message queue id */
#define MSG_Q_ID           0   /* Message receive message Q */
#define MSG_Q_SIZE         32  /* Message queue size */

/* Maximum queue id used by OS */
#define MAX_OS_QUEUE       MSG_Q_ID

/* OS Task defines */
#define NULL_ID            0   /* Process ID of OS_NullProcess( ) */
#define NULL_PRIORITY      0   /* Priority of NullProc - lowest possible */
#define NULL_STACK_SIZE    400 /* Stack size of null task */

#define MSG_ID             1   /* Process ID of Apex_MessageProcess( ) */
#define MSG_PRIORITY       50  /* Priority of Apex_MessageProcess  */
#define MSG_STACK_SIZE     800 /* Stack size of Msg task */

/* Maximum task id used by OS */
#define MAX_OS_ID          MSG_ID


struct OS_PROC_STRUCT
{
   UDINT  preserved[7];         /* Preserved reg set. For optimization keep as first element of structure */
   SINT   process_id;           /* ID of process - used for array indeces etc. */
   USINT  priority;             /* Process priority level (larger #is higher) */
   USINT  state;                /* Current process state, ready to run, blocking etc. */
   USINT  q_id;                 /* Queue affiliated with, if any */
   UDINT  stack_ptr;            /* Pointer to top of process's stack */
   UDINT  frame_ptr;            /* Pointer to process's current frame on stack */
   UDINT  reti_ptr;             /* Value to place in RETI reg to cause the proc to execute after RTI */
#ifdef  DO_CYCLE_COUNTS
   cycle_stats_t stats;         /* Performance measurments */
#endif
   struct OS_PROC_STRUCT* next; /* The next process node in linked list */
   struct OS_PROC_STRUCT* prev; /* The prior process node in linked list */
};


struct STACK_STRUCT
{
   UDINT* empty;                   /* Points to block of empty stack */
   UINT   empty_size;              /* Size of empty stack block */
   UDINT* proc_start[ MAX_PROCS ]; /* Where each process's top of stack is */
   UINT   proc_size[ MAX_PROCS ];  /* Size of process's stack */
   SINT   gaps[ MAX_PROCS ];       /* Array indicating if a gap exists in stack space from */
                                   /*   a deleted process - # is index into proc_start[] */
                                   /*   for a gap if value not less than zero.  -1 not empty */
};

#define SAVE_STACK_PTR( stack )       \
        asm("%0=SP;" : "=d" ( stack ) : )

#define SAVE_FRAME_PTR( frame )       \
        asm("%0=FP;" : "=d" ( frame ) : )

/* OS_Schedule saves and restores P5 so we just need to save the rest (R4-R7, P3-P4) */
#define SAVE_PRESERVED_REG( preserved ) \
        asm("%0=R4;" : "=d" ( preserved[0] ) : ); \
        asm("%0=R5;" : "=d" ( preserved[1] ) : ); \
        asm("%0=R6;" : "=d" ( preserved[2] ) : ); \
        asm("%0=R7;" : "=d" ( preserved[3] ) : ); \
        asm("%0=P3;" : "=d" ( preserved[4] ) : ); \
        asm("%0=P4;" : "=d" ( preserved[5] ) : ); 
 

#define SET_STACK( stack )    \
        asm("SP=%0;" : : "d" ( stack ) : )

#define SET_FRAME( frame )    \
        asm("FP=%0;" : : "d" ( frame ) : )

#define SET_PRESERVED_REG( preserved ) \
        asm("P4=%0;" : : "d" ( preserved[5] ) : );  \
        asm("P3=%0;" : : "d" ( preserved[4] ) : );  \
        asm("R7=%0;" : : "d" ( preserved[3] ) : );  \
        asm("R6=%0;" : : "d" ( preserved[2] ) : );  \
        asm("R5=%0;" : : "d" ( preserved[1] ) : );  \
        asm("R4=%0;" : : "d" ( preserved[0] ) : )

#define SWAP_RETI( store_in, restore )      \
        asm("%0=RETI;" : "=d" ( store_in ) : );  \
        asm("RETI=%0;" : : "d" ( restore ) : )

/* OS Reschedule Event Group mask - using Interrupt 14 on BlackFin */
#define OS_RESCHEDULE_EVT       EVT_IVG14

/* Trigger SW interrupt to reschedule */
#define RESCHEDULE( )                \
        asm("#include <def_LPBlackfin.h>");    \
        asm("RAISE EVT_IVG14_P;")

/* Mask to feed sti( ) to enables global interrupts for OS */
#define OS_INTERRUPTS           (OS_RESCHEDULE_EVT | EVT_IVG12 | EVT_IVG9)

/* OS Memory Allocation */
#define OS_HEAP_USER_ID          1

extern int os_heap_idx;

INLINE
void * OS_MALLOC(size_t length)
{
  UDINT imask;
  void *ptr;

  imask = cli();
  ptr = heap_malloc(os_heap_idx, length);
  sti(imask);
  return(ptr);
}

INLINE
void OS_FREE(void *ptr)
{
  UDINT imask;

  imask = cli();
  heap_free(os_heap_idx, ptr);
  sti(imask);
}

INLINE
void * OS_CALLOC(size_t nelem, size_t elsize)
{
  UDINT imask;
  void *ptr;

  imask = cli();
  ptr = heap_calloc(os_heap_idx, nelem, elsize);
  sti(imask);
  return(ptr);
}

INLINE
void * OS_REALLOC(void *ptr, size_t length)
{
  UDINT imask;
  void *ptr2;

  imask = cli();
  ptr2 = heap_realloc(os_heap_idx, ptr, length);
  sti(imask);
  return(ptr2);
}

/*  Define the Error Codes which may be returned by Operating System  */
/*  Calls.  OK for success already declared in defs.h                 */

#define FAILURE           0x7FFF  /* General non-specific failure code.*/
                                   /* This will fit into INT,UINT,USINT.*/
#define RESUME_PROCESS_ERROR    1 /* Attempt to RESUME process that was*/
#define PROC_DELETED_ERROR      2 /* Attempt to RESUME process that was*/
                                   /* not blocked by SUSPEND call.      */
#define QUEUE_OVERRUN_ERROR     3 /* Attempted put_message to full que.*/
#define STACK_ERROR             4 /* No room left on stack for request */ 

#define BLOCKING     1
#define NON_BLOCKING 0

/*  States of a process within the operating system scheduling scheme.  */
/*  A process may be in only ONE of these states at any one time.       */
/*  Combinations of states are not possible.                            */

#define READY_TO_RUN             0 /* Is no longer blocked...may execute. */
#define WRITE_BLOCKED_ON_QUEUE   1 /* Waiting to write to a queue.        */
#define READ_BLOCKED_ON_QUEUE    2 /* Waiting to read from a queue.       */
#define SUSPENDED                4 /* Execution has been suspended.       */

/*  Terminal Error state list.  */
#define OS_ASSERT_INITDATA       1      /* Init call from main( ) failed */
#define OS_ASSERT_APEXINIT       2      /* Apex initialization failed */
#define OS_ASSERT_APEXBOOT       3      /* Apex boot failed */
#define OS_ASSERT_USERINIT       4      /* User init call failed */
#define OS_ASSERT_MALLOC         5      /* Mem allocation failure */
#define OS_ASSERT_WDOG           6      /* WatchDog assert */
#define OS_ASSERT_HW_ERR         7      /* Hardware Error assert */
#define OS_ASSERT_FW_ERR         8      /* Firmware Error assert */
#define OS_ASSERT_APEXFAULT      9      /* Apex Fault assert */

/* Define the descriptor block for the system queues.  Queues are internally */
/* handled like ring buffers (circular queues), but this is transparent to   */
/* the user.                                                                 */

struct Q_STRUCT
{
   void **q_body;                       /* Pointer to Q data */
   USINT  q_size;                       /* Size of Q */
   USINT  q_head;                       /* Head of Q location as array slot */
   USINT  q_tail;                       /* Tail of Q as array slot */
};


/*---------------------------------------------------------------------------
**
** Public Data
**
**---------------------------------------------------------------------------
*/
extern UDINT os_reschedule_counter;

/*---------------------------------------------------------------------------
**
** Public Functions
**
**---------------------------------------------------------------------------
*/

/* Macro to convert old OS_Assert() to new _OS_Assert() */
#define OS_Assert(error_num)    _OS_Assert(error_num, __FILE__, __LINE__)

INT  OS_CreateTask( SINT process_id, USINT priority, UINT stack_size, void (* proc_name)( void ) );
INT  OS_CreateQueue( USINT queue_id, UINT queue_size );
void OS_GetQueue( USINT queue_id, void **message );
INT  OS_PutQueue( USINT queue_id, void *message );

/* coverity[+kill] */
extern "C" void _OS_Assert( INT error_num, char *file, int line );

/****************************************************************************
**
** User.c public function prototypes
**
*****************************************************************************
*/
INT  User_Init( void );
void User_NullProcess( void );

#endif  /* OS_IOTK_H */
