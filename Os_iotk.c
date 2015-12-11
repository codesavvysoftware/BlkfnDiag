/****************************************************************************
*****************************************************************************
**
** File Name
** ---------
**
** Os_iotk.c
**
*****************************************************************************
*****************************************************************************
**
** Description
** -----------
**
** OS - Operating System Functions and Globals
**
** Full pre-emptive multi-tasking RTOS designed to work with the I/O
** toolkit for the ControlLogix backplane.
**
** Function List
**  1. OS_Scheduler( )
**  2. OS_TimerIsr( )
**  3. OS_CreateTask( )
**  4. os_FindProcess( ) 
**  5. os_InsertProcess( )
**  6. os_RemoveProcess( )
**  7. OS_CreateQueue( )
**  8. OS_PutQueue( )
**  9. OS_GetQueue( )
** 10. os_InitData( )
** 11. main( )
** 12. OS_Assert( )
** 13. OS_DummyIsr( )
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
** I/O TOOLKIT EXAMPLE CODE                                                **
** Copyright (c) 2015 Rockwell Automation Technologies, Inc.               **
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
**     Rev 1.00   28 Jan 2011 DDM  Port from SOE
**     Rev 1.01   28 Aug 2015 BSH  Cleanup unused code - delete OS_Delete(),
**                                   OS_ChangePriority(), OS_Suspend() & 
**                                   OS_Resume(). Remove ROUND_ROBIN ifdefs.
**     Rev 1.02   27 Oct 2015 DDM  Setup for Blackfin watchdog (Lgx00176216)
**
*****************************************************************************
*****************************************************************************
*/

/* Allocate local event log if enabled */
#define LLOG_ALLOC

/* Include the kernel-specific include files.  */
#include "Defs.h"
#include "Os_iotk.h"
#include "Hw.h"
#include "Apex.h"
#include "Nvs_Obj.h"
#include "stdlib.h"
#include <bfrom.h>

/*---------------------------------------------------------------------------
**
** Public Data
**
** OS variables are held in scratchpad memory
**
**---------------------------------------------------------------------------
*/

/* Allocate the system queues. */
#pragma section("L1_scratchpad", NO_INIT)
struct Q_STRUCT queues[ NUM_QUEUES ];      /* Message Q's */

/* OS global variables */
#pragma section("L1_scratchpad", NO_INIT)
struct  OS_PROC_STRUCT*   proc_head;
#pragma section("L1_scratchpad", NO_INIT)
struct  OS_PROC_STRUCT*   current_proc;
#pragma section("L1_scratchpad", NO_INIT)
struct  OS_PROC_STRUCT*   proc_id_index[ MAX_PROCS ];
#pragma section("L1_scratchpad", NO_INIT)
struct  OS_PROC_STRUCT    proc_table[ MAX_PROCS ];
#pragma section("L1_scratchpad", NO_INIT)
struct  STACK_STRUCT      stack;
#pragma section("L1_scratchpad", NO_INIT)
        INT               num_procs;
#pragma section("L1_scratchpad", NO_INIT)
        int               os_heap_idx;
#pragma section("L1_scratchpad", NO_INIT)
        USINT             dummy_isr_count;  /* Count of bad interrupts seen */
#pragma section("L1_scratchpad", NO_INIT)
        UDINT             os_reschedule_counter;
/* User defined crash log data */
USER_CRASH_DATA user_crash_data;

/*---------------------------------------------------------------------------
**
** Private Data
**
**---------------------------------------------------------------------------
*/

/*---------------------------------------------------------------------------
**
** Private Functions
**
**---------------------------------------------------------------------------
*/

struct OS_PROC_STRUCT *os_FindReady(void);
INT  os_FindProcess( USINT priority, struct OS_PROC_STRUCT** proc );
void os_InsertProcess( USINT new_head, struct OS_PROC_STRUCT* proc, struct OS_PROC_STRUCT* new_proc );
void os_RemoveProcess( struct OS_PROC_STRUCT* proc );
INT  os_InitData( void );
void os_TaskReturnAssert( void );

/* Interrupt handlers */
EX_INTERRUPT_HANDLER( OS_Scheduler );
EX_INTERRUPT_HANDLER( OS_TimerIsr );
EX_INTERRUPT_HANDLER( OS_WatchdogIsr );
EX_INTERRUPT_HANDLER( OS_DummyIsr );

/*---------------------------------------------------------------------------
**
** OS_Scheduler( )
**
** Save current context and restore next process which is READY_TO_RUN.
**
**---------------------------------------------------------------------------
**
** Inputs:
**    None
**
** Outputs:
**    None
**
** Usage:
**    Invoked by User interrupt, OS_Scheduler( );
**
**---------------------------------------------------------------------------
*/

/* Turn off optimizer so all scratch registers are saved on stack */
#pragma optimize_off

