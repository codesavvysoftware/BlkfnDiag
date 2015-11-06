/****************************************************************************
*****************************************************************************
**
** File Name
** ---------
**
** User.c
**
*****************************************************************************
*****************************************************************************
**
** Description
** -----------
**
** User initilization routines
**
** Functions heavily customized by the user to initialize anything on the 
** module such as hardware etc.  Also the Null Task which should be customized
** by the user to perform any needed background tasks.
**
** Function List
**  1. User_Init( )
**  2. User_NullTask( )
**  3. User_ValidateKey( )
**  4. User_Reset( )
**  5. User_NvsUpdate( )
**
*****************************************************************************
*****************************************************************************
**
** Source Change Indices
** ---------------------
**
** Porting: <none>-0---<major>         Customization: <none>----0<major>
**
*****************************************************************************
*****************************************************************************
**                                                                         **
** I/O TOOLKIT EXAMPLE CODE                                                **
** COPYRIGHT (c) 2011 Rockwell Automation Inc.                             **
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
**     Rev 1.00   20 Feb 2011 DDM  Created from SOE
**
*****************************************************************************
*****************************************************************************
*/

#include "Defs.h"
#include "Mr.h"
#include "App.h"
#include "Os_iotk.h"
#include "Asm_Obj.h"
#include "AIG_Obj.h"
#include "Id_Obj.h"
#include "Nvs_Obj.h"
#include "IF8I.h"
#include "AIP_Obj.h"
#include "adcdef.h"

/* Include Build Block after product information has been defined */
#include "Bld_Block.h"

extern "C" void RunDiagnostics();

/*---------------------------------------------------------------------------
**
** Public Data
**
**---------------------------------------------------------------------------
*/

/*---------------------------------------------------------------------------
**
** Private Data
**
**---------------------------------------------------------------------------
*/

UDINT null_counter = 0;         /* Records how often NULL task runs */
/* Keep track of maximum time between null task loops */
ULINT   max_null_dsp_cycles = 0;

BOOL    waiting_on_erase = FALSE;      /* Flag we have an ongoing Flash erase */

/*---------------------------------------------------------------------------
**
** Private Functions
**
**---------------------------------------------------------------------------
*/

#ifdef  UNIT_TEST
void UnitTestMain(void);
void UnitTestTask(void);
#endif
extern INT nvs_FlashDelayEraseSector( UINT *start_address, UDINT size );
extern INT nvs_FlashWrite( UINT *start_address, UINT *data, UDINT data_size, bool kick_wd );
extern void if8i_Write_ADCs( UDINT write_out[IF8I_NUM_CH], UDINT write_len );


/*---------------------------------------------------------------------------
**
** OS_UserInit( )
**
** User initialization routine from which all user adder module initilization
** routines may be launched.  Called from Main( ).  Returns OK.
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
**    User_Init( );
**
**---------------------------------------------------------------------------
*/

INT User_Init(void)
{
  INT error = 0;  /* Error flag for initialization calls */
  INT i;

  /*
   * Set up application tasks
   */

  /* Initialize input task queue */
  error |= OS_CreateQueue(INPUT_Q_ID, INPUT_Q_SIZE);

  /* Create IF8I Input Task */
  error |= OS_CreateTask( INPUT_ID, INPUT_PRIORITY, INPUT_STACK_SIZE, IF8I_InputTask );

  /*
   * Fill in apex parameter table.
   */

  /* Set Led Display type */
  pHI_ApexParam->LedDisplay.analog.type = LED_DISPLAY_TYPE_ANALOG;

  /*
    Enable DC-DC conversion oscillator and configure it but don't start it because
    the APEX has to be running to start it.
  */
#define APEX_DCtoDC_Cfg    *((unsigned int *)(APEX_BASE_ADDR+0x4400))
  pHI_ApexReg->GpioFunction = 0x0040;   /* Enable DC-DC */
  APEX_DCtoDC_Cfg = 0x0021;             /* Set frequency to 125KHz, 1us deadband */
  
  /*
   * Initialize Objects
   */

  error |= MR_Init( pHI_ApexIoMem->classes );
  error |= Id_ObjInit( PRODUCT_MAJOR_REV, PRODUCT_MINOR_REV, PRODUCT_SUB_MINOR_REV );
  error |= Asm_ObjInit( );
  error |= Nvs_ObjInit( );
  error |= AIG_ObjInit( );
  error |= AIP_ObjInit( );
  return( error );
}

