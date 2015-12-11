/****************************************************************************
*****************************************************************************
**
** File Name
** ---------
**
** Apex.c
**
*****************************************************************************
*****************************************************************************
**
** Description
** -----------
**
** Apex support functions
**
** Functions which provide the interface to the Apex.
**
** Function List
**  1. Apex_Init( )
**  2. Apex_Boot( )
**  3. Apex_MessageProcess( )
**  4. Apex SendRqst( )
**  5. Apex SendResp(
**  6. Apex_SendCons( )
**  7. Apex_SendProd( )
**  8. Apex_CbaIsr( )
**  9. apex_cba_resp( )
** 10. Apex_CbbIsr( )
** 11. apex_cbb_resp( )
** 12. Apex_InitLock( )
** 13. Apex_Lock( )
** 14. Apex_TryLock( )
** 15. Apex_Unlock( )
** 16. Apex_WatchdogInit( )
** 17. Apex_WatchdogKick( )
** 18. apex_PreTest( )
** 19. apex_RegisterTest( )
** 20. apex_MemoryTest( )
** 21. apex_InterruptTest( )
** 22. apex_PostTest( )
** 23. apex_dummy_vector( )
** 24. Apex_GetHostCb( )
** 25. Apex_GetTime( )
** 26. Apex_UpdateTime( )
** 27. apex_AlarmInit()
** 28. Apex_AlarmSet()
** 29. Apex_AlarmReset()
** 30. apex_add_alarm()
** 31. Apex_AlarmCancel()
** 32. Apex_AlarmTrip()
** 33. apex_Alarm()
** 34. Apex_GetSlot( )   Inline function (see Apex.h)
**
*****************************************************************************
*****************************************************************************
**
** Source Change Indices
** ---------------------
**
** Porting: <none>0----<major>         Customization: <none>---0-<major>
**
*****************************************************************************
*****************************************************************************
**                                                                         **
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
**     Rev 1.00   04 Feb 2011 DDM  Created from SOE
**     Rev 1.01   19 Aug 2015 BCK  Re-Factored Apex_MessageProcess() to reduce complexity.
**     Rev 1.02   05 Nov 2015 DDM  Add Apex fault detections and Blackfin
**                                 watchdog (Lgx00176126)
**     Rev 1.03   11 Dec 2015 TNH  Added extern "C" to function signatures.
**
*****************************************************************************
*****************************************************************************
*/

#include <limits.h>
#include "Hw.h"
#include "Apex.h"
#include "Os_iotk.h"
#include "Mr.h"
#include "Nvs_Obj.h"

/* Set to 1 to perform runtime check alarm list */
#define ALARM_DEBUG     0

/*---------------------------------------------------------------------------
**
** Public Data
**
**---------------------------------------------------------------------------
*/

void *msg_q_buffer[MSG_Q_SIZE]; /* Message task queue */

/* Apex Host CB priority vector tables */
pFunc_t apex_cba_vectors[HI_NUM_CB_IRQ_VEC];
pFunc_t apex_cbb_vectors[HI_NUM_CB_IRQ_VEC];

/* Put Apex binary is separate segment to remain in flash (see *.ldf file) */
#pragma section("ApexBinary")
#include "apexbin.h"

/*---------------------------------------------------------------------------
**
** Private Data
**
**---------------------------------------------------------------------------
*/

UDINT apex_task_counter  = 0;
ULINT max_watchdog_time  = 0;
ULINT apex_watchdog_time = 0;

HOST_CBS host_cbs[NUM_HOST_CBS];  /* Host Cb table */

/* Apex time cache and rollover detect values */
UDINT   apex_time_high_cache      = 0;
UDINT   last_apex_time_high_cache = ULONG_MAX;

/*******************************************************************************
* These variables implement the alarm list.  The alarm list is a doubly linked,
* circular list of alarm records ordered on the amount of time remaining 
* before the alarm expires.  When an alarm expires, all alarm records with a 
* delta time of zero are removed from the alarm list and the routine specified
* in the alarm record is executed.
*******************************************************************************/
APEX_ALARM_REC *Alarm_Head;     /* Pointer to the first alarm list record */
APEX_ALARM_REC *Alarm_Tail;     /* Pointer to the last alarm list record */

/*******************************************************************************
* This is an alarm record which always resides on the alarm list.  It's purpose
* is to simplify the code which manipulates the alarm list.
*******************************************************************************/
APEX_ALARM_REC A_Head;          /* Always the first and last record on the alarm list */

/*---------------------------------------------------------------------------
**
** Private Functions
**
**---------------------------------------------------------------------------
*/

void Apex_MessageProcess(void);
void apex_cba_resp(void);
void apex_cbb_resp(void);
INT apex_PreTest(void);
INT apex_RegisterTest(void);
INT apex_MemoryTest(void);
INT apex_InterruptTest(void);
INT apex_PostTest(void);
void apex_dummy_vector(void);
INT apex_add_alarm(ULINT delay_time, APEX_ALARM_REC *alarm);
void apex_Alarm(void);
void dbg_chk_alarm_list(void);


/*---------------------------------------------------------------------------
**
** Apex_Init( )
**
** Basic initialization of Apex ASIC
**
**---------------------------------------------------------------------------
**
** Inputs:
**    None
**
** Outputs:
**    OK/ERROR
**
** Usage:
**    Apex_Init( );
**
**---------------------------------------------------------------------------
*/