EX_INTERRUPT_HANDLER ( OS_Scheduler )
{
  struct OS_PROC_STRUCT* proc;  /* Process pointer */

  /* Save preserved register set (R7-R4, P4-P3) in process structure */
  SAVE_PRESERVED_REG(current_proc->preserved);

  /* Macro to save the stack pointer off the current process' to later be restored */
  /* The ISR call itself handles saving all the scratch registers & we just juggle the stacks */
  SAVE_STACK_PTR( current_proc->stack_ptr );
  SAVE_FRAME_PTR( current_proc->frame_ptr );

  /* Increment reschedule counter */
  os_reschedule_counter++;

  /* Stop performance counters for this task */
  CYCLES_STOP(current_proc->stats);

  /* Find next ready task */
  /* Done as a function call to force compiler to save all scratch registers */
  proc = os_FindReady();

  /* Start performance counters for next task */
  CYCLES_START(proc->stats);

  /* Swap the PCSTK, which is the address to execute upon completion of the ISR */
  /*   storing off the existing PCSTK of the current_proc & putting the new proc */
  /*   to execute's address onto the PCSTK */
  SWAP_RETI( current_proc->reti_ptr, proc->reti_ptr );

  current_proc = proc;           /* Set selected proc as current */

  SET_STACK( current_proc->stack_ptr );  /* Restore selected proc's stack - ISR finish will pop it */
  SET_FRAME( current_proc->frame_ptr );  /* Restore selected proc's frame pointer */
  SET_PRESERVED_REG( current_proc->preserved ); /* Restore preserved register set (R7-R4, P4-P3) */
}

#pragma optimize_as_cmd_line

/*---------------------------------------------------------------------------
**
** os_FindReady( )
**
** Walks the ordered linked list to find the process which is READY_TO_RUN.
** The linked list is sorted in descending priority, oldest first in case of
** priority ties.  This allows the scheduler to simply walk the list & pick 
** the first process which is READY_TO_RUN instead of checking all processes.
**
**---------------------------------------------------------------------------
**
** Inputs:
**    None
**
** Outputs:
**    proc - pointer to next process that is ready to run
**
** Usage:
**    struct OS_PROC_STRUCT *os_FindReady(void)
**
**---------------------------------------------------------------------------
*/

struct OS_PROC_STRUCT *os_FindReady(void)
{
  INT                    i = 0;             /* Loop counter */
  INT                    found = FALSE;     /* Flag indicating proc found, early exit from loop */
  struct OS_PROC_STRUCT* proc = proc_head;  /* Process pointer, starts at q head */

  /* Loop through all processes checking if one is ready to be executed, if found */
  /*   exit early & run with it */
  while ( ( i < num_procs ) && !found )
    {
      /* Process ready to run?  Flag if yes, else age & skip to the next */
      if ( proc->state == READY_TO_RUN )
        {
          found = TRUE;
        }
      else
        {
          proc = proc->next;       /* Jump to next process */
          i++;                     /* Bump our loop counter */
        }
    }

  return(proc);
}

/*---------------------------------------------------------------------------
**
** OS_TimerIsr()
**
** Interrupt routine which runs every millisecond to allow background timing
** of various activities as needed.
**
**---------------------------------------------------------------------------
**
** Inputs:
**    None
**
** Outputs:
**    None
**
** Usage:
**    Invoked by low priority timer interrupt, OS_TimerIsr( );
**
**---------------------------------------------------------------------------
*/
EX_INTERRUPT_HANDLER( OS_TimerIsr )
{
  /* TTD
   *  What's it used for.
   */
  
  /* Clear timer interrupt 0 bit in status to ensure interrupt doesn't remain active */
  *pTIMER_STATUS = TIMIL0;  /* TTD which timer */
}

/*---------------------------------------------------------------------------
**
** OS_CreateTask( )
**
** Walks the ordered linked list to find the process which is READY_TO_RUN.
** The linked list is sorted in descending priority, oldest first in case of
** priority ties.  This allows the scheduler to simply walk the list & pick 
** the first process which is READY_TO_RUN instead of checking all processes.
**
**---------------------------------------------------------------------------
**
** Inputs:
**    process_id   Unique ID of process to create
**    priority     Priority level of process being created
**    stack_size   Size of stack to allocate for new process
**    q_id         ID of Q process is affiliated with
**
** Outputs:
**    OK/ERROR     Success code - created or failed based on process_id requested
**
** Alters:
**    proc_head    Adds new process node to linked list which may be new head
**    num_procs    Increments num_procs global variable by 1
**
** Usage:
**    OS_CreateTask( process_id, priority, stack_size, q_id );
**
**---------------------------------------------------------------------------
*/

/* Stucture of initial process stack */
/* Must match how OS_Schedule uses stack */
struct OS_SCH_STACK_STRUCT {
  UDINT prestack[C_PRESTACK/4];
  UDINT regs[OS_SCHEDULE_REGS];
  UDINT link[OS_SCHEDULE_LINK/4];
  UDINT fp;
  UDINT rets;
  UDINT pad[(C_PRESTACK*2)/4];
};

