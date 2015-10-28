/////////////////////////////////////////////////////////////////////////////
/// @file ApexDiagBackplaneLoopback.cpp
///
/// Implementation of the APEX loopback diagnostics including initialization,
/// power-up, and runtime diagnostics.
///
/// @if REVISION_HISTORY_INCLUDED
/// @par Edit History
/// - akk       21-MAY-2013 Created.
/// - mgrad     28-NOV-2013 File name changed during MISRA fixes.
/// - pszramo   28-NOV-2013 Refactoring to reduce DgnBPLoopbackTest() complexity.
/// - dtstalte  03-DEC-2013 More MISRA fixes.  Changing to a class.
/// - dtstalte  12-DEC-2013 Changed parameter type in ExecuteLoopback to an enum.
/// - dtstalte  16-DEC-2013 Add coding standard exception for a global variable.
/// - akkudva   16-JAN-2014 Includes code improvements to avoid response timeout
///                         due to heavily loaded CBus.  Also adds delay between
///                         TxControl write and enabling TAV & TE interrupt.
///                         Handles downsized frame on TxDone with retry.
/// - dtstalte  29-JAN-2014 MISRA fixes after resolving Lgx00147826.
///                         Complexity reduction of WaitForTxAvAndProceed.
/// - wmpeloso 15-JAN-2014 Added writing of diagnostics flag bits to the
///                        BuildConfig Spy object topic.
/// @endif
///
/// @par Copyright (c) 2014 Rockwell Automation Technologies, Inc. All rights reserved.
/////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
// <none>

// C PROJECT INCLUDES
// <none>

// C++ PROJECT INCLUDES
#include "ApexDiagBackplaneLoopback.hpp"        // for class declaration
#include "ApexRegisters.hpp"                    // for HI_ApexReg
#include "Apex.hpp"                             // for several function declarations
#include "ApexDebugLog.hpp"                     // for printl
#include "cbs.hpp"                              // for TxError

// FORWARD REFERENCES
// (none)

// STATIC MEMBER VARIABLES
ApexDiagBackplaneLoopback::ImageSelector ApexDiagBackplaneLoopback::m_ImgSelector = ApexDiagBackplaneLoopback::IMAGE1;
volatile ApexDiagBackplaneLoopback::LoopbackState ApexDiagBackplaneLoopback::m_LoopbackState = ApexDiagBackplaneLoopback::LOOPBACK_INIT;
volatile bool ApexDiagBackplaneLoopback::m_bSkipLoopbackTxRetry = false;
volatile bool ApexDiagBackplaneLoopback::m_bCorruptLoopbackCrc = false;
bool ApexDiagBackplaneLoopback::m_bIsBpLoopBackStarted = false;
UINT32 ApexDiagBackplaneLoopback::m_IterationStartTime = 0;
volatile bool ApexDiagBackplaneLoopback::m_bIsTxNotAv = false;