INT Apex_Init(void)
{
  volatile UDINT *dst;
  UINT apex_fault;
  INT  temp;
  int  i;
#ifdef  APEX_EMULATOR
  static bool first_time = true;
#endif

  /* Make sure fault latch is disabled before yanking APEX reset */
  DISABLE_FAULT();

#ifndef  APEX_EMULATOR

  /* Toggle Apex Reset */
  TOGGLE_APEX_RESET(apex_fault);

  /* Apex should have pulled fault line while reset */
  if(!apex_fault) {
    return(ERROR);
  }

  /* Allow Apex powerup to complete */
  HW_Delay(APEX_PWRUP_TIME);

  /* Is Apex faulted after powerup */
  if (*pPORTGIO & APEX_FAULT) {
    return(ERROR);
  }

  /* Set Host Interface Config */
  /* Must be 16 bit write to a 32 bit register */
  UINT *pHostInterfConf = (UINT *)&pHI_ApexReg->HostInterfConf;
  *pHostInterfConf = APEX_HOST_INTERF_CONF;

  HW_Delay(APEX_HOST_INTERF_TIME);

  /* Set AsicCtrl before we clear Apex memory */
  pHI_ApexReg->AsicCtrl = APEX_ASIC_CTRL;

  /* Perform pretest on Apex Asic */
  if (temp = apex_PreTest()) {
    return(temp);
  }

  /* Clear Apex memory */
  dst = (UDINT *)APEX_RAM;
  for(i = 0; i < APEX_RAM_SIZE/sizeof(UDINT); i++)
    {
      dst[i] = 0;
    }

#else   /* APEX_EMULATOR */

  /* Was this a power up reset */
  if ( first_time ) {

    first_time = false;

    /* Toggle Apex Reset */
    TOGGLE_APEX_RESET(apex_fault);

    /* Allow Apex powerup to complete */
    HW_Delay(APEX_PWRUP_TIME);

    /* Set Host Interface Config */
    /* Must be 16 bit write to a 32 bit register */
    UINT *pHostInterfConf = (UINT *)&pHI_ApexReg->HostInterfConf;
    *pHostInterfConf = APEX_HOST_INTERF_CONF;

    HW_Delay(APEX_HOST_INTERF_TIME);

    /* Set AsicCtrl before we clear Apex memory */
    pHI_ApexReg->AsicCtrl = APEX_ASIC_CTRL;

    /* Clear Apex memory */
    dst = (UDINT *)APEX_RAM;
    for(i = 0; i < APEX_RAM_SIZE/sizeof(UDINT); i++)
      {
        dst[i] = 0;
      }
  }
  else {
    /* Set Host Interface Config */
    /* Must be 16 bit write to a 32 bit register */
    UINT *pHostInterfConf = (UINT *)&pHI_ApexReg->HostInterfConf;
    *pHostInterfConf = APEX_HOST_INTERF_CONF;

    HW_Delay(APEX_HOST_INTERF_TIME);

    /* Set AsicCtrl before we clear Apex memory */
    pHI_ApexReg->AsicCtrl = APEX_ASIC_CTRL;

    /* Clear Apex parameter memory */
    dst = (UDINT *)APEX_PARAM;
    for(i = 0; i < APEX_PARAM_SIZE/sizeof(UDINT); i++)
      {
        dst[i] = 0;
      }
    /* Clear Apex I/O memory */
    dst = (UDINT *)APEX_IO_MEMORY;
    for(i = 0; i < APEX_IO_MEMORY_SIZE/sizeof(UDINT); i++)
      {
        dst[i] = 0;
      }
  }

#endif  /* APEX_EMULATOR */

  /* Preset all Apex CB vectors to dummy vector */
  for(i = 0; i < HI_NUM_CB_IRQ_VEC ;i++)
    {
      apex_cba_vectors[i] = apex_dummy_vector;
      apex_cbb_vectors[i] = apex_dummy_vector;
    }

  /*
   * Initialize the alarm list head and tail pointers.
   */
  Alarm_Head = &A_Head;
  Alarm_Tail = &A_Head;

  /*
   * Initialize the link fields in the alarm header record. 
   */
  A_Head.prev = &A_Head;
  A_Head.next = &A_Head;

  /*
   * Initialize the data fields in the alarm header record.
   */
  A_Head.expiration_time = 0;
  A_Head.alarm_routine = NULL;

  /* Initialize vector table */
  apex_cba_vectors[HI_VAL_CB_IRQ_VEC_RESA] = apex_cba_resp;
  apex_cbb_vectors[HI_VAL_CB_IRQ_VEC_RESB] = apex_cbb_resp;
  apex_cba_vectors[HI_VAL_CB_IRQ_VEC_TTEH] = apex_Alarm;

  return(OK);
}

/*---------------------------------------------------------------------------
**
** Apex_Boot( )
**
** Boot Apex ASIC
**
**---------------------------------------------------------------------------
**
** Inputs:
**    None
**
** Outputs:
**    OK/ERROR
**
** Usage:
**    Apex_Boot( );
**
**---------------------------------------------------------------------------
*/

INT Apex_Boot(void)
{
  UDINT *src, *dst;
  int  i;
  INT temp;

#ifndef APEX_EMULATOR

  dst = (UDINT *)APEX_RAM;
  src = (UDINT *)apexbin;

  /* Copy Apex code from flash */
  for(i = 0; i < sizeof(apexbin)/sizeof(UDINT); i++)
    {
      *dst++ = *src++;
    }

  /* Tell Apex it's code is there */
  pHI_ApexReg->GloIntBusLock |= HI_BIT_GIBL_VRC;

  /* Wait for ack from firmware */
  i = APEX_BOOT_TIME / 100000LL;
  do {
    HW_Delay(100000LL);

    /* Manually kick DSP watchdog */
    HW_WatchdogKick();

    /* Did Apex fault during boot */
    if (*pPORTGIO & APEX_FAULT) {
      return(ERROR);
    }
  } while((pHI_ApexReg->GloIntBusLock & HI_BIT_GIBL_VRC) && (--i));

  /* Apex did not respond */
  if (i == 0) {
    return(ERROR);
  }

  /* Finally do a check to make sure apex firmware is active */
  if(temp = apex_PostTest()) {
    return(temp);
  }

#else   /* APEX_EMULATOR */

  /* Sync with Apex emulator */
  /* Apex emulator init script sets VRC high */
  do {
    HW_Delay(100000LL);
  } while( !(pHI_ApexReg->GloIntBusLock & HI_BIT_GIBL_VRC) );

  /* Then Apex firmware clears VRC once it starts */
  do {
    HW_Delay(100000LL);
  } while( pHI_ApexReg->GloIntBusLock & HI_BIT_GIBL_VRC );

#endif  /* APEX_EMULATOR */

  /* Set default Apex interrupt channels */
  pHI_ApexReg->InterruptEnableA = HI_BIT_INTR_ENA_RESA | HI_BIT_INTR_ENA_TTEH;
  pHI_ApexReg->InterruptEnableB = HI_BIT_INTR_ENA_RESB;

  return(OK);
}

/*---------------------------------------------------------------------------
**
** Apex_MessageProcess()
**
** Apex Unconnected Message handler.
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
**    OS_MessageProcess( );
**
**---------------------------------------------------------------------------
*/

void Apex_MessageProcess( void )
{
  GS_TYPE                  status;
  struct MESSAGE_STRUCT    msg;         /* Message router structure */
  Cb                      *msgCb;       /* Apex Cb structure */
  INT                      i;

  /* initialize the Host Cbs table */
  for(i = 0; i < NUM_HOST_CBS; i++)
    {
      host_cbs[i].rx_rsp_routine = NULL;
      host_cbs[i].pCb = pHI_ApexParam->pHostCbTable + i;
    }

  while( TRUE )
  {	
    /* Retrieve buffer address - will block if none available */
    OS_GetQueue( MSG_Q_ID, (void *)&msgCb );

    /* Track how many times we been through loop */
    apex_task_counter++;

    /* Did we get a Cb */
    if (msgCb == NULL) {
      OS_Assert(OS_ASSERT_FW_ERR);
    }
	
	switch (msgCb->command){
	
	/* Is this a request Cb */	
	case CB_CMD_TX_CMD:
	case CB_CMD_TX_REQ:	
	case CB_CMD_RX_CMD:
	case CB_CMD_RX_REQ:
	
		/* Initialize MESSAGE_STRUCT */
		msg.cb = msgCb;
		msg.extended_length = 0;
		msg.prep_size = 0;

		/* Attempt to route the message */
		if ((status = MR_MessageRouter(&msg)) != GS_SUCCESS) {
			/* Report error if can't route */
			MR_Response(status, &msg);
		}
		break;
	
	/* Is this a response to something we sent */
	case CB_CMD_RX_RSP:
	    /* find response routine */
        for ( i = 0; i < NUM_HOST_CBS; i++)
        {
            if ((GET_APEX_PTR(host_cbs[i].pCb) == msgCb) &&
                (host_cbs[i].rx_rsp_routine != NULL))
            {
                /* call the response routine */
                (host_cbs[i].rx_rsp_routine)(msgCb);

                /* clear the response routine for this Cb */
                host_cbs[i].rx_rsp_routine = NULL;

                break;
            }
        }
		break;
		
	/* Unknown cb command so return to Apex with cb error */
	default:
		msgCb->state = CB_STS_UNKNOWN_COMMAND;
		Apex_SendResp(msgCb);
		break;
	}
  }
}