INT OS_CreateTask( SINT process_id,            /* Unique ID of process to create */
                   USINT priority,             /* Priority level of process being created */
                   UINT stack_size,            /* Size of stack to allocate for new process */
                   void (* proc_name)( void )  /* Pointer to location of process being added */
                 )
{
   INT              found = 0;        /* Flag indicating search succesfully found condition */
   INT              i = 0;            /* Loop counter */
   UDINT            Critical;         /* Scratch variable for disabling interrupts */
   UDINT*           stack_ptr;        /* Pointer to stack we're allocating & initalizing */
   UDINT*           temp_stack_ptr;   /* Temporary pointer to stack_ptr we'll manipulate */
   struct OS_PROC_STRUCT*      new_proc;  /* Pointer to new process node being created */
   struct OS_PROC_STRUCT*      proc;      /* Pointer to linked list of process nodes */
   struct OS_SCH_STACK_STRUCT* procStack; /* Initial process stack */
   
   proc = new_proc = NULL;            /* Initialize as NULL pointer */

   /* Ensure a valid process ID was requested.  Valid is within range, non-zero & not */
   /*   currently being used - return error on invalid */
   if ( ( process_id > MAX_PROCS ) || ( process_id < 0 ) || ( proc_id_index[ process_id ] != NULL ) )
   {
      return( ERROR );
   }

   /* Allocate new process node */
   new_proc = &proc_table[process_id];

   /* Initialize new process structure & proc_id_index array */
   new_proc->process_id = process_id;
   new_proc->priority = priority;
   new_proc->state = READY_TO_RUN;               /* Process eligible to begin execution immediately */
   new_proc->q_id = NO_QUEUE;                    /* Process not currently blocked on Q */
   proc_id_index[ process_id ] = new_proc;       /* Add process to array which allows looking up task node off ID */
   CYCLES_INIT(new_proc->stats);

   /* First look for a gap in the stack layout created by a deleted task */
   while( ( i < MAX_PROCS ) && !found )
   {
      if ( stack.gaps[ i ] != NO_GAP )
      {
         if ( stack_size < stack.proc_size[ stack.gaps[ i ] ] )
         {
            found = 1;
            /* Beginning of stack is same as last process to occupy slot */
            stack_ptr = stack.proc_start[ process_id ] = stack.proc_start[ stack.gaps[ i ] ];
            /* Save off full gap despite requested size to ensure we don't have tiny gaps develop */
            stack.proc_size[ process_id ] = stack.proc_size[ stack.gaps[ i ] ];
         }
      }
      i++;
   }

   /* If no gaps to fill, allocate stack from empty space if possible */
   if( !found )
   {
      /* Check if requested stack size will fit - error if not */
      if( stack_size > stack.empty_size )
      {
         return( STACK_ERROR );
      }
      /* Fits so allocate the requested stack space to process */
      else
      {
         stack.empty_size -= stack_size;                /* Reduce empty space by that allocated */
         stack.proc_start[ process_id ] = stack.empty;  /* Save off stack starting point for this process */
         stack_ptr = stack.empty;                       /* Setup local stack pointer for initializing things */
         stack.proc_size[ process_id ] = stack_size;    /* Save off stack size for process */
         stack.empty -= stack_size;                     /* Adjust empty pointer by the stack size we gave away */
      }
   }


   /* Initialize the process's stack unless it's the NULL task - the NULL task */
   /*   is the first task created & thus uses the existing stack and will be */
   /*   the current process when the scheduler is first called which will push */
        /*   the stack */
   if ( process_id != NULL_ID )
   {
      /* Initialize a temporary pointer we'll walk through stack to blank everything */
      temp_stack_ptr = stack_ptr;
      temp_stack_ptr--;

      /* Loop through and initialize everything on stack to 0 */
      /*   These will be used on task switches to store state of proc */
      for( i = 1; i < stack_size ; i++ )
      {
         *temp_stack_ptr-- = 0;
      }
     
      /* Setup initial process stack */
      procStack = (struct OS_SCH_STACK_STRUCT *)(stack_ptr - (sizeof(struct OS_SCH_STACK_STRUCT))/4);
      procStack->fp = (UDINT) ( &procStack->pad[C_PRESTACK/4] );
      procStack->rets = (UDINT)&os_TaskReturnAssert; /* Assert if task ever returns */

      /* Point process stack pointer to top of stack, i.e. last used location */
      new_proc->stack_ptr = (UDINT)procStack;

      /* Point process frame pointer to frame pointer on stack ready for UNLINK */
      new_proc->frame_ptr = (UDINT) ( &procStack->fp );

      /* Store process's location.  This will be the start point for execution when */
      /*   the process is run & will be switched onto the PCSTK in the scheduler    */
      /*   enabling the RTI instruction to pop that location into the PC & execute. */
      new_proc->reti_ptr = (UDINT) proc_name;
   }
   else
   {
      /* Initialize a temporary pointer we'll walk through stack to blank everything */
      SAVE_STACK_PTR( temp_stack_ptr );
      temp_stack_ptr--;

      /* Loop through and initialize NULL stack to 0 below our current SP */
      for( i = 1; i < stack_size ; i++ )
      {
         *temp_stack_ptr-- = 0;
      }
   }

      
   /* Disable interrupts while inserting process into linked list */
   Critical = cli( );
   {
      /* Find insert point in linked list for new process */
      found = os_FindProcess( new_proc->priority, &proc );

      /* Insert new process at insert point */
      os_InsertProcess( found, proc, new_proc );
   } sti( Critical );

   return( OK );
}




/*---------------------------------------------------------------------------
**
** os_FindProcess( )
**
** Walks the ordered linked list to find the process where we would insert 
** a new process of the specified priority.  Returns a pointer to the
** process node which we would insert a new process in front of.            
**
**---------------------------------------------------------------------------
**
** Inputs:
**    priority     Priority level of process to find next lowest
**    proc         Scratch pointer into linked list
**
** Outputs:
**    found        Flag indicating if a node to insert in front of was found
**    *proc        Pointer to process node where a new proc of passed priority
**                   would be inserted in front of.
**
** Alters:
**    None
**
** Usage:
**    os_FindProcess( priority, proc );
**
**---------------------------------------------------------------------------
*/

INT os_FindProcess( USINT                   priority,/* Priority level of process being created */
                    struct OS_PROC_STRUCT** proc     /* Ptr into linked list to walk & return */
                  )
{
   INT         i = 0;         /* Loop counter */
   INT         found = 0;     /* Flag indicating we found it */

   *proc = proc_head;         /* Ensure linked list search starts at head */

   /* Loop through all processes until find proc which would follow the priority/age keys */
   while ( ( i < num_procs ) && !found )
   {
      /* Is proc to find, higher priority?  If so, found it */
      if ( (*proc)->priority < priority )
      {
         found = 1;
      }
      else
      {
         /* Didn't find it yet, check next process node */
         i++;
         *proc = (*proc)->next;
      }
   }
   return( found );
}