/*---------------------------------------------------------------------------
**
** User_NullProcess()
**
** Null process which always executes as a background task and is lowest
** priority of all processes.
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
**    User_NullProcess( );
**
**---------------------------------------------------------------------------
*/
void User_NullProcess( void )
{
   BOOL           send_lock;            /* Flag we need to do a Source Lock */
   INT            i;                    /* Loop counter */
   UDINT          adc_data[ IF8I_NUM_CH ];
   LINT           current_dsp_cycles;   /* Diagnostic timestamp */
   LINT           last_cst_update;      /* */
   static ULINT   last_dsp_cycles;
   static UINT    last_rolling;         /* Last rolling timestamp - ensure we only run Over Source on new samples */
   volatile UINT *working_addr;         /* Flash Working address for waiting on Flash burn to finish */
   extern BOOL    new_flash_pending;    /* Flag we need to do a Cal Flash in the background task */
   extern DINT    over_current_timer_expired;     /* How many RTS for timer to expire */
   extern UDINT   rts;                            /* Requested RTS in us */

   extern ADC_CHAN_CFG ADC_config[ IF8I_NUM_CH ];


   /* Init times */
   _GET_CYCLE_COUNT( last_dsp_cycles );
   last_cst_update = last_dsp_cycles;
   
#ifdef  UNIT_TEST
   UnitTestMain();       /* Run standalone unit tests */
#endif

   /* Turns on IMASK bit for all system interrupts */
   sti( OS_INTERRUPTS );

   /* Selftest should be done by the time we get here so set initial state */
   Id_SetState(ID_STATE_NO_CONNS);

   /* Kick scheduler - all tasks will execute & suspend or wait on message Q */
   /* until Null task is scheduled and we return here */
   RESCHEDULE( );
   
   /* ARM Apex/Blackfin watchdog - only enables if Release Build based on ifdef */
   Apex_WatchdogInit( &pHI_ApexIoMem->watchdog );
   Apex_WatchdogKick( );

   /* Null Task loop which runs as the background task when no other task needs to execute */
   while( TRUE )
   {

#ifdef  UNIT_TEST
      UnitTestTask();     /* Run task level unit tests */
#endif

      /* Grab copy of current dsp cycles */
      _GET_CYCLE_COUNT(current_dsp_cycles);

      send_lock = FALSE;
      
      /* Calculate max time between null loops */
      if ( ( ( current_dsp_cycles - last_dsp_cycles ) > max_null_dsp_cycles ) && null_counter )
      {
         max_null_dsp_cycles = current_dsp_cycles - last_dsp_cycles;
      }
      last_dsp_cycles = current_dsp_cycles;
      
      /* Keep track of number of times null task has run */
      null_counter++;
    
      /* Time to update CST and Time Sync data */
      if ( ( current_dsp_cycles - last_cst_update ) > US_TO_CCLK( APP_CST_POLL_RATE ) )
      {
         /* Kick Apex watchdog */
         Apex_WatchdogKick( );
        
         /* Update local copies of CST and Time Sync data */
         APP_UpdateCST( );
         APP_UpdateTimeSyncData( );
         last_cst_update = current_dsp_cycles;
      }

            
      /* Delayed Write to Flash? */
      /* Check if we need to store off the new InputRange/NotchFilter settings */
      if ( new_flash_pending )
      {
         if ( ! waiting_on_erase )
         {
            /* Now erase Cal data (configuration segment) and burn the new values. */
            if ( nvs_FlashDelayEraseSector( (UINT *)CAL_ADDRESS, 0x10000 ) != OK )
            {
               OS_Assert( OS_ASSERT_HW_ERR );
            }
            waiting_on_erase = TRUE;
         }
         /* Go back to the beginning and wait for erase to complete */
         working_addr = (volatile UINT *)( CAL_ADDRESS & FLASH_ADDR_MASK_NO_CARE );
         if ( *working_addr == 0xffff )
         {
            if ( nvs_FlashWrite( (UINT *)CAL_ADDRESS, (UINT *)&cal_flash, sizeof( cal_flash ), TRUE ) != OK )
            {
               OS_Assert( OS_ASSERT_HW_ERR );
            }
            new_flash_pending = FALSE;
         }
      }
      else
      {
         /* Check if we finished a Cal recently - wait at least 1 second before clearing bit */
         /* Only check to clear the CalSuccessful bit if data is written to Flash already */
         for ( i = 0 ; i < IF8I_NUM_CH ; i++ )
         {
            if ( if8i_i.Ch[ i ].CalSuccessful )
            {
               if ( ( if8i_i.timestamp - cal_flash.last_cal_date ) > 1000000 )
               {
                  if8i_i.Ch[ i ].CalSuccessful = FALSE;
               }
            }
         }
      }
      
      
      /* Sourcing Over Current? */
      /* Check for a Sourcing Over Current condition we need to deal with here */
      /* Rolling check ensures a new sample so we only check 1 time per sample */
      if ( last_rolling != if8i_i.rolling )
      {
         last_rolling = if8i_i.rolling;
         for ( i = 0 ; i < IF8I_NUM_CH ; i++ )
         {
            /* Check for a new Over Source condition on any channel */
            /*   Count non-zero & not already locked */
            if ( ( dbase[ i ].Alarm.SourceOverCount > over_current_timer_expired ) &&
                  !dbase[ i ].Alarm.SourceOverLocked )
            {
               /* Flag we found one & flag the channel as locked so Input remains set high */
            	send_lock = TRUE;
         	   dbase[ i ].Alarm.SourceOverLocked = TRUE;
            	/* Clear Sourcing Field Power & VBias Disable bits in AFE Ctrl2 register which should remove over current */
               ADC_config[ i ].chan_reg.afe_cntrl2 &= ~( AFE_FIELD_POWER | AFE_DVBIAS_DIS );
            }
            adc_data[ i ] = ADC_config[ i ].cfg_regs[ 3 ];
         }
         /* Check for flag we need to reconfig on a new lock */
         if ( send_lock )
         {
            /* Reconfigure ADCs to turn off sourcing on Over Current channel(s) */
            /*   Just write the 4th register AFE Ctrl2 */
            if8i_Write_ADCs( adc_data, 3 );
         }
      }
   }
}