/*---------------------------------------------------------------------------
**
** Apex_SendRqst( )
**
** Send message request to Apex port B
**
**---------------------------------------------------------------------------
**
** Inputs:
**    theCb   -  Request cb
**
** Outputs:
**    None
**
** Usage:
**    Apex_SendRqst(volatile Cb *theCb);
**
**---------------------------------------------------------------------------
*/

void Apex_SendRqst(volatile Cb *theCb)
{
  UDINT imask;
  INT   timeout = APEX_CB_TIMEOUT/1000;

  /* Set CB command to request */
  theCb->command = CB_CMD_TX_REQ;

  /* Lock interrupts to prevent preemption while checking apex ready */
  imask = cli();

  /* Has Apex processed the last Cb */
  /* Don't need GET_APEX_PTR when looking for NULL */
#ifndef APEX_EMULATOR
  while(pHI_ApexReg->pCommRqstCbB && timeout--)
    {
      sti(imask);
      HW_Delay(1000LL);
      imask = cli();
    }
#else
  /* If Apex emulator wait forever */
  while(pHI_ApexReg->pCommRqstCbB)
    {
      /* Bail if Apex emulator has restarted */
      if ( pHI_ApexReg->GloIntBusLock & HI_BIT_GIBL_VRC ) {
        sti(imask);
        return;
      }
      sti(imask);
      HW_Delay(1000LL);
      imask = cli();
    }
#endif

  /* We timed out waiting */
  /* TTD error or Assert ? */
  if (timeout == -1) {
    sti(imask);
    // return(ERROR);
    OS_Assert(OS_ASSERT_FW_ERR);
  }

  /* Send request to Apex */
  PUT_APEX_PTR(pHI_ApexReg->pCommRqstCbB, (void *)theCb);

  /* Restore interrupts */
  sti(imask);

  return;
}

/*---------------------------------------------------------------------------
**
** Apex_SendResp( )
**
** Send message response to Apex port B
**
**---------------------------------------------------------------------------
**
** Inputs:
**    theCb   -  Response cb
**
** Outputs:
**    None
**
** Usage:
**    Apex_SendResp(volatile Cb *theCb);
**
**---------------------------------------------------------------------------
*/

void Apex_SendResp(volatile Cb *theCb)
{
  UDINT imask;
  INT   timeout = APEX_CB_TIMEOUT/1000;
  USINT wdog_count = 0;

  /* Set CB command to response */
  theCb->command = CB_CMD_TX_RSP;

  /* Lock interrupts to prevent preemption while checking apex ready */
  imask = cli();

  /* Has Apex processed the last Cb */
  /* Don't need GET_APEX_PTR when looking for NULL */
#ifndef APEX_EMULATOR
  while(pHI_ApexReg->pCommRespCbB && timeout--)
    {
      sti(imask);
      HW_Delay(1000LL);
      imask = cli();
    }
#else
  /* If Apex emulator wait forever */
  while(pHI_ApexReg->pCommRespCbB)
    {
      /* Bail if Apex emulator has restarted */
      if ( pHI_ApexReg->GloIntBusLock & HI_BIT_GIBL_VRC ) {
        sti(imask);
        return;
      }
      sti(imask);
      HW_Delay(1000LL);
      imask = cli();
    }
#endif

  /* We timed out waiting */
  /* TTD error or Assert ? */
  if (timeout == -1) {
    sti(imask);
    // return(ERROR);
    OS_Assert(OS_ASSERT_FW_ERR);
  }

  /* Send response to Apex */
  PUT_APEX_PTR(pHI_ApexReg->pCommRespCbB, (void *)theCb);

  /* Restore interrupts */
  sti(imask);
}

/*---------------------------------------------------------------------------
**
** Apex_SendCons( )
**
** Send consumer CB to Apex port A to let Apex know that buffer is available
** for new data.
** Used when cb is configured for single buffering.
**
**---------------------------------------------------------------------------
**
** Inputs:
**    theCb   -  Consumer cb
**    timeout -  Timeout value in usec
**
** Outputs:
**    OK/ERROR - OK if success or ERROR if Apex busy
**
** Usage:
**    Apex_SendCons(volatile Cb *theCb, INT timeout);
**
**---------------------------------------------------------------------------
*/

INT Apex_SendCons(volatile Cb *theCb, INT timeout)
{
  UDINT imask_working,imask_reschedule;

  /* Make sure we check at least once */
  if (timeout <= 0) {
    timeout = 1;
  }

  /* Lock interrupts to prevent preemption while checking apex ready */
  imask_working = cli();

  /* Save reschedule level and don't allow reschedule while waiting for Apex */
  imask_reschedule = imask_working & OS_RESCHEDULE_EVT;
  imask_working &= ~OS_RESCHEDULE_EVT;

  /* Has Apex processed the last Cb */
#ifndef APEX_EMULATOR
  while(pHI_ApexReg->pCommConsCbA && timeout--)
    {
      sti(imask_working);       /* Enable all but reschedule events */
      HW_Delay(1000LL);
      imask_working = cli();
    }
#else
  /* If Apex emulator wait forever */
  while(pHI_ApexReg->pCommConsCbA)
    {
      /* Bail if Apex emulator has restarted */
      if ( pHI_ApexReg->GloIntBusLock & HI_BIT_GIBL_VRC ) {
        sti(imask_working | imask_reschedule);
        return(OK);
      }
      sti(imask_working);       /* Enable all but reschedule events */
      HW_Delay(1000LL);
      imask_working = cli();
    }
#endif

  /* We timed out waiting */
  if (timeout == -1) {
    sti(imask_working | imask_reschedule);
    return(ERROR);
  }

  /* Send consumer to Apex */
  PUT_APEX_PTR(pHI_ApexReg->pCommConsCbA, (void *)theCb);

  /* Restore interrupts */
  sti(imask_working | imask_reschedule);

  return(OK);
}

/*---------------------------------------------------------------------------
**
** Apex_SendProd( )
**
** Send producer CB to Apex port A.
** Triggers Apex to send any CB_CMD_WRITE CBs that are linked to this
** producer.
**
**---------------------------------------------------------------------------
**
** Inputs:
**    theCb   -  Producer cb
**    timeout -  Timeout value in usec
**
** Outputs:
**    OK/ERROR - OK if success or ERROR if Apex busy
**
** Usage:
**    Apex_SendProd(volatile Cb *theCb, INT trigger);
**
**---------------------------------------------------------------------------
*/