/*---------------------------------------------------------------------------
**
** os_InsertProcess( )
**
** Inserts the requested process node, new_proc, prior to the specified node,
**   proc.  Assigns proc_head to new_proc if flagged as such (essentially flag
**   indicates if inserting at head, is the new node the new head or the tail)
**
**---------------------------------------------------------------------------
**
** Inputs:
**    new_head - Flag indicating new_proc is our new proc_head
**    proc     - Node in linked list we're inserting new_proc before
**    new_proc - New node to insert
**
** Outputs:
**    None
**
** Usage:
**    os_InsertProcess( new_head, proc, new_proc );
**
**---------------------------------------------------------------------------
*/

void os_InsertProcess( USINT                  new_head, /* Flag indicating insert a new head */
                       struct OS_PROC_STRUCT* proc,     /* Node in linked list to insert new_proc prior to */
                       struct OS_PROC_STRUCT* new_proc  /* New process node to insert */
                     )
{
   /* Increment process count */
   num_procs++;

   /* If the linked list is empty, the new node is the head */
   if ( proc == NULL )
   {
      /* Point to new head */
      proc_head = new_proc;

      /* Indicate sole process by pointing prev & next to self */
      new_proc->next = new_proc->prev = new_proc;
   }
   else
   {
      /* List not empty, if we're pointing at the list head, check if we actually */
      /*   have a new list head (found on search) */
      if ( ( proc == proc_head ) && new_head )
      {
         /* Point to new head */
         proc_head = new_proc;
      }
      new_proc->next = proc;              /* New next is proc inserting before */
      new_proc->prev = proc->prev;        /* New prev is proc inserting before's prev */
      proc->prev = new_proc;              /* Proc inserting before new prev is new proc */
      new_proc->prev->next = new_proc;    /* Proc inserting after's new next is new proc */
   }
}


/*---------------------------------------------------------------------------
**
** os_RemoveProcess()
**
** Walks the ordered linked list to find the process indicated, removes it from
** linked list, decrements num_procs, and returns the removed process.  Returns
** NULL if process not found.
**
**---------------------------------------------------------------------------
**
** Inputs:
**    *proc        Pointer to process node which is to be removed.
**
** Outputs:
**    None
**
** Alters:
**    proc_head    Deletes process node from linked list which may create new head
**    num_procs    Decrements num_procs global variable by 1
**
** Usage:
**    os_RemoveProcess( *proc );
**
**---------------------------------------------------------------------------
*/

void os_RemoveProcess( struct OS_PROC_STRUCT* proc  /* Ptr to node being removed */
                     )
{
   /* Ensure process to delete pointer is valid (not NULL) */
   if ( proc != NULL )
   {
      num_procs--;                                /* Decrement number of processes, for 1 we're removing */   
      if ( num_procs )                            /* If more than 1 process, remove normally */
      {
         proc->prev->next = proc->next;           /* Point prev to following removed */
         proc->next->prev = proc->prev;           /* Point next to preceding removed */
         proc->prev = proc->next = proc;          /* Disengage removed from pointing into list */
      }
      else                                        /* If no processes left, set head to NULL */
      {
         proc_head = NULL;
      }
   }
}

/*---------------------------------------------------------------------------
**
** OS_CreateQueue( )
**
** Initialize queue, "queue_id".
**
**---------------------------------------------------------------------------
**
** Inputs:
**    queue_id       - Which Q to initialize
**    queue_size     - Max queue size in DINT's
**
** Outputs:
**    OK | ERROR     - Error on invalid Q ID, else OK
**
** Usage:
**    OS_CreateQueue( USINT queue_id, UINT queue_size );
**
**---------------------------------------------------------------------------
*/
INT OS_CreateQueue( USINT  queue_id,         /* ID of Q to initialize */
                    UINT   queue_size        /* Size of queue */
                    )
{
  INT i;

  /* Ensure Q ID is valid */
  if ( queue_id > (USINT)( NUM_QUEUES - 1 ) )
    {
      OS_Assert(OS_ASSERT_FW_ERR);
    }

  /* Queue already in use */
  if ( queues[ queue_id ].q_body != NULL )
    {
      OS_Assert(OS_ASSERT_FW_ERR);
    }

   /*  Initialize the user queue descriptor blocks. */
  queues[ queue_id ].q_size = queue_size;    /* Setup size */
  queues[ queue_id ].q_body = OS_MALLOC(queue_size * sizeof(void *)); /* Setup buffer address */
  queues[ queue_id ].q_head = 0;             /* Set head to first entry of body */
  queues[ queue_id ].q_tail = 0;             /* Set tail to first entry as well */

  /* Did we have memory for queue */
  if ( queues[ queue_id ].q_body == NULL )
    {
      return( ERROR );
    }

  /* Initialize entire message Q to 0 for safety */
  for ( i = 0; i < queue_size; i++ )
    {
      queues[ queue_id ].q_body[ i ] = NULL;
    }
  return( OK );
}