///////////////////////////////////////////////////////////////////////////////
//  ApexDiagBackplaneLoopback::CrcCopy8()
///
/// @memberof ApexDiagBackplaneLoopback
/// @details This function calculates the CRC over the loopback image & stores
/// the image & CRC into the Tx-FIFO.
///////////////////////////////////////////////////////////////////////////////
void ApexDiagBackplaneLoopback::CrcCopy8()
{
    // Keep track and seed the CRC value
    UINT32 crcAccumulator = 0xFFFF;

    // load the respective pattern, but do the crc over the first 222 bytes
    for (UINT32 i = 0; i < ((HI_VAL_TXRAM_MAX_LEN_20 - 4) / 4); i++)
    {

        if (m_ImgSelector == IMAGE1)
        {
            if (i == 0)
            {
                // obtain the slot number & plug into the 'destination slot' field of the frame header
                crcAccumulator ^= (HI_ApexReg.TxRam1[0] = (LOOP_BACK_PATTERN_OF_ZEROES_ONES | (HI_ApexReg.ModuleAddress & HI_MASK_MODADDR_MA)));
            }
            else
            {
                crcAccumulator ^= (HI_ApexReg.TxRam1[i] = LOOP_BACK_PATTERN_OF_32_BIT_ZEROES_ONES);
            }
        }
        else
        {
            if (i == 0)
            {
                // obtain the slot number & plug into the 'destination slot' field of the frame header
                crcAccumulator ^= (HI_ApexReg.TxRam1[0] = (LOOP_BACK_PATTERN_OF_ONES_ZEROES  | (HI_ApexReg.ModuleAddress & HI_MASK_MODADDR_MA)));
            }
            else
            {
                crcAccumulator ^= (HI_ApexReg.TxRam1[i] = LOOP_BACK_PATTERN_OF_32_BIT_ONES_ZEROES);
            }
        }

        // evaluate the parallel CRC expression for 32-bit data
        crcAccumulator = parCrc32Compute(crcAccumulator);
    }

    UINT32 lastWord;
    // load the last word, but do CRC over the last 2 bytes only
    if (m_ImgSelector == IMAGE1)
    {
        lastWord = LOOP_BACK_PATTERN_OF_32_BIT_ZEROES_ONES;
    }
    else
    {
        lastWord = LOOP_BACK_PATTERN_OF_32_BIT_ONES_ZEROES;
    }

    // continue CRC calculation for the last 2 bytes
    for (UINT32 i = 0; i < 2; i++)
    {
        crcAccumulator ^= lastWord & 0xff;

        // evaluate the parallel CRC expression for 8-bit data
        crcAccumulator = parCrc8Compute(crcAccumulator);

       lastWord >>= 8;
    }

    crcAccumulator ^= 0xffff;

    // load CRC bytes into the last 2 bytes of the Tx-FIFO at offset = 226
    HI_ApexReg.TxRam1[(HI_VAL_TXRAM_MAX_LEN_20 / 4) - 1] = ((lastWord & 0xffff) | (UINT32)(crcAccumulator << 16));

    printl(__func__, "Tx0=0x%08x,Tx56=0x%08x,crc=0x%08x,lastword=0x%08x",
           HI_ApexReg.TxRam1[0], HI_ApexReg.TxRam1[56], crcAccumulator, lastWord,
           BP_LPBK_LOG_FILTER_MASK);

    // toggle image selection for next pass
    if (m_ImgSelector == IMAGE1)
    {
        m_ImgSelector = IMAGE2;
    }
    else
    {
        m_ImgSelector = IMAGE1;
    }
}