INT Apex_SendProd(volatile Cb *theCb, INT timeout)
{
  UDINT imask_working,imask_reschedule;

  /* Make sure we check at least once */
  if (timeout <= 0) {
    timeout = 1;
  }

  /* Lock interrupts to prevent preemption while checking apex ready */
  imask_working = cli();

  /* Save reschedule level and don't allow reschedule while waiting for Apex */
  imask_reschedule = imask_working & OS_RESCHEDULE_EVT;
  imask_working &= ~OS_RESCHEDULE_EVT;

  /* Has Apex processed the last Cb */
#ifndef APEX_EMULATOR
  while(pHI_ApexReg->pCommProdCbA && timeout--)
    {
      sti(imask_working);       /* Allow all but reschedule events */
      HW_Delay(1000LL);
      imask_working = cli();
    }
#else
  /* If Apex emulator wait forever */
  while(pHI_ApexReg->pCommProdCbA)
    {
      /* Bail if Apex emulator has restarted */
      if ( pHI_ApexReg->GloIntBusLock & HI_BIT_GIBL_VRC ) {
        sti(imask_working | imask_reschedule);
        return(OK);
      }
      sti(imask_working);       /* Allow all but reschedule events */
      HW_Delay(1000LL);
      imask_working = cli();
    }
#endif

  /* We timed out waiting */
  if (timeout == -1) {
    sti(imask_working | imask_reschedule);
    return(ERROR);
  }

  /* Send producer to Apex */
  PUT_APEX_PTR(pHI_ApexReg->pCommProdCbA, (void *)theCb);

  /* Restore interrupts */
  sti(imask_working | imask_reschedule);

  return(OK);
}

/*---------------------------------------------------------------------------
**
** Apex_CbaIsr( )
**
** Apex channel A interrupt service routine.
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
**    Apex_ChaIsr( void );
**
**---------------------------------------------------------------------------
*/
#ifdef  APEX_REENTRANT
EX_REENTRANT_HANDLER( Apex_CbaIsr )
#else
EX_INTERRUPT_HANDLER( Apex_CbaIsr )
#endif
{
  INT vector;

  /* Apex fault can pull interrupt so first check fault line */
  if ( *pPORTGIO & APEX_FAULT ) {
    OS_Assert(OS_ASSERT_APEXFAULT);
  }

  vector = pHI_ApexReg->CbIrqPrioA >> 2;

  if ( vector >= HI_NUM_CB_IRQ_VEC ) {
    OS_Assert(OS_ASSERT_HW_ERR);
  }

  /* Call interrupt vector routine */
  (*apex_cba_vectors[vector])();
}

/*---------------------------------------------------------------------------
**
** apex_cba_resp( )
**
** Handles realtime connection related events by calling the ResponseRoutine
** registered with the CB.
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
**    apex_cba_resp( void );
**
**---------------------------------------------------------------------------
*/

void apex_cba_resp(void)
{
  Cb        *theCb;
  pFuncRsp_t responseRoutine;

  /* Get new cb from Apex */
  theCb = GET_APEX_PTR(pHI_ApexReg->pResponseCbA);

  /* If valid then process it */
  if (theCb != (Cb *)APEX_NULL) {

    /* Perform callback */
    responseRoutine = theCb->responseRoutine;
    if (responseRoutine) {
      (responseRoutine)(theCb);
    }

#if     1       /* TTD responseRoutine == NULL should we assert or ignore ? */
    else {
      OS_Assert(OS_ASSERT_FW_ERR);
    }
#endif

    /* Acknowledge the Apex interrupt */
    // PUT_APEX_PTR(pHI_ApexReg->pResponseCbB, APEX_NULL);
    pHI_ApexReg->pResponseCbA = NULL;
    ssync();
  }

#if     0       /* TTD theCb == NULL should we assert or ignore ? */
  else {
    OS_Assert(OS_ASSERT_FW_ERR);
  }
#endif
}

/*---------------------------------------------------------------------------
**
** Apex_CbbIsr( )
**
** Apex channel B interrupt service routine.
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
**    Apex_ChbIsr( void );
**
**---------------------------------------------------------------------------
*/

#ifdef  APEX_REENTRANT
EX_REENTRANT_HANDLER( Apex_CbbIsr )
#else
EX_INTERRUPT_HANDLER( Apex_CbbIsr )
#endif
{
  INT vector;

  /* Apex fault can pull interrupt so first check fault line */
  if ( *pPORTGIO & APEX_FAULT ) {
    OS_Assert(OS_ASSERT_APEXFAULT);
  }

  vector = pHI_ApexReg->CbIrqPrioB >> 2;

  if ( vector >= HI_NUM_CB_IRQ_VEC ) {
    OS_Assert(OS_ASSERT_HW_ERR);
  }

  /* Call interrupt vector routine */
  (*apex_cbb_vectors[vector])();
}

/*---------------------------------------------------------------------------
**
** apex_cbb_resp( )
**
** Handles realtime connection related events by calling the ResponseRoutine
** registered with the CB.
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
**    apex_cbb_resp( void );
**
**---------------------------------------------------------------------------
*/

void apex_cbb_resp(void)
{
  Cb *theCb;

  /* TTD
   * What to do with RESP if can't put it on the queue.
   *  May be able to optimize error recovery
   *    If the MSG_Q_SIZE is set to greater than the number
   *    of free, maint and host cbs, then in theory we cannot
   *    overflow.
   */

  /* Get new cb from Apex */
  theCb = GET_APEX_PTR(pHI_ApexReg->pResponseCbB);

  /* If valid then process it */
  if (theCb != (Cb *)APEX_NULL) {

    /* Send cb to message task */
    if (OS_PutQueue(MSG_Q_ID, (void *)theCb) != OK) {

      /* Can't forward so reject it with a busy. */
      theCb->state = CB_STS_BUSY;
      Apex_SendResp(theCb);
    }

    /* Acknowledge the Apex interrupt */
    // PUT_APEX_PTR(pHI_ApexReg->pResponseCbB, APEX_NULL);

    pHI_ApexReg->pResponseCbB = NULL;
    ssync();
  }

#if     0   /* TTD should we assert or ignore NULL CB ? */
  else {
    OS_Assert(OS_ASSERT_FW_ERR);
  }
#endif
}

/*---------------------------------------------------------------------------
**
** Apex_InitLock( )
**
** Initialize Apex/Host firmware based lock.
** Should only by called by Apex OR Host depending on who initializes the
** the structure that is being protected by the lock.
**
**---------------------------------------------------------------------------
**
** Inputs:
**    lock  -  pointer to firmware lock structure
**
** Outputs:
**    None
**
** Usage:
**    Apex_InitLock(volatile LOCK *lock)
**
**---------------------------------------------------------------------------
*/