/*---------------------------------------------------------------------------
**
** OS_GetQueue( )
**
** Retrieves message pointer from queue, "queue_id" and places it into "msg".
** If there is not one available, the function will block and wait.
**
**---------------------------------------------------------------------------
**
** Inputs:
**    queue_id       - Which Q to get pointer from
**    *msg           - Where to put the pointer
**
** Outputs:
**    None
**
** Usage:
**    OS_GetQueue( UINT16 queue_id, void* msg );
**
**---------------------------------------------------------------------------
*/

void OS_GetQueue( USINT  queue_id,         /* ID of Q to retrieve message from */
                  void **msg               /* Where to put message upon retrieval */
               )
{
  UDINT  Critical;                        /* Temp variable for disabling interrupts */
  USINT  message_complete = FALSE;        /* Flag indicating if complete message was retrieved */
  struct Q_STRUCT*       q_ptr;           /* Pointer into Q structure */

  /* Ensure Q ID is valid */
  if ( queue_id > (USINT)( NUM_QUEUES - 1 ) )
    {
      OS_Assert(OS_ASSERT_FW_ERR);
    }

  q_ptr = &queues[ queue_id ];             /* Point to Q structure we're interested in */

  /* Don't use queue before it is initialized */
  if ( q_ptr->q_body == NULL )
    {
      OS_Assert(OS_ASSERT_FW_ERR);
    }

  /* Loop until message is done - will read block on Q if data not ready */
  while ( !message_complete )
    {
      /* Disable interrupts prior to checking if blocked, to avoid case of */
      /*   being interrupted just after determining to block, but not having */
      /*   actually blocked which could lead to lost data */
      Critical = cli( );
      {

        /* If the data not yet available, block and reschedule ourselves for execution later. */
        if ( q_ptr->q_tail == q_ptr->q_head )
          {
            current_proc->q_id = queue_id;
            current_proc->state = READ_BLOCKED_ON_QUEUE;
            RESCHEDULE( );
          }
        else
          {
            /* Retrieve message data from Q */
            *msg = q_ptr->q_body[ q_ptr->q_head++ ];

            /* Check if reached end of circular Q, if so, reset to front */
            if ( q_ptr->q_head > (UINT8)( q_ptr->q_size - 1 ) )
              {
                q_ptr->q_head = 0;
              }
            message_complete = TRUE;        /* Have enough data to process */
          }
      }
      sti( Critical );
    }
}


/*---------------------------------------------------------------------------
**
** OS_PutQueue()
**
** Writes message pointer to the specified message Q.  If not enough
** Once write performed, loops through process checking if any read blocked
** on Q specified and if so unblocks them and calls the scheduler.
** Returns an ERROR on an invalid Q ID or Queue full, otherwise OK.
**
**---------------------------------------------------------------------------
**
** Inputs:
**    queue_id       - Which Q to get messages from
**    *msg           - There to put the message
**
** Outputs:
**    OK | ERROR     - Error on invalid Q ID, or overflow, else OK
**
** Usage:
**    OS_PutQueue( UINT16 queue_id, void* msg, UINT8 irq_flag );
**
**---------------------------------------------------------------------------
*/

INT OS_PutQueue( UINT8   queue_id,        /* ID of Q to send message to */
                 void   *msg              /* Where to get message to send */
               )
{
  UINT   data_available;                   /* Amount of space left on Q */
  UDINT  Critical;                         /* Temp variable for disabling interrupts */
  UINT   i;
  INT    got_one          = FALSE;         /* Flag indicating found a blocked process & to run scheduler */
  struct OS_PROC_STRUCT* process_ptr;      /* Pointer into process linked list */
  struct Q_STRUCT*       q_ptr;            /* Pointer into Q structure */

  /* Ensure Q ID is valid */
  if ( queue_id > (UINT16)( NUM_QUEUES - 1 ) )
    {
      OS_Assert(OS_ASSERT_FW_ERR);
    }

  q_ptr = &queues[ queue_id ];             /* Point to Q structure we're interested in */

  /* Don't use queue before it is initialized */
  if ( q_ptr->q_body == NULL )
    {
      OS_Assert(OS_ASSERT_FW_ERR);
    }

  Critical = cli( );   /* Disable interrupts */
  {
    /* Calculate free space on Q - circular buffer so account for head & tail location */
    if ( q_ptr->q_head <= q_ptr->q_tail )
      {
        data_available = q_ptr->q_size - ( q_ptr->q_tail - q_ptr->q_head ) - 1;
      }
    else
      {
        data_available = ( q_ptr->q_head - q_ptr->q_tail ) - 1;
      }

    /* If the Q space requested is not yet available, return error */
    if ( !data_available )
      {
        sti( Critical );
        return(QUEUE_OVERRUN_ERROR);
      }

    /* Write message to Q, we have enough space. */
    q_ptr->q_body[ q_ptr->q_tail++ ] = msg;

    /* Check if circular buffer has looped & if so, reset tail */
    if ( q_ptr->q_tail > ( q_ptr->q_size - 1 ) )
      {
        q_ptr->q_tail = 0;
      }

    /* The fact that we have added characters to the queue may add enough */
    /*    bytes so that reading processes that are blocked on this queue */
    /*    may now execute.  Scan the process blocks.  Unblock any process that */
    /*    is blocked on this queue, and let the scheduler decide which one is */
    /*    to execute next. */

    /* Point to process linked list */
    process_ptr = proc_head;

    /* Loop through all processes searching for a process READ blocked on this Q */
    for ( i = 0; i < num_procs; i++ )
      {
        if ( ( process_ptr->state == READ_BLOCKED_ON_QUEUE ) &&
             ( process_ptr->q_id == queue_id ) )
          {
            /* Found a process Read blocked, clear the block */
            process_ptr->q_id = NUM_QUEUES;                /* Clear processes Q affiliation */
            process_ptr->state = READY_TO_RUN;             /* Set process as ready to execute */
            got_one = TRUE;                                /* Flag we unblocked someone */
          }
        process_ptr = process_ptr->next;
      }
  }
  sti( Critical );

  /* If there was a process blocked on this queue, release it & reschedule */
  if ( got_one )
    {
      RESCHEDULE( );
    }

  return( OK );
}