///////////////////////////////////////////////////////////////////////////////
//  ApexDiagBackplaneLoopback::DgnInitBPLoopback()
///
/// @memberof ApexDiagBackplaneLoopback
/// @details This function initializes the loopback test. Called once at
/// power up.
///////////////////////////////////////////////////////////////////////////////
void ApexDiagBackplaneLoopback::Init()
{
    if (BP_LOOPBACK_ENABLED)
    {
        // Set the shared memory HI_ApexParam.BuildConfig backplane loopback
        // enabled bit.
        HI_ApexParam.BuildConfig |= (1 << BP_LOOPBACK_ENABLED_BIT);

        // Use this to select a pattern to load into Tx-FIFO
        m_ImgSelector = IMAGE1;

        // Initial State for Diagnostics
        m_LoopbackState = LOOPBACK_INIT;
        
        // Just to be safe, initialize these to false too
        m_bSkipLoopbackTxRetry = false;
        m_bIsBpLoopBackStarted = false;

        printl(__func__, "Init Complete. IsCBusUp?=%d,%d %d %d", ControlBusIsUp, 0, 0, 0,
              BP_LPBK_LOG_FILTER_MASK);
    }
    else
    {
        // Clear the shared memory HI_ApexParam.BuildConfig backplane loopback
        // enabled bit.
        HI_ApexParam.BuildConfig &= ~(1 << BP_LOOPBACK_ENABLED_BIT);
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///@begin_claim_coding_exception
///
/// Rule 7-3-1 The global namespace shall only contain main, namespace declarations and extern "C" declarations.
/// HI_BIT_INTR_CMD1 used in XmitFrameEnd is a global hardware register used
/// throughout Apex code.  Changing it would require heavy modification to the
/// existing legacy code base.
///
//@end_claim_coding_exception
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
//  ApexDiagBackplaneLoopback::XmitFrameEnd()
///
/// @memberof ApexDiagBackplaneLoopback
/// @details This function is called once the loopback frame completes
/// transmission.  It will check for complete conditions & finally point the
/// Tx interrupt vectors to their normal XmitStart & TxError handlers.
///////////////////////////////////////////////////////////////////////////////
void ApexDiagBackplaneLoopback::XmitFrameEnd()
{
    if (m_LoopbackState != LOOPBACK_WAITING)
    {
        printl(__func__, "Unexpected loopback state=%d %d %d %d", m_LoopbackState, 0, 0, 0,
              BP_LPBK_LOG_FILTER_MASK);
        firmExcept();
    }

    // Remember error/status
    UINT32 txStatus = HI_ApexReg.TxStatus1;
    // Clear error
    HI_ApexReg.TxStatus1 = 0;

    UINT32 txDoneLength = HI_ApexReg.TxControl1 & HI_MASK_TXCTRL_LENGTH;

    printl(__func__, "txStatus=0x%08x txDoneLen=%d %d %d", txStatus, txDoneLength, 0, 0,
          BP_LPBK_LOG_FILTER_MASK);

    // check if one of the error bits are set
    // also verify that this frame was transmitted 16bits@DDR if there was ever such a transmission before
    if ((txStatus & (HI_BIT_TXSTAT_BDF | HI_BIT_TXSTAT_NRP)) != 0)
    {
        // either BDF or NRP is set
        printl(__func__, "unexpected frame status! %d %d %d %d", 0, 0, 0, 0,
              BP_LPBK_LOG_FILTER_MASK);
        firmExcept();
    }
    else if ( ((txStatus & HI_BIT_TXSTAT_BSY) == HI_BIT_TXSTAT_BSY) || ((txStatus & HI_BIT_TXSTAT_CRC) == HI_BIT_TXSTAT_CRC) )
    {
        printl(__func__, "busy or backplane crc error status=0x%08x %d %d %d", txStatus, 0, 0, 0,
              BP_LPBK_LOG_FILTER_MASK);

        if (FAULT_INJECTION_SUPPORT_ENABLED)
        {
            if (!m_bSkipLoopbackTxRetry)
            {
                // Skip a retry attempt on a CRC or BUSY error to inducing a timeout
                // on the loopback response as part of the fault injection test
                m_LoopbackState = LOOPBACK_RETRY;
            }
        }
        else
        {
            m_LoopbackState = LOOPBACK_RETRY;
        }
    }
    else if (txDoneLength != 0)
    {
        printl(__func__, "unexpected length len=%d %d %d %d", txDoneLength, 0, 0, 0,
              BP_LPBK_LOG_FILTER_MASK);
        // it may be probable that due to noise on the corresponding status lines on the backplane
        // the frame got downsized; so instead of trying to transmit remaining bytes, simply retry
        // the loopback; the Rx screener is going to drop anything received loopedback frame less than 
        // the complete length
        m_LoopbackState = LOOPBACK_RETRY;
    }
    else
    {
        // Invalid case.  Should never get here, but need
        // 'else' clause to not violate MISRA.
    }

    printl(__func__, "XmitDone Complete! DgnLoopBackState=%d %d %d %d", m_LoopbackState, 0, 0, 0,
          BP_LPBK_LOG_FILTER_MASK);

    // restore normal handling for TxAv/TxE interrupts
    HI_pIrqVectors[HI_VAL_IRQ_VEC_TE1] = TxError;
    XmitOk = XmitStart;

    // Enable Host/Rx Interrupts (so loopback response may be processed)
    ///@begin_code_exception
    HI_interruptEnable(HI_BIT_INTR_FIB | HI_BIT_INTR_CMD1);
    //@end_code_exception

    // do the same stuff as in XmitCleanup() -- this will normally turn off Tx interrupts
    XmitStart();
    HI_interruptFairnessDelay(HI_BIT_INTR_TAV1 | HI_BIT_INTR_TE1);

}

///////////////////////////////////////////////////////////////////////////////
//  ApexDiagBackplaneLoopback::SetupFirstIteration()
///
/// @memberof ApexDiagBackplaneLoopback
/// @details This function performs initiliazation for the first run of the
/// loopback test.
///////////////////////////////////////////////////////////////////////////////
void ApexDiagBackplaneLoopback::SetupFirstIteration()
{
    // record start time for the 1st iteration
    m_IterationStartTime = HI_ApexReg.SystemTime;

    // change to NOT_READY, so that we don't update the Start Time
    // every time we re-enter to check whether the host has started the loopback
    m_LoopbackState = LOOPBACK_NOT_READY;
    
    // look at ControlBus and Apex params
    CheckForTestReady();

    printl(__func__, "1st Iteration>LpBkStart?=%d,CBusUp?=%d,time=%d %d %d %d", 
           m_bIsBpLoopBackStarted, ControlBusIsUp, HI_ApexReg.SystemTime, 0,
          BP_LPBK_LOG_FILTER_MASK);
}

///////////////////////////////////////////////////////////////////////////////
//  ApexDiagBackplaneLoopback::CheckForTestReady()
///
/// @memberof ApexDiagBackplaneLoopback
/// @details This function checks if ControlBus and the host have are ready
/// for the loopback test.
///////////////////////////////////////////////////////////////////////////////
void ApexDiagBackplaneLoopback::CheckForTestReady()
{
    if (HI_ApexParam.StartBPLoopBack && ControlBusIsUp)
    {
        // remember we started our 1st iteration
        m_bIsBpLoopBackStarted = true;

        printl(__func__, "1st Iteration > LpBkStarted @ time=%d %d %d %d", HI_ApexReg.SystemTime, 0, 0, 0,
              BP_LPBK_LOG_FILTER_MASK);
    }
}

///////////////////////////////////////////////////////////////////////////////
//  ApexDiagBackplaneLoopback::CheckFrameHeader()
///
/// @memberof ApexDiagBackplaneLoopback
/// @details This function makes sure a frame header is valid.
///////////////////////////////////////////////////////////////////////////////
void ApexDiagBackplaneLoopback::CheckFrameHeader()
{
    UINT32 FrHdr = HI_ApexReg.TxRam1[0];
    if ((FrHdr != (LOOP_BACK_PATTERN_OF_ZEROES_ONES | (HI_ApexReg.ModuleAddress & HI_MASK_MODADDR_MA)))
            &&
        (FrHdr != (LOOP_BACK_PATTERN_OF_ONES_ZEROES | (HI_ApexReg.ModuleAddress & HI_MASK_MODADDR_MA)))
       )
    {
        printl(__func__, "!!Corrupt TxHeader=0x%08x, %d %d %d", FrHdr, 0, 0, 0,
              BP_LPBK_LOG_FILTER_MASK);
        firmExcept();
    }
}

///////////////////////////////////////////////////////////////////////////////
//  ApexDiagBackplaneLoopback::WaitForTxAvAndProceed()
///
/// @memberof ApexDiagBackplaneLoopback
/// @details This function makes sure that the CBus Tx is available before 
/// returning.
///////////////////////////////////////////////////////////////////////////////
void ApexDiagBackplaneLoopback::WaitForTxAvAndProceed()
{
    if (XmitOk == XmitEnd)
    {
        printl(__func__, "TxInProgress startTime=%d curTime=%d",
               m_IterationStartTime, HI_ApexReg.SystemTime, 0, 0,
               BP_LPBK_LOG_FILTER_MASK);
        
        // express an interest to the CBus code that loopback needs to be performed
        // so that we get a chance to grab the transmitter; otherwise, on a heavily
        // loaded module, each time we come here, we may find the transmitter busy
        SetTxNotAv();
        
        // TxAv / TxE interrupts should already be left enabled if we are here; since 
        // TxDone is pending; so spin here for a little while in the hope that the event 
        // will be completed soon
        
        UINT32 txWaitStartUs = HI_ApexReg.SystemTime;
        
        while ( m_bIsTxNotAv && ((pFunc_t volatile )XmitOk == XmitEnd))
        {
            // spin here waiting for the last pending transmission to complete
            // it will complete due to one of the following reasons:
            // a. this sequence got preempted just before setting the TxNotAv flag,
            // AND the Tx is now Idle, [OR]
            // b. this sequence set the flag, then an in-progress frame completed,
            // AND the flag got cleared inside XmitCleanup
            
            // process a bunch of NOPs to avoid overwhelming the bus
            INT32 nopCount = BACKPLANE_TXAV_SPIN_NOP_CNT;
            while (nopCount-- > 0)
            {
                NopDelay();
            }
            
            // check for timeout
            if ((HI_ApexReg.SystemTime - txWaitStartUs) > BACKPLANE_TX_AV_TIMEOUT_US)
            {
                // timeout value should consider the latencies in:                 
                // i.  completing the frame transmission 
                //      (typically, it will take 3-4us for 224 byte frame @ 16-bit DDR)
                // ii. returning to the IrqHandler to clean-up on success/error
                //      (worst case should consider longest path inside XmitEnd() viz. mCrc path
                //      along TxWriteEnd() and TxReqstEnd())
                // iii.switching back to the background loop
                //      (for Debug builds, containing may printl statements in the code path - 
                //      dump the timeout by a significant 2x-4x multiplier)
                
                printl(__func__, "Tx Too Busy> startTime=%d curTime=%d",
                       m_IterationStartTime, HI_ApexReg.SystemTime, 0, 0,
                       BP_LPBK_LOG_FILTER_MASK);
                firmExcept();
            }
        }
        
        CheckIfTxIsAvailable();
    }
    
    // Make sure TAV/TE interrupts are now turned off
    // they should be for each of these cases:
    // - we started this sequence & Tx was already idle
    // - we started this sequence, Tx was busy & then became idle
    // - we started this sequence, Tx was busy & then we forced 
    //   a switch back here after the last frame completed 
    if ( (HI_ApexReg.IrqEnbl & (HI_BIT_INTR_TAV1 | HI_BIT_INTR_TE1)))
    {
        // assert here since we have found otherwise
        //
        printl(__func__, "TAV/TE Not Cleared> IrqEnbl=0x%08x", (UINT32)HI_ApexReg.IrqEnbl, 0, 0, 0,
               BP_LPBK_LOG_FILTER_MASK);
        firmExcept();
    }
}

///////////////////////////////////////////////////////////////////////////////
//  ApexDiagBackplaneLoopback::CheckIfTxIsAvailable()
///
/// @memberof ApexDiagBackplaneLoopback
/// @details This function checks the state of CBus Tx.
///////////////////////////////////////////////////////////////////////////////
void ApexDiagBackplaneLoopback::CheckIfTxIsAvailable()
{
        // test for all conditions to determine whether Tx is now indeed available
        //
        if (m_bIsTxNotAv && (XmitOk == XmitStart))
        {
            // Clear the flag for the case where Tx int. ran just before setting it
            // & then became idle
            m_bIsTxNotAv = false;
            printl(__func__, "Tx Now Idle> curTime=%d",
                   HI_ApexReg.SystemTime, 0, 0, 0,
                   BP_LPBK_LOG_FILTER_MASK);
            
        }
        else if (!m_bIsTxNotAv && (XmitOk == XmitEnd))
        {
            // the spin loop in WaitForTxAvAndProceed was effective
            printl(__func__, "Tx Now Av> curTime=%d", 
                   HI_ApexReg.SystemTime, 0, 0, 0,
                   BP_LPBK_LOG_FILTER_MASK);
        }
        else
        {
            // there should be no other reason for the WaitForTxAvAndProceed loop to have terminated
            printl(__func__, "Unexpected Tx State> Flag=%d, XmitOk=0x%08x", 
                   m_bIsTxNotAv, (UINT32)XmitOk, 0, 0,
                   BP_LPBK_LOG_FILTER_MASK);
            firmExcept();
        }
}

///////////////////////////////////////////////////////////////////////////////
//  ApexDiagBackplaneLoopback::ExecuteLoopback()
///
/// @memberof ApexDiagBackplaneLoopback
/// @details This function contains the main body of the test.  It checks what
/// the current test state is and takes appropriate action.
///
/// @param 
///     none
/// @return TestState
///     @retval TEST_LOOP_COMPLETE - the test has completed
///     @retval TEST_IN_PROGRESS - the test is still in progress
///////////////////////////////////////////////////////////////////////////////
ApexDiagnostic::TestState ApexDiagBackplaneLoopback::ExecuteLoopback()
{
    // Host has started the loopback & we are not waiting for an earlier test iteration
    // to complete; so initiate a new one
    // Loopback state may be one of the following:
    // INIT: if this is our first iteration & host had already started the loopback
    // NOT_READY: if our first iteration was already started but host has just now started the loopback
    // SUCCESS: the previous iteration was a success
    // RETRY:   the previous iteration requires a retry

    if (m_LoopbackState == LOOPBACK_SUCCESS)
    {
        // if the previous iteration was a SUCCESS, we were polling in short intervals to check for completion
        // so simply report completion status to the scheduler. we will come back here to execute the next
        // iteration at the next scheduled interval
        m_LoopbackState = LOOPBACK_COMPLETE;
        
        printl(__func__, "LoopBack Iteration Complete! %d %d %d %d", 0, 0, 0, 0,
               BP_LPBK_LOG_FILTER_MASK);
        
        return ApexDiagnostic::TEST_LOOP_COMPLETE;
    }

    printl(__func__, "Start New Loopback Iteration> State=%d %d %d %d", m_LoopbackState, 0, 0, 0,
          BP_LPBK_LOG_FILTER_MASK);

    
    // reset the start time for every attempt! (If we never get around to a retry being successful
    // the completion check diagnostic will catch us & assert the module
    m_IterationStartTime = HI_ApexReg.SystemTime;

    // Disable Host/Receiver Interrupts, so that we don't invalidate the Tx-FIFO contents
    // representing the loopback image
    HI_interruptDisable(HI_BIT_INTR_CMD1 | HI_BIT_INTR_FIB);

    // Check if the Transmitter is really idle? We could hit a timing window where a frame
    // was just transmitted, we returned to the background loop and locked interrupts here, but
    // transmitter didn't finish with the frame. So attempting to access it in that state will
    // generate a data abort!
    WaitForTxAvAndProceed();
    
    // we (the ARM core) own the TxRAM; so we shall start loading the loopback frame in the Transmit buffer
    
    // Load the pattern & its crc directly into the Tx-FIFO
    CrcCopy8();

    if (FAULT_INJECTION_SUPPORT_ENABLED)
    {
        if (m_bCorruptLoopbackCrc)
        {
            // Intentionally modify the loop back to invalidate the pre-computed CRC
            // for the purpose of fault injection
            //
            HI_ApexReg.TxRam1[1] = 0xFFFFFFFF;
            m_bCorruptLoopbackCrc = false;
        }
    }

    // temporarily re-route Tx interrupts to handle end of loopback transmit
    XmitOk = XmitFrameEnd;
    HI_pIrqVectors[HI_VAL_IRQ_VEC_TE1] = XmitFrameEnd;

    // update loopback state
    m_LoopbackState = LOOPBACK_WAITING;

    printl(__func__, "Loaded Image%d,m_IterationStartTime=%d, %d %d",
            m_ImgSelector, m_IterationStartTime, 0, 0,
            BP_LPBK_LOG_FILTER_MASK);
    
    // re-schedule at a smaller interval to check for loopback response
    // or execute a retry attempt or check for timeout!
    ApexDiagnostic::DgnList[ApexDiagnostic::DGN_BP_LOOPBACK_NUM].triggerValueTimeslice = ApexDiagnostic::m_SliceNumber + ApexDiagnostic::MsToDgnSlices(BP_LOOPBACK_SHORT_STEP_MS);

    printl(__func__, "Re-schedule:DgnSliceNo=%d TrigVal=%d %d %d",
           ApexDiagnostic::m_SliceNumber, ApexDiagnostic::DgnList[ApexDiagnostic::DGN_BP_LOOPBACK_NUM].triggerValueTimeslice, 0, 0,
          BP_LPBK_LOG_FILTER_MASK);

    // once we loaded the frame, it should not have changed, since Tx was idle
    // AND we disabled FIB & CMD interrupts; but be sure about it by checking
    // the header bytes (if someone else wrote to the TxRAM, header would have 
    // at least got corrupted
    CheckFrameHeader();

    // trigger transmission
    HI_ApexReg.TxControl1 = (((HI_VAL_TXRAM_MAX_LEN_20-4) & HI_MASK_TXCTRL_LENGTH) |  // length
                              (PRIO_TOP << 8));  // priority

    
    // spin here for a little while to make sure TAV/TE bits have cleared already
    // due to the write to the TxControl1 register; it may take a very little delay 
    // for the CBus transmitter to relay its status to the interrupt controller
    INT32 nopCount = BACKPLANE_TXAV_DELAY_NOP_CNT;
    while (nopCount-- > 0)
    {
        NopDelay();
    }
    
    // turn on Tx interrupt to check for transmit status
    HI_interruptEnable(HI_BIT_INTR_TAV1 | HI_BIT_INTR_TE1);

    // we will come back & check for a timeout or a response
    return ApexDiagnostic::TEST_IN_PROGRESS;
}

///////////////////////////////////////////////////////////////////////////////
//  ApexDiagBackplaneLoopback::CheckForCompletion()
///
/// @memberof ApexDiagBackplaneLoopback
/// @details This function checks if an initiated loopback completed or if it
/// failed for some reason.
///
/// @return         ApexDiagnostic::TestState
///     @retval     TEST_LOOP_COMPLETE   if test has finished. 
///     @retval     TEST_IN_PROGRESS     if test is still in progress.
///////////////////////////////////////////////////////////////////////////////
ApexDiagnostic::TestState ApexDiagBackplaneLoopback::CheckForCompletion()
{
    // check for timeout...
    if ((INT32)(HI_ApexReg.SystemTime - (m_IterationStartTime + BACKPLANE_LOOPBACK_RESPONSE_TIMEOUT_US)) > 0)
    {
        // timeout expired!
        // we are here either because
        // loopback was never started by the host (NOT_READY), OR
        // we did not get a response back for the earlier iteration (WAITING) OR
        // we got a response, but background loop was suspended too long
        // so check loopback state again before asserting the fault line
        if (m_LoopbackState != LOOPBACK_SUCCESS)
        {
            printl(__func__, "No Response Arrived! LoopbackState=%d, time=%d %d %d",
                   m_LoopbackState, HI_ApexReg.SystemTime, 0, 0,
                  BP_LPBK_LOG_FILTER_MASK);

            firmExcept();
            // for sake of compiler warning
            return (ApexDiagnostic::TEST_LOOP_COMPLETE);
        }
        else
        {
            printl(__func__, "Timeout but Response Arrived! LoopbackState=%d, time=%d %d %d",
                   m_LoopbackState, HI_ApexReg.SystemTime, 0, 0,
                  BP_LPBK_LOG_FILTER_MASK);

            return (ApexDiagnostic::TEST_LOOP_COMPLETE);
        }
    }
    else
    {
        ApexDiagnostic::TestState status;
        switch (m_LoopbackState)
        {
        case LOOPBACK_WAITING:
        case LOOPBACK_NOT_READY:        
            printl(__func__, "Waiting or Not Ready...state=%d %d %d %d", m_LoopbackState, 0, 0, 0,
                  BP_LPBK_LOG_FILTER_MASK);

            // re-schedule at a smaller interval to check for loopback response
            ApexDiagnostic::DgnList[ApexDiagnostic::DGN_BP_LOOPBACK_NUM].triggerValueTimeslice = ApexDiagnostic::m_SliceNumber + ApexDiagnostic::MsToDgnSlices(BP_LOOPBACK_SHORT_STEP_MS);

            printl(__func__, "Re-schedule:DgnSliceNo=%d TrigVal=%d %d %d",
                    ApexDiagnostic::m_SliceNumber, ApexDiagnostic::DgnList[ApexDiagnostic::DGN_BP_LOOPBACK_NUM].triggerValueTimeslice, 0, 0,
                   BP_LPBK_LOG_FILTER_MASK);

            status = ApexDiagnostic::TEST_IN_PROGRESS;
            break;
        default:
            // no other state expected
            printl(__func__, "...Unexpected loopback state = %d %d %d %d", m_LoopbackState, 0, 0, 0,
                  BP_LPBK_LOG_FILTER_MASK);

            firmExcept();

            // for sake of compiler warning
            status = ApexDiagnostic::TEST_LOOP_COMPLETE;
            break;
        }
        return status;
    }
}

/////////////////////////////////////////////////////////////////////////////
//  FUNCTION NAME: DgnBPLoopbackTest
//

/////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//  ApexDiagBackplaneLoopback::RunTest()
///
/// @memberof ApexDiagBackplaneLoopback
/// @details This function is the runtime backplane loopback test. It will
/// send a loopback frame with mCRC included & check the response conditions
/// to determine whether the test has passed or failed. If a failure occurs,
/// the test will cause an assert by itself.
///
/// @param <anonymous>  Unused.
///
/// @return         ApexDiagnostic::TestState
///     @retval     TEST_LOOP_COMPLETE   if test has finished. 
///     @retval     TEST_IN_PROGRESS     if test is still in progress.
///////////////////////////////////////////////////////////////////////////////
ApexDiagnostic::TestState ApexDiagBackplaneLoopback::RunTest(ApexDiagnostic::DiagControlBlock *)
{
    if (BP_LOOPBACK_ENABLED)
    {
        printl(__func__, "Iteration>DgnSliceNo=%d, TrigVal=%d %d %d",
               ApexDiagnostic::m_SliceNumber, ApexDiagnostic::DgnList[ApexDiagnostic::DGN_BP_LOOPBACK_NUM].triggerValueTimeslice, 0, 0,
              BP_LPBK_LOG_FILTER_MASK);

        if (DEBUG_LOGGING_ENABLED)
        {
            if (!HI_ApexParam.StartBPLoopBack)
            {
                return ApexDiagnostic::TEST_LOOP_COMPLETE;
            }
        }

        if (m_LoopbackState == LOOPBACK_INIT)
        {
            SetupFirstIteration();
        }
        else if (m_LoopbackState == LOOPBACK_NOT_READY)
        {
            CheckForTestReady();
        }
        else
        {
            // To avoid MISRA violation
        }

        if (m_bIsBpLoopBackStarted && (m_LoopbackState != LOOPBACK_WAITING))
        {
            // the return value tells us whether the curent iteration of the 
            // test is in progress or completed; so simply return that value
            // back to the scheduler
            return (ExecuteLoopback());
        }
        // if we did not return, check for timeouts

        return CheckForCompletion();
    }
    else
    {
        // BP LOOPBACK DISABLED
        return ApexDiagnostic::TEST_LOOP_COMPLETE;
    }
}