void Apex_InitLock(volatile LOCK *lock)
{
  lock->flag[HOST_LOCK] = FALSE;
  lock->flag[APEX_LOCK] = FALSE;
  lock->turn = 0;
  ssync();
}

/*---------------------------------------------------------------------------
**
** Apex_Lock( )
**
** Get firmware based lock.
** Does not return until lock acquired.
** User is responsable for disabling interrupts if needed,
**
**---------------------------------------------------------------------------
**
** Inputs:
**    lock  -  pointer to firmware lock structure
**
** Outputs:
**    None
**
** Usage:
**    Apex_Lock(volatile LOCK *lock)
**
**---------------------------------------------------------------------------
*/

void Apex_Lock(volatile LOCK *lock)
{
  lock->flag[LOCK_MINE] = TRUE;
  lock->turn = LOCK_OTHER;
  ssync();
  while((lock->flag[LOCK_OTHER] == TRUE) && (lock->turn == LOCK_OTHER)) {}
}

/*---------------------------------------------------------------------------
**
** Apex_TryLock( )
**
** Attemp firmware based lock.
** Return true if lock acquired.
** User is responsable for disabling interrupts if needed,
**
**---------------------------------------------------------------------------
**
** Inputs:
**    lock   -   pointer to firmware lock structure
**
** Outputs:
**    true/false true if loack acquired
**
** Usage:
**    Apex_TryLock(volatile LOCK *lock)
**
**---------------------------------------------------------------------------
*/

bool Apex_TryLock(volatile LOCK *lock)
{
  lock->flag[LOCK_MINE] = TRUE;
  lock->turn = LOCK_OTHER;
  ssync();
  if ((lock->flag[LOCK_OTHER] == TRUE) && (lock->turn == LOCK_OTHER)) {
    lock->flag[LOCK_MINE] = 0;
    return(false);
  }
  return(true);
}

/*---------------------------------------------------------------------------
**
** Apex_UnLock( )
**
** Release firmware based lock.
** User is responsable for disabling interrupts if needed,
**
**---------------------------------------------------------------------------
**
** Inputs:
**    lock  -  pointer to firmware lock structure
**
** Outputs:
**    None
**
** Usage:
**    Apex_UnLock(volatile LOCK *lock)
**
**---------------------------------------------------------------------------
*/

void Apex_Unlock(volatile LOCK *lock)
{
  lock->flag[LOCK_MINE] = 0;
}

/*---------------------------------------------------------------------------
**
** Apex_WatchdogInit( )
**
** Initialize Apex/DSP Watchdogs
**
**---------------------------------------------------------------------------
**
** Inputs:
**    watchdog   - Watchdog structure
**
** Outputs:
**    None
**
** Usage:
**    Apex_WatchdogInit(watchdog_t watchdog)
**
**---------------------------------------------------------------------------
*/

void Apex_WatchdogInit(volatile watchdog_t *watchdog)
{
#ifdef  RELEASE_BUILD
  PUT_APEX_PTR(pHI_ApexParam->pWatchdog, watchdog);
  HW_WatchdogInit(US_TO_SCLK(APEX_WDOG_TIMEOUT));
#endif
}

/*---------------------------------------------------------------------------
**
** Apex_WatchdogKick( )
**
** Kick Apex/DSP Watchdog
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
**    Apex_WatchdogKick(void)
**
**---------------------------------------------------------------------------
*/
extern "C"
void Apex_WatchdogKick(void)
{
  UDINT       imask;
  ULINT       current_dsp_time;
  watchdog_t *watchdog;
  watchdog_t  working;

  /* Only kick watchdog if it is enabled */
  if ( (watchdog = GET_APEX_PTR(pHI_ApexParam->pWatchdog)) != (watchdog_t *)APEX_NULL ) {

    /* First check if Apex faulted */
    if ( *pPORTGIO & APEX_FAULT ) {
      OS_Assert(OS_ASSERT_APEXFAULT);
    }

    /* Make call thread safe */
    imask = cli();
    {
      /* Calc max time between kicks */
      _GET_CYCLE_COUNT(current_dsp_time);
      if ( apex_watchdog_time && ((current_dsp_time - apex_watchdog_time) > max_watchdog_time) ) {
        max_watchdog_time = current_dsp_time - apex_watchdog_time;
      }
      apex_watchdog_time = current_dsp_time;

      /* Get watchdog elements as a whole to reduce Apex access */
      working = *watchdog;

      /* Did Apex echo watchdog value */
      if ( working.parts.host == working.parts.asic ) {

        /* Kick Blackfin watchdog */
        HW_WatchdogKick();

        /* Tell Apex we did our part */
        watchdog->parts.host = working.parts.host + 1;
      }
    }
    sti(imask);

  }

  /* If not enabled then keep track of maximum time between calls (for debuging) */
  else {

#ifdef  RELEASE_BUILD
    /* Watchdog not allowed to be disabled for RELEASE_BUILDs */
    OS_Assert(OS_ASSERT_WDOG);
#endif

    /* Make call thread safe */
    imask = cli();
    {
      /* Calc max time between kicks */
      _GET_CYCLE_COUNT(current_dsp_time);
      if ( apex_watchdog_time && ((current_dsp_time - apex_watchdog_time) > max_watchdog_time) ) {
        max_watchdog_time = current_dsp_time - apex_watchdog_time;
      }
      apex_watchdog_time = current_dsp_time;
    }
    sti(imask);
  }

  /* Monitor for manufactureing block lock timeout  */
  Nvs_MfgLockCheck();
}

/*---------------------------------------------------------------------------
**
** apex_PreTest( )
**
** Perform pretest of Apex interface before we send it boot code.
**
**---------------------------------------------------------------------------
**
** Inputs:
**    None
**
** Outputs:
**    OK/ERROR
**
** Usage:
**    apex_PreTest( void );
**
**---------------------------------------------------------------------------
*/

INT apex_PreTest(void)
{

  /* Make sure Apex is right hardware rev */
  if ( (((pHI_ApexReg->AsicCtrl & HI_MASK_ASIC_MAJOR_REV) >> HI_VAL_ASIC_MAJOR_REV_SHIFT) + 1) != APEX_HW_MAJOR_REV ) {
    return(ERROR);
  }
  /* Test registers */
  if (apex_RegisterTest()) {
    return(ERROR);
  }
  /* Test Apex internal memory */
  if (apex_MemoryTest()) {
    return(ERROR);
  }
  /* Test Apex channel A and B interrupts */
  if (apex_InterruptTest()) {
    return(ERROR);
  }

  return(OK);
}

/*---------------------------------------------------------------------------
**
** Apex pretests
**
** Various Apex interface pretest before we send it boot code.
**   Register test.
**   Internal memory test.
**   Interrupt test.
**
**---------------------------------------------------------------------------
**
** Inputs:
**    None
**
** Outputs:
**    OK/ERROR
**
** Usage:
**    apex_RegisterTest(void);
**    apex_MemoryTest(void);
**    apex_InterruptTest(void)
**
**---------------------------------------------------------------------------
*/