/*---------------------------------------------------------------------------
**
** os_InitData( )
**
** Initializes OS.
**
**---------------------------------------------------------------------------
**
** Inputs:
**    None
**
** Outputs:
**    OK/ERROR          OK on success, ERROR on invalid ID
**
** Usage:
**    os_InitData( );
**
**---------------------------------------------------------------------------
*/

/* Variable and function extern to the OS */
void Apex_MessageProcess( void );

/* Declare C accessable lable to get stack address from ldf */
extern DINT stack_space asm("ldf_stack_space");

INT os_InitData( void )
{
  INT              i;          /* Loop counter */
  INT              error = 0;  /* Error flag for initialization calls */

  /* Init reschedule counter */
  os_reschedule_counter = 0;

  /* Find index number for os heap */
  if ( (os_heap_idx = heap_lookup(OS_HEAP_USER_ID)) == -1)
    {
      return(ERROR);
    }

  /* Clear SIC IMASK registers since emulator reset does not seem to */
  *pSIC_IMASK0 = 0;
  *pSIC_IMASK1 = 0;

  /*
   * Setup for when Blackfin watchdog expires.
   *   Set NOBOOT so we don't reboot.
   *   Change reset vector so that we jump directly to OS_WatchdogIsr.
   */
  *pSYSCR = (*pSYSCR & ~BCODE) | BCODE_NOBOOT;
  register_handler_ex( ik_reset, OS_WatchdogIsr, EX_INT_DISABLE );

  /* Setup Dummy interrupt handler for any extaneous interrupts */
  register_handler_ex( ik_nmi,          OS_DummyIsr, EX_INT_DISABLE );
  register_handler_ex( ik_exception,    OS_DummyIsr, EX_INT_DISABLE );
  register_handler_ex( ik_hardware_err, OS_DummyIsr, EX_INT_DISABLE );
  register_handler_ex( ik_timer,        OS_DummyIsr, EX_INT_DISABLE );
  register_handler_ex( ik_ivg7 ,        OS_DummyIsr, EX_INT_DISABLE );
  register_handler_ex( ik_ivg8 ,        OS_DummyIsr, EX_INT_DISABLE );
  //  register_handler_ex( ik_ivg9 ,        OS_DummyIsr, EX_INT_DISABLE );
  register_handler_ex( ik_ivg10,        OS_DummyIsr, EX_INT_DISABLE );
  register_handler_ex( ik_ivg11,        OS_DummyIsr, EX_INT_DISABLE );
  //  register_handler_ex( ik_ivg12,        OS_DummyIsr, EX_INT_DISABLE );
  register_handler_ex( ik_ivg13,        OS_DummyIsr, EX_INT_DISABLE );
  //  register_handler_ex( ik_ivg14,        OS_DummyIsr, EX_INT_DISABLE );
  register_handler_ex( ik_ivg15,        OS_DummyIsr, EX_INT_DISABLE );

  /*  Setup ISRs that os uses */

  /* Apex ISRs use ivg9 and ivg12 */
  register_handler_ex( ik_ivg9 ,        Apex_CbaIsr, EX_INT_DISABLE );
  register_handler_ex( ik_ivg12,        Apex_CbbIsr, EX_INT_DISABLE );

  /* OS rescheduler uses ivg14 */
  register_handler_ex( ik_ivg14,        OS_Scheduler, EX_INT_DISABLE );

  /*
   * Setup Blackfin port G
   */
  *pPORTG_FER = 0;
  *pPORTG_HYSTERESIS = 0;

  /* Preset active low output pins */
  *pPORTGIO = DSP_DETECTS_FAULT | APEX_GPIO_0;

  /* Configure active low input pins */
  *pPORTGIO_POLAR = APEX_FAULT | APEX_CBB_INT | APEX_CBA_INT;

  /* Configure output pins */
  *pPORTGIO_DIR = DSP_DETECTS_FAULT | APEX_GPIO_0 | DSP_HB_DATA | DSP_HB_E1 | DSP_HB_E2;

  /* Configure input pins */
  *pPORTGIO_INEN = APEX_FAULT | APEX_CBB_INT | APEX_CBA_INT;

  /* Put Apex CBA and Apex Fault on port G interrupt channel A */
  *pPORTGIO_MASKA = APEX_FAULT | APEX_CBA_INT;

  /* Put Apex CBB on port G interrupt channel B */
  *pPORTGIO_MASKB = APEX_CBB_INT;

  /* Route PORT G interrupt channels to ivg9 and ivg12 respectivly */
  *pSIC_IAR5 = ( *pSIC_IAR5 & 0xFFFFFF00 ) | 0x00000052;

  /* Setup empty & empty size according to stack parameters in LDF file */
  SAVE_STACK_PTR( stack.empty );
  stack.empty_size = ((DINT)stack.empty - (DINT)&stack_space) / sizeof(DINT);

  /* No processes */
  proc_head = NULL;
  num_procs = 0;

  /* Initialize stack gaps tracker to no gaps - stack is continuous & unallocated */
  /*   also clear proc_id_index[] */
  for ( i = 0; i < MAX_PROCS; i++ )
    {
      stack.gaps[ i ] = NO_GAP;
      proc_id_index[ i ] = (struct OS_PROC_STRUCT *)NULL;
    }

  /*  Clear the user queue descriptor blocks. */
  for ( i = 0; i < NUM_QUEUES; i++ )
    {
      queues[ i ].q_size = 0;;            /* Setup size */
      queues[ i ].q_body = NULL;          /* Setup buffer pointer */
      queues[ i ].q_head = 0;             /* Set head to first entry of body */
      queues[ i ].q_tail = 0;             /* Set tail to first entry as well */
    }

  /* Initialize message queue */
  OS_CreateQueue(MSG_Q_ID, MSG_Q_SIZE);

  /* Create the toolkit system tasks which must be present.  In the task creation call, the  */
  /*   process ID, Priority, Stack Size, and routine name are all passed.                    */
  /*   System tasks are:                                                                     */
  /*     User_NullProcess    - the null task which is lowest priority & never suspends       */
  /*     Apex_MessageProcess - message task to handle service requests                       */
  error |= OS_CreateTask( NULL_ID, NULL_PRIORITY, NULL_STACK_SIZE, User_NullProcess );
  error |= OS_CreateTask( MSG_ID,  MSG_PRIORITY,  MSG_STACK_SIZE,  Apex_MessageProcess );

  /* Initialize local event log if enabled */
  LLOG_INIT();

  return( error );
}