/*---------------------------------------------------------------------------
**
** User_ValidateKey()
**
** User defined electronic key check.
** Checks electronic key against product information. 
**
**---------------------------------------------------------------------------
**
** Inputs:
**    msg          - message structure
**    parsed_epath - parsed epath data
**
** Outputs:
**    status - general status
**
** Usage:
**    User_ValidateKey(MESSAGE_STRUCT_TYPE *msg, PARSED_EPATH_TYPE *parsed_epath)
**
**---------------------------------------------------------------------------
*/

GS_TYPE User_ValidateKey(MESSAGE_STRUCT_TYPE *msg, PARSED_EPATH_TYPE *parsed_epath)
{
  /*
   * Current IF8I is not compatable with any modules so use standard match rules.
   */

  /* Check vendor id */
  if ( (parsed_epath->key->vendor_id != 0) &&
       (parsed_epath->key->vendor_id != id_obj_inst_attr.vendor_id) ) {
    msg->extended_length = 1;
    msg->extended_status[0] = ES_PRODUCT_CODE_MISMATCH;
    return(GS_KEY_FAILURE);
  }
  /* Check product type */
  if ( (parsed_epath->key->product_type != 0) &&
       (parsed_epath->key->product_type != id_obj_inst_attr.product_type) ) {
    msg->extended_length = 1;
    msg->extended_status[0] = ES_PRODUCT_TYPE_MISMATCH;
    return(GS_KEY_FAILURE);
  }
  /* Check product code */
  if ( (parsed_epath->key->product_code != 0) &&
       (parsed_epath->key->product_code != id_obj_inst_attr.product_code) ) {
    msg->extended_length = 1;
    msg->extended_status[0] = ES_PRODUCT_CODE_MISMATCH;
    return(GS_KEY_FAILURE);
  }
  /* Major rev not wild card */
  if ( parsed_epath->key->major_rev & ~KEY_MUST_MATCH_MASK ) {

    /* Exact match */
    if ( !(parsed_epath->key->major_rev & KEY_MUST_MATCH_MASK) ) {

      /* Major/Minor rev must be equal to product rev */
      if ( (parsed_epath->key->major_rev != id_obj_inst_attr.major_rev) ||
           (parsed_epath->key->minor_rev != id_obj_inst_attr.minor_rev) ) {
        msg->extended_length = 1;
        msg->extended_status[0] = ES_REVISION_MISMATCH;
        return(GS_KEY_FAILURE);
      }
    }

    /* Compatable key */
    else {

      /* Major/Minor rev must be less than or equal to product rev */
      if ( ((parsed_epath->key->major_rev & ~KEY_MUST_MATCH_MASK) > id_obj_inst_attr.major_rev) ||
           (((parsed_epath->key->major_rev & ~KEY_MUST_MATCH_MASK) == id_obj_inst_attr.major_rev) &&
            (parsed_epath->key->minor_rev > id_obj_inst_attr.minor_rev)) ) {
        msg->extended_length = 1;
        msg->extended_status[0] = ES_REVISION_MISMATCH;
        return(GS_KEY_FAILURE);
      }
    }
  }

  return(GS_SUCCESS);
}