/* Write/Read Apex SystemTimeHigh */
INT apex_RegisterTest(void)
{
  pHI_ApexReg->SystemTimeHigh = 0x55555555L;
  if (pHI_ApexReg->SystemTimeHigh != 0x55555555L) {
    return(ERROR);
  }
  pHI_ApexReg->SystemTimeHigh = 0xAAAAAAAAL;
  if (pHI_ApexReg->SystemTimeHigh != 0xAAAAAAAAL) {
    return(ERROR);
  }
  pHI_ApexReg->SystemTimeHigh = 0;
  if (pHI_ApexReg->SystemTimeHigh != 0) {
    return(ERROR);
  }
  return(OK);
}

/* Perform walking 1's test on Apex internal memory */
INT apex_MemoryTest(void)
{
  volatile USINT *ptr;
  USINT temp;
  int size;
  USINT lastValue = 0;
  USINT nextValue = 1;

  ptr = (USINT *)APEX_RAM;
  for(size = 0; size < APEX_RAM_SIZE; size++)
    {
      *ptr++ = lastValue;
    }
  do
    {
      ptr = (USINT *)APEX_RAM;
      size = APEX_RAM_SIZE;
      while(size--)
        {
          temp = *ptr;
          if(temp != lastValue) {
            return(ERROR);
          }
          *ptr++ = nextValue;
        }

      lastValue = nextValue;

      if(nextValue & 0x80) {
        nextValue = nextValue << 1;
      }
      else {
        nextValue = (nextValue << 1) | 1;
      }
    } while(lastValue);

  return(0);
}

/* Toggle channel A and channel B interrupts */
INT apex_InterruptTest(void)
{
  /* Enable channel A and B interrupt lines on Apex */
  pHI_ApexReg->InterruptEnableA = HI_BIT_INTR_ENA_RESA;
  pHI_ApexReg->InterruptEnableB = HI_BIT_INTR_ENA_RESB;

  /* Is either line already set */
  if ( *pPORTGIO & (APEX_CBB_INT | APEX_CBA_INT) ) {
    return(ERROR);
  }

  /* Set channel A interrupt */
  pHI_ApexReg->pResponseCbA = (Cb *)0xaaaa;
  ssync();
  HW_Delay(1000LL);
  if ( (*pPORTGIO & (APEX_CBB_INT | APEX_CBA_INT)) != APEX_CBA_INT ) {
    return(ERROR);
  }

  /* Reset channel A interrupt */
  pHI_ApexReg->pResponseCbA = NULL;
  ssync();
  HW_Delay(1000LL);
  if ( (*pPORTGIO & (APEX_CBB_INT | APEX_CBA_INT)) != 0 ) {
    return(ERROR);
  }

  /* Set channel B interrupt */
  pHI_ApexReg->pResponseCbB = (Cb *)0xaaaa;
  ssync();
  HW_Delay(1000LL);
  if ( (*pPORTGIO & (APEX_CBB_INT | APEX_CBA_INT)) != APEX_CBB_INT ) {
    return(ERROR);
  }

  /* Reset channel B interrupt */
  pHI_ApexReg->pResponseCbB = NULL;
  ssync();
  HW_Delay(1000LL);
  if ( (*pPORTGIO & (APEX_CBB_INT | APEX_CBA_INT)) != 0 ) {
    return(ERROR);
  }

  /* Turn interrupt lines back off */
  pHI_ApexReg->InterruptEnableA = 0;
  pHI_ApexReg->InterruptEnableB = 0;

  return(OK);
}

/*---------------------------------------------------------------------------
**
** apex_PostTest( )
**
** Perform post test of Apex interface after it begins executing.
**
**---------------------------------------------------------------------------
**
** Inputs:
**    None
**
** Outputs:
**    OK/ERROR
**
** Usage:
**    apex_PostTest( void );
**
**---------------------------------------------------------------------------
*/

INT apex_PostTest(void)
{

  /* Right now just make sure Apex Firmware revision is what we expect */
  if ( (pHI_ApexParam->Revision.major != APEX_FW_MAJOR_REV) ||
       (pHI_ApexParam->Revision.minor != APEX_FW_MINOR_REV) ) {
    return(ERROR);
  }

  /* TTD
   *  cb test interface A and B
   *  ???
   */
  return(OK);
}

/*---------------------------------------------------------------------------
**
** Apex dummy soft vector
**
** Force Assert on unintialized Apex interrupt vector
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
**    apex_dummy_vector(void)
**
**---------------------------------------------------------------------------
*/

void apex_dummy_vector(void)
{
  OS_Assert(OS_ASSERT_HW_ERR);
}

/*---------------------------------------------------------------------------
**
** Apex_GetHostCb()
**
** Find and return a pointer to a Host Cb
**
**---------------------------------------------------------------------------
**
** Inputs:
**    pointer to a function for the response
**
** Outputs:
**    pointer to a Host Cb if available otherwise NULL.
**
** Usage:
**    Apex_GetHostCb( );
**
**---------------------------------------------------------------------------
*/

Cb * Apex_GetHostCb( pFuncRsp_t rx_rsp_routine )
{
    INT   i;
    static INT   rotate_cbs;

    /* Find a free Host Cb (has no response routine defined) */
    for ( i = 0; i < NUM_HOST_CBS; i++)
    {
        if (++rotate_cbs >= NUM_HOST_CBS)
        {
            rotate_cbs = 0;
        }
        if (host_cbs[rotate_cbs].rx_rsp_routine == NULL)
        {
            /* assign a response routine for this Cb */
            host_cbs[rotate_cbs].rx_rsp_routine = rx_rsp_routine;
            /* get the pointer to the host Cb data */
            return ( GET_APEX_PTR(host_cbs[rotate_cbs].pCb) );
        }
    }
    return ( (Cb *)APEX_NULL );
}

/*---------------------------------------------------------------------------
**
** Apex_GetTime{}
**
** Get 64 bit apex system time
**
**---------------------------------------------------------------------------
**
** Inputs:
**    None.
**
** Outputs:
**    time      - 64 bit apex time
**
** Usage:
**    Apex_GetTime( void )
**
**---------------------------------------------------------------------------
*/

extern "C" ULINT Apex_GetTime( void )
{
  UDINT apex_time_low;

  /* Get low 32 bit time directly from Apex */
  apex_time_low = pHI_ApexReg->SystemTime;

  /* Get high 32 bits from cached value */

  /* Did low word rollover recently */
  if ( apex_time_low < (ULONG_MAX / 4) ) {

    /* We may have not bumped the high word yet so use last value + 1 */
    return ((((ULINT)last_apex_time_high_cache + 1) << 32) | (ULINT)apex_time_low);
  }

  /* Else if not near rollover then apex_time_high is up to date */
  else {
    return (((ULINT)apex_time_high_cache << 32) | (ULINT)apex_time_low);
  }
}