/*---------------------------------------------------------------------------
**
** Main()
**
** Initial C routine which initializes the system & eventually calls the
** Null task to execute...
**
**---------------------------------------------------------------------------
**
** Inputs:
**    None
**
** Outputs:
**    None
**
** Usage:
**    Main( );
**
**---------------------------------------------------------------------------
*/

void main( void )
{
  INT    error;
  UDINT  Critical = 0;             /* Temp to disable interrupts */

  SET_BUS_CORE_SPEEDS( Critical ); /* Core at 520 MHz, SCLK at 86.667 MHz */
  ENABLE_EXTERNAL_MEM( );          /* Enable all external memory banks */

  /* Temporarily shutdown all interrupts until we're initialized properly */
  Critical = cli( );

  /* Running in boot code */
#ifdef  COMPILE_BOOT
  /* Do we need to update flash and if so did we update the boot code */
  if ( Nvs_Program() == OK )
    {

      /* Boot code changed so reboot */

      /* Reset Blackfin DSP */
      RESET_DSP();

      /* Should never get here */
      while(1);
    }

  /* Do we have a valid application */
  if ( Nvs_CheckApp((UINT*)NVS_MAIN_START_ADDR,
                    NVS_MAIN_MAX_SIZE,
                    NULL,
                    false) == OK )
    {
      /* If valid app then start it */
      bfrom_MemBoot((void *)NVS_MAIN_START_ADDR, 0, 0, 0);
    }
#endif

  /* Initialize all system data structures and variables. */
  if ( ( error = os_InitData( ) ) != OK )
    {
      /* Major failure - assert  */
      OS_Assert( OS_ASSERT_INITDATA | ( error << 16 ) );
    }

  /* Initialize Apex ASIC */
  if ( ( error = Apex_Init( ) ) != OK )
    {
      /* Major failure - assert  */
      OS_Assert( OS_ASSERT_APEXINIT | ( error << 16 ) );
    }

  /* Initialize Host/Apex watchdog and start DSP watchdog */
  Apex_WatchdogInit( &pHI_ApexParam->Watchdog );

  /* Start Fault Injection Point 1 */
  /* Don't enable fault latch for Apex Fault Injection tests */
#ifdef  RELEASE_BUILD
  /* Enable fault logic for Release builds */
  ENABLE_FAULT();
#endif
  /* End Fault Injection Point 1 */

  /* Now call the user's initialization routine. If it fails to */
  /*   return SUCCESSFUL, halt the kernel NOW. */
  if ( User_Init( ) != OK )
    {
      /* Major failure - assert */
      OS_Assert( OS_ASSERT_USERINIT );
    }

  /* Manually kick DSP watchdog (Apex not started yet) */
  HW_WatchdogKick();

  /* Boot Apex ASIC */
  /*   Done after OS_UserInit modifies values in Apex parameter table */
  if ( ( error = Apex_Boot( ) ) != OK )
    {
      /* Major failure - assert  */
      OS_Assert( OS_ASSERT_APEXBOOT | ( error << 16 ) );
    }

  /* Apex started so start normal watchdog */
  Apex_WatchdogKick();

  /* Setup PORT G interrupt masks */
  /* Apex toggles CBA_INT line after reset so we have to set the masks here */
  *pSIC_IMASK1 |= (IRQ_PFA_PORTG | IRQ_PFB_PORTG);
  
   /* Set current process to the Null task, User_NullProcess( ) */
  current_proc = proc_id_index[ NULL_ID ];

  /* Start NULL task performance counters */
  CYCLES_START(current_proc->stats);

  /* Call the null process - will not return */
  User_NullProcess( );
}

/*---------------------------------------------------------------------------
**
** OS_Assert( )
**
** Routine which is called upon catostrophic error allowing an orderly crash.
**
**---------------------------------------------------------------------------
**
** Inputs:
**    error - who called us in crash proceedings.
**
** Outputs:
**    None
**
** Usage:
**    
** Treat _OS_Assert as a killpath in Coverity
**
**---------------------------------------------------------------------------
*/