/*---------------------------------------------------------------------------
**
** User_Reset()
**
** User defined reset.
** Perform any user defined actions before we pull the plug.
**
** If return GS_SUCCESS toolkit will send response and reset module.
**
** If return error then toolkit will send error response and not reset module.
**
** Note:
** If reset is due to NVS update complete, returning an error will also cause
** the NVS update to abort.
**
**---------------------------------------------------------------------------
**
** Inputs:
**    msg        - message structure
**    reset_type - POWER_UP_RESET, OUT_OF_BOX_RESET or UNLOCK_RESET 
**
** Outputs:
**    status - general status
**
** Usage:
**    User_Reset(MESSAGE_STRUCT_TYPE *msg, USINT reset_type)
**
**---------------------------------------------------------------------------
*/

GS_TYPE User_Reset(MESSAGE_STRUCT_TYPE *msg, USINT reset_type)
{
#if     0       /*** Used to test initcode() *** */
    void initcode(void);
    initcode();
#endif

  return(GS_SUCCESS);
}

/*---------------------------------------------------------------------------
**
** User_NvsUpdate()
**
** User defined NVS Update request.
** Perform any user defined actions before NVS Update starts.
**
** If return GS_SUCCESS toolkit will continue with update.
**
** If return error then toolkit will send error response and not update.
**
**---------------------------------------------------------------------------
**
** Inputs:
**    msg    - message structure
**
** Outputs:
**    status - general status
**
** Usage:
**    User_NvsUpdate(MESSAGE_STRUCT_TYPE *msg)
**
**---------------------------------------------------------------------------
*/

GS_TYPE User_NvsUpdate(MESSAGE_STRUCT_TYPE *msg)
{
  return(GS_SUCCESS);
}