/*---------------------------------------------------------------------------
**
** Apex_UpdateTime( )
**
** Done as backgrond task to update Apex time rollover detect.
** Must be called from background task at least every 1073 seconds but is normally
** called from APP_UpdateCST().
**
**   Apex SystemTime register is a 32 bit register that increments every 1usec
**   and rolls over every 4294.967295 seconds.
**   Since Apex accesses are expensive we break this time into 4 equal sections
**   to minimize Apex_GetTime() time.
**
**      Section 1 (0 - 1073 seconds)
**        Apex_UpdateTime() Keeps apex_time_high_cache up to date
**        Apex_GetTime      Uses last_apex_time_high_cache + 1
**
**      Section 2 (1073 - 2147 seconds)
**        Apex_UpdateTime() Keeps apex_time_high_cache up to date
**        Apex_GetTime      Starts using apex_time_high_cache
**
**      Section 3 (2147 - 3221 seconds)
**        Apex_UpdateTime() Keeps apex_time_high_cache up to date and
**                          updates last_apex_time_high_cache in preperation for next rollover
**        Apex_GetTime      Still using apex_time_high_cache
**
**      Section 4 (3221 - 4294 seconds)
**        Apex_UpdateTime() Goes back to just keeping apex_time_high_cache up to date
**        Apex_GetTime      Keeps using apex_time_high_cache
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
**    Apex_UpdateTime( void )
**
**---------------------------------------------------------------------------
*/

void Apex_UpdateTime( void )
{
  UDINT apex_time_low;

  /* Keep apex_time_high_cache up to date */
  apex_time_high_cache = pHI_ApexReg->SystemTimeHigh;

  /* Get low word of system time from Apex */
  apex_time_low = pHI_ApexReg->SystemTime;

  /* If no where near rollover then apex_time_high_cache is up to date */
  if ( (apex_time_low > (ULONG_MAX / 2)) &&
       (apex_time_low < (3 * (ULONG_MAX / 4))) ) {
    last_apex_time_high_cache = apex_time_high_cache;
  }
}

/*---------------------------------------------------------------------------
**
** Apex_AlarmInit{}
**
** This module initializes an alarm record.
**
**---------------------------------------------------------------------------
**
** Inputs:
**    alarm     - The address of the alarm record to be 
**
** Outputs:
**    None
**
** Usage:
**    Apex_AlarmInit(APEX_ALARM_REC *alarm)
**
**---------------------------------------------------------------------------
*/

void Apex_AlarmInit(APEX_ALARM_REC *alarm)
{
  /* Initialize the alarm record.*/
  alarm->next = alarm->prev = NULL;
  alarm->expiration_time    = 0;
  alarm->alarm_routine      = NULL;
  alarm->parm               = NULL;
}

/*---------------------------------------------------------------------------
**
** Apex_AlarmSet( )
**
** This routine is used to add an alarm request to a list of alarm
** requests.  By using an alarm request list, the availability of multiple
** multiple can be simulated.
**
**      RESTRICTIONS ON ALARM ROUTINES
**
**       1.  The routine to be executed on alarm expiration must conform
**           to the following synopsis:
**
**              void alarm_routine(void *parm);
**
**       2.  The specified routine should be one#if (ALARM_DEBUG == 1)
 that executes very
**           quickly as it will be invoked from an interrupt service
**           routine.
**
**       3.  The specified routine should contain no OS calls that
**           are not allowed from interrupt service routines.
**
** The alarm list is a doubly linked, circular list containing a sequence
** of alarm interrupt requests ordered on alarm expiration time.  After
** execution of this routine, the alarm list will have the following
** structure.
**
**                   +----------------+
**                   |                |
**                   V                |
**                   +-----+          |
**                   |     |          |
**    Alarm_Head     v     |          |
**    Alarm_Tail---->+-----|------+   |
**             next  |     o      |   |
**                   +------------+   |
**             prev  |     o----------+
**                   +------------+
**                   |            |
**                   |            |
**                   +------------+
**
**---------------------------------------------------------------------------
**
** Inputs:
**    alarm_time        - Apex time when the alarm is to go off (usec).
**    alarm             - The address of an alarm record to use for this alarm.
**    alarm_routine     - The address of a routine to call when the alarm goes off.
**    parm              - This pointer is pas#if (ALARM_DEBUG == 1)
sed into the alarm routine.
**
** Outputs:
**    OK/ERROR          - ERROR if time has already expired.
**
** Usage:
**    Apex_AlarmSet(ULINT alarm_time,
**                  APEX_ALARM_REC *alarm,
**                  void (*alarm_routine)(),
**                  void *parm)
**
**---------------------------------------------------------------------------
*/

INT Apex_AlarmSet(ULINT alarm_time,
                  APEX_ALARM_REC *alarm,
                  void (*alarm_routine)(void *parm),
                  void *parm)
{

#if (ALARM_DEBUG == 1)
  if ( (alarm->prev != NULL) || (alarm->next != NULL) ) {
    OS_Assert(OS_ASSERT_FW_ERR);
  }
#endif


  alarm->alarm_routine = alarm_routine;
  alarm->parm          = parm;

  return(apex_add_alarm(alarm_time, alarm));
}

/*---------------------------------------------------------------------------
**
** Apex_AlarmReset()
**
** This routine is used to reset an alarm which already resides on the
** alarm list but has not yet expired.  The alarm is first removed from the 
** alarm list and then is added back to the alarm list in it's new location.
** If necessary, the APEX alarm interrupt is reprogrammed.
**
**---------------------------------------------------------------------------
**
** Inputs:
**    alarm_time        - Apex time when the alarm is to go off (usec).
**    alarm             - The address of the alarm record to reset.
**
** Outputs:
**    OK/ERROR          - ERROR if time has already expired.
**
** Usage:
**    Apex_AlarmReset(ULINT alarm_time, APEX_ALARM_REC *alarm)
**
**---------------------------------------------------------------------------
*/

INT Apex_AlarmReset(ULINT alarm_time, APEX_ALARM_REC *alarm)
{
  UDINT imask;
  INT   error;

  /* Disable interrupts and remove the record from the list. */
  imask = cli();
  {
    /* If the alarm record is still on the list */
    if ( alarm->prev ) {
      alarm->prev->next = alarm->next;
      alarm->next->prev = alarm->prev;
      alarm->next = alarm->prev = NULL;
    }
  }
  sti(imask);

  /* Add it back in at the new time */
  return(apex_add_alarm(alarm_time, alarm));
}

/*---------------------------------------------------------------------------
**
** apex_add_alarm( )
**
** Support routine is used to add an alarm request to a list of alarm
**
**---------------------------------------------------------------------------
**
** Inputs:
**    alarm_time        - Apex time when the alarm is to go off (usec).
**    alarm             - The address of the alarm record to be started.
**
** Outputs:
**    OK/ERROR          - ERROR if time has already expired.
**
** Usage:
**    apex_add_alarm(ULINT alarm_time, APEX_ALARM_REC *alarm)
**
**---------------------------------------------------------------------------
*/