/* coverity[+kill] */
void _OS_Assert( INT error_num, char *file, int line )
{
  INT              i;
  NVS_CRASH_HEADER header;
  UDINT            imask;       /* Temp variable for disabling interrupts */

  imask = cli( );               /* Turn off all interrupts so we can't escape */

  /* Collect crash info */
  header.dsp_error_num = error_num; 
  header.dsp_file      = file;
  header.dsp_line      = line;
  asm("%0=SEQSTAT;" : "=d" ( header.dsp_seq_stat) : );
  asm("%0=RETI;"    : "=d" ( header.dsp_reti) : );
  asm("%0=RETX;"    : "=d" ( header.dsp_retx) : );
  asm("%0=RETE;"    : "=d" ( header.dsp_rete) : );
  asm("%0=RETN;"    : "=d" ( header.dsp_retn) : );
  asm("%0=FP;"      : "=d" ( header.dsp_fp) : );
  asm("%0=SP;"      : "=d" ( header.dsp_sp) : );

  /* Save preserved register set (R7-R4, P4-P3) in header */
  SAVE_PRESERVED_REG(header.preserved);

  /*
   * Collect Apex info
   * Only try to get Apex exception file and line number if assert was NOT a watchdog.
   * One reason for watchdog is Apex bus lockup so don't try to access Apex memory
   * or we might lockup again.
   */
  header.dsp_port_g = *pPORTGIO;
  if ( error_num != OS_ASSERT_WDOG ) {
    header.apex_HwExceptFile = pHI_ApexParam->HwExceptFile;
    header.apex_HwExceptLine = pHI_ApexParam->HwExceptLine;
  }
  else {
    header.apex_HwExceptFile = NULL;
    header.apex_HwExceptLine = 0;
  }

  /* Add user info */
  header.user.param1 = user_crash_data.param1;
  header.user.param2 = user_crash_data.param2;
  header.user.param3 = user_crash_data.param3;

  /* Write crash log data */
  Nvs_WriteCrashLog(&header);

  /* Force module fault from our side */
  FAULT();
  ssync();

  /* MODULE_FAULT_L requires Apex clock so give it a chance before we try anything else */
  HW_Delay(250);

#ifdef	RELEASE_BUILD
  /*
   * If RELEASE_BUILD and we make it here some hardware is broke because module fault
   * should have already reset us. Try to shutdown manually.
   */

  /* Force Apex reset */
  RESET_APEX();
  ssync();

  /* Reset Blackfin I/O */
  bfrom_SysControl(SYSCTRL_SYSRESET, NULL, NULL);
#endif

  /* Things are really messed up and nothing else to do but spin. */
  while( TRUE )
    {
    }
}

/*---------------------------------------------------------------------------
**
** os_TaskReturnAssert( )
**
** Function to declare assert if task ever returns
**
**---------------------------------------------------------------------------
**
** Inputs:
**    None
**
** Outputs:
**    None
**
** Usage:
**    
**
**---------------------------------------------------------------------------
*/
void os_TaskReturnAssert( void )
{
  OS_Assert(OS_ASSERT_FW_ERR);
}

/*---------------------------------------------------------------------------
**
** OS_WatchdogIsr( )
**
** ISR runs if WatchDog failure.
**
**---------------------------------------------------------------------------
**
** Inputs:
**    None.
**
** Outputs:
**    None
**
** Usage:
**    Invoked by RESET exception on WDOG TimeOut
**
**    Warning
**    This routine uses the boot rom stack in scratch pad memory.
**    We should be ok since the scratch pad is reserved for OS data and we are
**    not running the OS any more.
**---------------------------------------------------------------------------
*/
EX_INTERRUPT_HANDLER ( OS_WatchdogIsr )
{
  UDINT  Critical = 0;

  /*
   * Since DSP was reset, restart enough hardware so we can process the Assert
   */

  /* Re-enable the cycle counter */
  asm volatile ("%0 = syscfg; bitset(%0, 1); syscfg = %0;" ::"?d"(0));
  /* Core at 520 MHz, SCLK at 86.667 MHz */
  SET_BUS_CORE_SPEEDS( Critical );
  /* Enable all external memory banks */
  ENABLE_EXTERNAL_MEM( );
  /* Setup Blackfin port G */
  *pPORTG_FER = 0;
  *pPORTG_HYSTERESIS = 0;
  *pPORTGIO = DSP_DETECTS_FAULT | APEX_GPIO_0 | DSP_HB_DATA | DSP_HB_E1 | DSP_HB_E2;
  *pPORTGIO_POLAR = APEX_FAULT | APEX_CBB_INT | APEX_CBA_INT;
  *pPORTGIO_DIR = DSP_DETECTS_FAULT | APEX_GPIO_0 | DSP_HB_DATA | DSP_HB_E1 | DSP_HB_E2;

   /*
    * Now we can Assert
    */
   OS_Assert( OS_ASSERT_WDOG );
}

/*---------------------------------------------------------------------------
**
** OS_DummyIsr( )
**
** Extraneous interrupt handler to minimize the chance of crash on a bad ISR.
**
**---------------------------------------------------------------------------
**
** Inputs:
**    int_num - number of interrupt which invoked ISR.
**
** Outputs:
**    None
**
** Usage:
**    Should never execute, OS_DummyIsr( int_num, b, c );
**
**---------------------------------------------------------------------------
*/
EX_INTERRUPT_HANDLER ( OS_DummyIsr )
{

  /* Assert - stop running */
  OS_Assert( OS_ASSERT_HW_ERR );
}