INT apex_add_alarm(ULINT alarm_time, APEX_ALARM_REC *alarm)
{

  UDINT           imask;
  APEX_ALARM_REC *list_ptr;           /* Pointer to the current alarm record      */
  APEX_ALARM_REC *tail_ptr;           /* Local copy of alarm list tail pointer    */
  ULINT           current_time;       /* The current value of the alarm counter   */

  /* Disable interrupts and read the current Apex time. */
  imask = cli();

  /* Get current Apex hardware time */
  current_time = Apex_GetTime();

  /* Is alarm_time in the future */
  if ( alarm_time > (current_time + MIN_APEX_ALARM_TIME + CCLK_TO_US(APEX_ALARM_INT_LATENCY)) ) {

    /* Locate where the new alarm belongs in the alarm list. */
    tail_ptr = Alarm_Tail;
    list_ptr = Alarm_Head->next;
    while ( (list_ptr != tail_ptr) &&
            (alarm_time >= (list_ptr->expiration_time + CCLK_TO_US(APEX_ALARM_INT_LATENCY))) ) {
      list_ptr = list_ptr->next;
    }

    /* Link the new record into the alarm list. */
    alarm->expiration_time = alarm_time - CCLK_TO_US(APEX_ALARM_INT_LATENCY);
    alarm->prev = list_ptr->prev;
    alarm->next = list_ptr;
    list_ptr->prev->next = alarm;
    list_ptr->prev = alarm;

    /*
     * If the alarm was added to the front of the alarm list, reprogram the
     * Apex ASIC alarm interrupt.
     * Put lower 32 bits in HostTargetTime and check for rollover in isr.
     */
    if ( alarm == Alarm_Head->next ) {
      pHI_ApexReg->HostTargetTime = alarm->expiration_time;
    }

    /* Enable interrupts and return.*/
#if (ALARM_DEBUG == 1)
    dbg_chk_alarm_list();
#endif

    sti(imask);
    return(OK);
  }

  /* Return error if new time is not in the future */
  else {
    sti(imask);
    return(ERROR);
  }
}

/*---------------------------------------------------------------------------
**
** Apex_AlarmCancel()
**
** This routine is used to cancel an alarm request.  Requests to cancel
** an alarm which is not on the alarm list are ignored.
**
**---------------------------------------------------------------------------
**
** Inputs:
**    alarm     - The address of the alarm record to be cancelled.
**
** Outputs:
**    None
**
** Usage:
**    Apex_AlarmCancel(APEX_ALARM_REC *alarm)
**
**---------------------------------------------------------------------------
*/

void Apex_AlarmCancel(APEX_ALARM_REC *alarm)
{
  UDINT imask;

  /* Disable interrupts and remove the record from the list.*/
  imask = cli();
  {

    /* If the alarm record is still on the list */
    if ( alarm->prev ) {
      alarm->prev->next = alarm->next;
      alarm->next->prev = alarm->prev;
      alarm->next = alarm->prev = NULL;
    }
    else if ( alarm->next != NULL ) {
      OS_Assert(OS_ASSERT_FW_ERR);
    }

    /* Enable interrupts and return.*/
#if (ALARM_DEBUG == 1)
    dbg_chk_alarm_list();
#endif
  }
  sti(imask);
}

/*---------------------------------------------------------------------------
**
** Apex_AlarmTrip()
**
** This routine is used to cause an alarm to go off immediately.  If the
** specified alarm record is still resident on the alarm list, the record is
** removed from the alarm list and the corresponding alarm routine is invoked.
**
**---------------------------------------------------------------------------
**
** Inputs:
**    alarm     - The address of the alarm record
**
** Outputs:
**    OK/ERROR  - ERROR if time has already expired.
**
** Usage:
**    Apex_trip_alarm(APEX_ALARM_REC *alarm)
**
**---------------------------------------------------------------------------
*/

void Apex_AlarmTrip(APEX_ALARM_REC *alarm)
{
  UDINT imask;

  imask = cli();

  /*
   * If the specified alarm record is still on the alarm list, remove
   * the record from the alarm list and invoke the alarm routine.
   */
  if ( alarm->prev ) {  
    alarm->prev->next = alarm->next;
    alarm->next->prev = alarm->prev;
    alarm->next = alarm->prev = NULL;

    sti(imask);
    /* Execute the alarm routine if present. */
    if ( alarm->alarm_routine ) {
      (*alarm->alarm_routine)(alarm->parm);  
    }
  }
  else {
    sti(imask);
  }
}

/*---------------------------------------------------------------------------
**
** apex_Alarm()
**
** This routine is used to service the alarm interrupt from the APEX
** ASIC.  All alarm records on the alarm list with a delta time of zero
** are removed from the alarm list and the specified alarm routines are 
** executed.  The alarm is then reprogrammed for the next alarm interval.
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
**    apex_Alarm(void)
**
**---------------------------------------------------------------------------
*/

void apex_Alarm(void)
{
  APEX_ALARM_REC *list_ptr;     /* Pointer to the current alarm list record */
  APEX_ALARM_REC *tail_ptr;     /* Local copy of alarm list tail pointer   */
  ULINT           current_time; /* The current value of the alarm counter  */

  /* Acknowledge interrupt */
  pHI_ApexReg->ExtIntPending = HI_BIT_INTR_ENA_TTEH;

  list_ptr = Alarm_Head->next;
  tail_ptr = Alarm_Tail;

  /*
   * For each entry on the alarm list that has expired, unlink the alarm
   * record from the alarm list and execute the routine specified in the 
   * alarm record.
   */
  while (list_ptr != tail_ptr) {

    /* Get current Apex hardware time */
    current_time = Apex_GetTime();

    if ( (current_time + MIN_APEX_ALARM_TIME) >= list_ptr->expiration_time ) {

      /* Remove the alarm record from the alarm list */
      Alarm_Head->next     = list_ptr->next;
      list_ptr->next->prev = Alarm_Head;
      list_ptr->prev       = list_ptr->next = NULL;

      /* Execute the alarm routine if present. */
      if ( list_ptr->alarm_routine ) {
        (*list_ptr->alarm_routine)(list_ptr->parm);  
      }

      list_ptr = Alarm_Head->next;

#if (ALARM_DEBUG == 1)
      dbg_chk_alarm_list();
#endif
    }
    else
      break;
  }

  /* Make sure interrupt has really been acked before we exit */
  ssync();

  /* Reprogram the alarm interrupt for the next interval if needed */
  list_ptr = Alarm_Head->next;
  if (list_ptr != tail_ptr) {
    pHI_ApexReg->HostTargetTime = list_ptr->expiration_time;
  }
}

/*******************************************************************************
* dbg_chk_alarm_list  -  Check Alarm List.
*
* This is a debug routine designed to verify the consistency of the ASIC
* alarm list.
*******************************************************************************/
void dbg_chk_alarm_list(void)
{
  APEX_ALARM_REC *prev_ptr;
  APEX_ALARM_REC *list_ptr;

  list_ptr = Alarm_Head;
  do {
    prev_ptr = list_ptr;
    list_ptr = list_ptr->next;
    if ( list_ptr->prev != prev_ptr )
      OS_Assert(OS_ASSERT_FW_ERR);
  } while ( list_ptr != Alarm_Tail );
}
