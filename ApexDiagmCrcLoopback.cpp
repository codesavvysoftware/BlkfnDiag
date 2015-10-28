/////////////////////////////////////////////////////////////////////////////
/// @file ApexDiagmCrcLoopback.cpp
///
/// Implementation of the APEX mCRC loopback diagnostics (for the ENet/CNet
/// CommPort) including initialization, power-up, and runtime diagnostics.
///
/// @if REVISION_HISTORY_INCLUDED
/// @par Edit History
/// - [0] akk   12-JUL-2013 Created.
/// - mgrad     28-NOV-2013 File name changed during MISRA fixes.
/// - pszramo   29-NOV-2013 Refactoring to reduce DgnMCRCLoopbackTest()
///                             complexity.
/// - dtstalte  04-DEC-2013 MISRA changes.  Converted to a class.
/// - dtstalte  12-DEC-2013  Moved in mCRC specific functions from global files.
/// - wmpeloso  15-JAN-2014 Added writing of diagnostics flag bits to the
///                         BuildConfig Spy object topic.
/// @endif
///
/// @par Copyright (c) 2014 Rockwell Automation Technologies, Inc. All rights reserved.
/////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
// <none>

// C PROJECT INCLUDES
// <none>

// C++ PROJECT INCLUDES
#include "ApexStdType.hpp"              // for UINT types
#include "ApexDiagmCrcLoopback.hpp"     // for class declaration
#include "Apex.hpp"                     // for MCRC_LOOPBACK_ENABLED
#include "ApexDebugLog.hpp"             // for printl
#include "ApexDiagnostic.hpp"           // for test related constants

// FORWARD REFERENCES
// (none)

// STATIC MEMBER VARIABLES
UINT32   ApexDiagmCrcLoopback::m_IterationStartTime = 0;
UINT16   ApexDiagmCrcLoopback::m_LoopbackSequeneNumber = 0;
bool     ApexDiagmCrcLoopback::m_WaitingStateSelect = false;
UINT32   ApexDiagmCrcLoopback::m_TxmCrc = 0;
UINT32   ApexDiagmCrcLoopback::m_TestPatternmCrc = 0;
ApexDiagmCrcLoopback::mCrcLoopbackState ApexDiagmCrcLoopback::m_mCrcLoopbackState = ApexDiagmCrcLoopback::MCRC_INIT;



/////////////////////////////////////////////////////////////////////////////
//  ApexDiagmCrcLoopback::Init()
///
/// @memberof ApexDiagmCrcLoopback
/// @details This function initializes the mCRC loopback test parameters.
/// Called once at power-up.
/////////////////////////////////////////////////////////////////////////////
void ApexDiagmCrcLoopback::Init()
{
    // Write the shared memory HI_ApexParam.BuildConfig MCRC support
    // enabled bit.
    if (MCRC_SUPPORT_ENABLED)
    {
        HI_ApexParam.BuildConfig |= (1 << MCRC_SUPPORT_ENABLED_BIT);
    }
    else
    {
        HI_ApexParam.BuildConfig &= ~(1 << MCRC_SUPPORT_ENABLED_BIT);
    }

    if (MCRC_LOOPBACK_ENABLED)
    {
        // Set the shared memory HI_ApexParam.BuildConfig backplane loopback
        // enabled bit.
        HI_ApexParam.BuildConfig |= (1 << MCRC_LOOPBACK_ENABLED_BIT);

        //  should be doing this just once
        if (m_mCrcLoopbackState != 0)
        {
            firmExcept();
        }

        // Establish Initial Values for Diagnostics
        // !! Note that the Apex build will zero initialize all global data
        // so this cannot be done at declaration !!
        //
        m_IterationStartTime    = 0;
        m_LoopbackSequeneNumber = 0;
        m_WaitingStateSelect    = false;
        m_TxmCrc                = 0xffff;
        m_TestPatternmCrc       = 0xffff;
        m_mCrcLoopbackState     = MCRC_INIT;

        printl(__func__, "mCRCLoopback Test is Init %d,%d %d %d");
    }
    else
    {
        // Clear the shared memory HI_ApexParam.BuildConfig backplane loopback
        // enabled bit.
        HI_ApexParam.BuildConfig &= ~(1 << MCRC_LOOPBACK_ENABLED_BIT);
    }
}

/////////////////////////////////////////////////////////////////////////////
//  ApexDiagmCrcLoopback::TestNotSetUp()
///
/// @memberof ApexDiagmCrcLoopback
/// @details This function is called when the mCRC test is in the init state.
/// It sets up initial values and marks the test ready to run.
///
/// @param LpBkSize     Loopback packet size.
/// @param LpBkTxAddr   Loopback packet transmit address.
/// @param LpBkRxAddr   Loopback packet receive address.
/////////////////////////////////////////////////////////////////////////////
void ApexDiagmCrcLoopback::TestNotSetUp(UINT32 LpBkSize, void* LpBkTxAddr, void* LpBkRxAddr)
{
    // At least one of checked loopback parameters is not set up
    // we cannot proceed to trigger the loopback, but will count this as the
    // first iteration

    // record start time for the 1st iteration
    if (m_mCrcLoopbackState == MCRC_INIT)
    {
        m_IterationStartTime = HI_ApexReg.SystemTime;
        printl(__func__, "First Iteration Started @ Time=%d %d %d %d",
               m_IterationStartTime);

        m_mCrcLoopbackState = MCRC_NOT_READY;
    }

    // we can be here only if the state was INIT AND we changed it to NOT_READY in the first
    // iteration itself. Thereafter, once the parameters are established, they should never
    //  be destroyed
    if ((!DEBUG_LOGGING_ENABLED) && (m_mCrcLoopbackState != MCRC_NOT_READY))
    {
        firmExcept();
    }
    else if (DEBUG_LOGGING_ENABLED)
    {
        // for supporting unit tests with different loopback sizes
        m_mCrcLoopbackState = MCRC_NOT_READY;
    }
    else
    {
        // To avoid MISRA violation
    }

    // obviously, all we need is not established to be able to continue
    // the 1st iteration; so just skip to the timeout loop
    //

    printl(__func__, "LpBkPmtNotSet,TxAd=0x%08x,RxAddr=0x%08x,Sz=%d,Time=%d",
           (UINT32)LpBkTxAddr, (UINT32)LpBkRxAddr, LpBkSize, HI_ApexReg.SystemTime);

    // we are now going to skip into the timing loop
}

/////////////////////////////////////////////////////////////////////////////
//  ApexDiagmCrcLoopback::PreparePacket()
///
/// @memberof ApexDiagmCrcLoopback
/// @details This function prepares a test mCRC packet for transmission.
///
/// @param LpBkSize     Loopback packet size.
/// @param LpBkTxAddr   Loopback packet transmit address.
/////////////////////////////////////////////////////////////////////////////
void ApexDiagmCrcLoopback::PreparePacket(UINT32 LpBkSize, void* LpBkTxAddr)
{
    // increment the sequence number, doing so will ensure that we get a new response for each
    // looped back packet
    ++m_LoopbackSequeneNumber;
    // copy it to the end of the Tx-buffer, just before the mCRC; we will use this new value to
    // continue the CRC calculation over the intermediate mCRC already calculated over the test pattern
    // doing so will save shared memory accesses
    //
    // Use SR_UINT8 data type to overcome unsupported native 8-bit writes AND
    // since we do not know if the UINT16 is going to cross a word boundary
    //
    *(SR_UINT8*)((UINT32)LpBkTxAddr + LpBkSize - 4) = (UINT8)(m_LoopbackSequeneNumber & 0xff);
    *(SR_UINT8*)((UINT32)LpBkTxAddr + LpBkSize - 3) = (UINT8)(m_LoopbackSequeneNumber >> 8);

    // select one of the previously not selected WAITING states
    if (m_WaitingStateSelect)
    {
        m_mCrcLoopbackState = MCRC_WAITING_FOR_MCRC_ERROR;
        m_WaitingStateSelect = false;
    }
    else
    {
        m_mCrcLoopbackState = MCRC_WAITING_FOR_RX_AVAILABLE;
        m_WaitingStateSelect = true;
    }

    // calculate the final mCRC over the Test pattern followed by the sequence number
    // the mCRC (non-inverted) for the test pattern is already available from the 1st
    // iteration; so use it to carry-on the calculation over the current seq no.
    UINT16 LpBkSeqNoCopy = m_LoopbackSequeneNumber;
    m_TxmCrc = m_TestPatternmCrc;

    for (UINT32 idx = 0; idx < sizeof(m_LoopbackSequeneNumber); idx++)
    {
        // calculate the remaining mCrc over the sequence number
        // byte at a time
        //
        m_TxmCrc ^= (LpBkSeqNoCopy & 0xff);
        // invoke the 8-bit parallel CRC expression
        m_TxmCrc  = parCrc8Compute(m_TxmCrc);
        // next byte
        LpBkSeqNoCopy >>= 8;
    }

    if (m_mCrcLoopbackState == MCRC_WAITING_FOR_RX_AVAILABLE)
    {
        // invert the calculated value to produce the correct mCRC, only
        // if this test iteration demands so...
        m_TxmCrc ^= 0xffff;
    }

    // store the final 16-bit mCRC into the Tx-Buffer, once again use SR_UINT8 since
    // we are going to shared memory
    *(SR_UINT8*)((UINT32)LpBkTxAddr + LpBkSize - 2) = (UINT8)((UINT16)m_TxmCrc & 0xff);
    *(SR_UINT8*)((UINT32)LpBkTxAddr + LpBkSize - 1) = (UINT8)((UINT16)m_TxmCrc >> 8);
}

/////////////////////////////////////////////////////////////////////////////
//  ApexDiagmCrcLoopback::PreparePattern()
///
/// @memberof ApexDiagmCrcLoopback
/// @details This function prepares a test pattern to use on the mCRC packet.
///
/// @param LpBkSize     Loopback packet size.
/// @param LpBkTxAddr   Loopback packet transmit address.
/// @param LpBkRxAddr   Loopback packet receive address.
/////////////////////////////////////////////////////////////////////////////
void ApexDiagmCrcLoopback::PreparePattern(UINT32 LpBkSize, void* LpBkTxAddr, void* LpBkRxAddr)
{
    // we have the loopback parameters successfully initialized
    // go do the things within this if () only once, as
    // we are never going back to the INIT or NOT_READY state (except for unit tests)!

    // first check the addresses are word aligned
    if ( (((UINT32)LpBkTxAddr & 3) != 0) || (((UINT32)LpBkRxAddr & 3) != 0) )
    {
        firmExcept();
    }

    // load the TxBuffer with the test pattern & calculate the intermediate
    // mCRC over the fixed test pattern. This needs to be done just once, when the
    // 1st test iteration is executed. Thereafter, only the sequence number is going
    // to change, the fixed pattern & its mCRC won't

    // size for the test pattern is BufSize set by the ENet/CNet CommPort (which includes
    // everything) less 2 bytes for sequence Number less 2 bytes for mCRC

    // Fill a test pattern (of 0s & 1s) into the Transmit loopback buffer
    // calculate the mCRC over this pattern ( excluding the sequence number)

    UINT32* TxBufAddr = (UINT32*)LpBkTxAddr;
    UINT32  lastWord  = LPBK_PATTERN_OF_32_BIT_ZEROES_ONES;

    if (DEBUG_LOGGING_ENABLED)
    {
        // to support unit tests with different loopback sizes
        m_TestPatternmCrc = 0xffff;
    }

    // do a bunch of words
    for (UINT32 idx = 0; idx < ((LpBkSize - 4) / 4); idx++)
    {
        // write to the Tx buffer in shared memory, word at a time
        *TxBufAddr++ = LPBK_PATTERN_OF_32_BIT_ZEROES_ONES;

        m_TestPatternmCrc ^= LPBK_PATTERN_OF_32_BIT_ZEROES_ONES;

        // invoke the 32-bit parallel CRC expression
        m_TestPatternmCrc  = parCrc32Compute(m_TestPatternmCrc);
    }

    // any remaining bytes ?
    UINT32 numRemainingBytes = ((LpBkSize - 4) % 4);

    if (numRemainingBytes > 0)
    {
        // store the last word
        *TxBufAddr++ = LPBK_PATTERN_OF_32_BIT_ZEROES_ONES;

        // continue crc over remaining bytes
        for (UINT32 idx = 0; idx < numRemainingBytes; idx++)
        {
            m_TestPatternmCrc ^= (lastWord & 0xff);
            lastWord >>= 8;

            // invoke the 8-bit parallel CRC expression
            m_TestPatternmCrc  = parCrc8Compute(m_TestPatternmCrc);
        }
    }
    // leave the mCRC value as is, without inverting it, we will keep using
    // it to feed the remaining mCRC for the sequence number incremented in
    // every iteration
}

/////////////////////////////////////////////////////////////////////////////
//  ApexDiagmCrcLoopback::TestNotWaiting()
///
/// @memberof ApexDiagmCrcLoopback
/// @details 
///
/// @param LpBkSize     Loopback packet size.
/// @param LpBkTxAddr   Loopback packet transmit address.
/// @param LpBkRxAddr   Loopback packet receive address.
/////////////////////////////////////////////////////////////////////////////
void ApexDiagmCrcLoopback::TestNotWaiting(UINT32 LpBkSize, void* LpBkTxAddr, void* LpBkRxAddr)
{
    // we are not in one of the WAITING states, so continue the test iteration
    // we are either here for the first time, because we were just initialized or previously not ready),
    // OR the last iteration was a SUCCESS!
    //
    if ((m_mCrcLoopbackState == MCRC_INIT)
            ||
        (m_mCrcLoopbackState == MCRC_NOT_READY)
            ||
        (DEBUG_LOGGING_ENABLED && ((m_mCrcLoopbackState == MCRC_SUCCESS) || (m_mCrcLoopbackState == MCRC_FAIL)))
       )
    {
        PreparePattern(LpBkSize, LpBkTxAddr, LpBkRxAddr);

        printl(__func__, "Size=%d,TxAddr=0x%08x,RxAddr=0x%08x,mCRC=0x%04x",
               LpBkSize, (UINT32)LpBkTxAddr, (UINT32)LpBkRxAddr, m_TestPatternmCrc);
    }
    else if (m_mCrcLoopbackState != MCRC_SUCCESS)
    {
        printl(__func__, "Unexpected State=%d while NotWaiting! %d %d %d",
               m_mCrcLoopbackState);

        if (!DEBUG_LOGGING_ENABLED)
        {
            firmExcept();
        }
    }
    else
    {
        // To avoid MISRA violation
    }

    PreparePacket(LpBkSize, LpBkTxAddr);

    // record start time of this iteration
    m_IterationStartTime = HI_ApexReg.SystemTime;

    printl(__func__, "Starting loopback>State=%d,Seq=%d,m_TxmCrc=0x%04x,Time=%d",
           m_mCrcLoopbackState, m_LoopbackSequeneNumber, m_TxmCrc, HI_ApexReg.SystemTime);

    // tell the ENet/CNet CommPort that the Txbuffer is ready to be transmitted
    SetTxReqstReady();

    // re-schedule at a smaller interval to check for loopback response
    ApexDiagnostic::DgnList[ApexDiagnostic::DGN_MCRC_LOOPBACK_NUM].triggerValueTimeslice = ApexDiagnostic::m_SliceNumber + ApexDiagnostic::MsToDgnSlices(MCRC_LOOPBACK_SHORT_STEP_MS);

    printl(__func__, "Re-schedule:DgnSliceNo=%d TrigVal=%d %d %d",
           ApexDiagnostic::m_SliceNumber, ApexDiagnostic::DgnList[ApexDiagnostic::DGN_MCRC_LOOPBACK_NUM].triggerValueTimeslice);
}

/////////////////////////////////////////////////////////////////////////////
//  ApexDiagmCrcLoopback::CheckResponse()
///
/// @memberof ApexDiagmCrcLoopback
/// @details This function checks a response mCRC packet from the host
/// for validity.
///
/// @param LpBkSize     Loopback packet size.
/// @param LpBkRxAddr   Loopback packet receive address.
/////////////////////////////////////////////////////////////////////////////
void ApexDiagmCrcLoopback::CheckResponse(UINT32 LpBkSize, void* LpBkRxAddr)
{
    // initialize the mCRC to be calculated over the Rx-Buffer
    UINT32 RxCalcmCRC = 0xffff;

    UINT32 numWords = (LpBkSize - 2) / 4;


    // looks like ENet/CNet CommPort has cooperated & performed the loopback :-)
    // now's the time to inspect the Rx Buffer & verify the received mCRC
    //
    // calculate the mCRC over the test pattern portion & the sequence number
    // do words first (we can use the normal parCRC routine, as 32-bit reads
    // to shared memory are allowed)
    //
    ParallelCrc32((UINT32*)LpBkRxAddr, numWords, &RxCalcmCRC);

    UINT32 numRemainingBytes = (LpBkSize - 2) % 4;
    
    // bytes remaining ?
    if (numRemainingBytes > 0)
    {
        // read the last word
        UINT32 lastWord = *((UINT32*)LpBkRxAddr + (LpBkSize - 2) / 4);

        // continue crc over remaining bytes
        for (UINT32 idx = 0; idx < numRemainingBytes; idx++)
        {
            RxCalcmCRC ^= (lastWord & 0xff);
            lastWord >>= 8;

            // invoke the 8-bit parallel CRC expression
            RxCalcmCRC  = parCrc8Compute(RxCalcmCRC);
        }
    }

    // invert the final accumulator value to get the mCRC
    RxCalcmCRC ^= 0xffff;

    // obtain the mCRC sitting in the receive buffer, only 32-bit reads
    // are allowed by Apex BBUS Host Interface, so use SR_UINT8 type
    // to extract the mCRC bytes from the RxBuffer & assemble the 16-bit value
    // RxmCRC is a placeholder for the mCRC to be read from the Rx-Buffer
    UINT16 RxmCRC = ((*(UINT8*)((UINT32)LpBkRxAddr + LpBkSize - 1)) << 8)
            | (*(UINT8*)((UINT32)LpBkRxAddr + LpBkSize - 2));

    // finally, compare all the #3 mCRCs
    // i. the calculated one for Tx buffer
    // ii. the calculated one for Rx buffer
    // iii. the mCRC in the Receive buffer (i.e. part of the looped back packet)

    UINT16 RxLpBkSeqNo = 0;

    if (DEBUG_LOGGING_ENABLED)
    {
        // fetch the sequence number from the end of the RxBuffer
        RxLpBkSeqNo =  ((*(UINT8*)((UINT32)LpBkRxAddr + LpBkSize - 3)) << 8)
                      | (*(UINT8*)((UINT32)LpBkRxAddr + LpBkSize - 4));
    }

    if ((RxmCRC == RxCalcmCRC) && (RxmCRC == m_TxmCrc))
    {
        // all's well that ends well :-)
        m_mCrcLoopbackState = MCRC_SUCCESS;
        printl(__func__, "SUCCESS!mCRC match> SeqNo=%d,RxmCRC=0x%04x,%d,%d",
               RxLpBkSeqNo, RxmCRC);
    }
    else
    {
        // something, somewhere in the Comms/CNet VB has gone wrong
        // (its mCRC diagnostic can't be trusted)
        //
        m_mCrcLoopbackState = MCRC_FAIL;
        printl(__func__, "FAIL!mCRC Mismatch>SeqNo=%d,Tx=0x%04x,Rx=0x%04x,RxCalc=0x%04x",
               RxLpBkSeqNo, m_TxmCrc, RxmCRC, RxCalcmCRC);
    }
}

/////////////////////////////////////////////////////////////////////////////
//  ApexDiagmCrcLoopback::TestResponseAvailable()
///
/// @memberof ApexDiagmCrcLoopback
/// @details This function checks if a response mCRC packet is available.
///
/// @param LpBkSize     Loopback packet size.
/// @param LpBkRxAddr   Loopback packet receive address.
/////////////////////////////////////////////////////////////////////////////
void ApexDiagmCrcLoopback::TestResponseAvailable(UINT32 LpBkSize, void* LpBkRxAddr)
{
    printl(__func__, "loopback Resp Avail>State=%d,Time=%d,%d,%d",
           m_mCrcLoopbackState, HI_ApexReg.SystemTime);

    switch (m_mCrcLoopbackState)
    {
    case MCRC_WAITING_FOR_MCRC_ERROR:
        if (!IsmCRCErrorReported())
        {
            // we were waiting for mCRCError notification, but got something
            // else, so change the state to FAIL
            m_mCrcLoopbackState = MCRC_FAIL;
            printl(__func__, "!!! FAIL !! mCRCError Not Reported %d,%d,%d,%d");
        }
        else
        {
            if (!IsmCRCErrorSeqMatch())
            {
                // we got the mCRCError notification, but does not seem to be for
                // this iteration; may be spurious, but cannot be tolerated!
                m_mCrcLoopbackState = MCRC_FAIL;
                printl(__func__, "!FAIL!mCRCErrorSeqMismatch:LpBkSeq=%d,CtrlSts=0x%08x,%d,%d",
                       m_LoopbackSequeneNumber, HI_ApexParam.mCRC.LpBkCtrlSts);
            }
            else
            {
                // all's well that ends well :-)
                m_mCrcLoopbackState = MCRC_SUCCESS;
                printl(__func__, "SUCCESS!mCRCErrorReported>Seq=%d,CtrlSts=0x%08x,%d,%d",
                       m_LoopbackSequeneNumber, HI_ApexParam.mCRC.LpBkCtrlSts);
            }
        }

        break;

    case MCRC_WAITING_FOR_RX_AVAILABLE:
        if (!IsRxAvailReported())
        {
            // we were waiting for RxAvail notification, but got something
            // else, so change the state to FAIL
            m_mCrcLoopbackState = MCRC_FAIL;
            printl(__func__, "!!! FAIL !! RxAvail Not Reported %d,%d,%d,%d");
        }
        else
        {
            CheckResponse(LpBkSize, LpBkRxAddr);
        }

        break;

    default:
        printl(__func__, "Unexpected State=%d when Resp is Available! %d %d %d",
               m_mCrcLoopbackState, HI_ApexReg.SystemTime);

        firmExcept();

        break;
    }
}

/////////////////////////////////////////////////////////////////////////////
//  ApexDiagmCrcLoopback::CheckState()
///
/// @memberof ApexDiagmCrcLoopback
/// @details This function checks the current state of the diagnostic test
/// and calls a function to take appropriate action.
///
/// @param LpBkSize     Loopback packet size.
/// @param LpBkTxAddr   Loopback packet transmit address.
/// @param LpBkRxAddr   Loopback packet receive address.
/////////////////////////////////////////////////////////////////////////////
void ApexDiagmCrcLoopback::CheckState(UINT32 LpBkSize, void* LpBkTxAddr, void* LpBkRxAddr)
{
    // check if the loopback parameters have been setup already
    if (!
        // Tx Buffer Address is mapped to shared memory
        ((((UINT32)LpBkTxAddr & BSP_APEX_SH_RAM_ADDR) == BSP_APEX_SH_RAM_ADDR)
                &&
        // Rx Buffer Address is mapped to shared memory
        (((UINT32)LpBkRxAddr & BSP_APEX_SH_RAM_ADDR) == BSP_APEX_SH_RAM_ADDR)
                &&
        // Buffer Size is legitimate
        (LpBkSize > 0)
                &&
        (LpBkSize <= MAX_LPBK_BUF_SIZE))
       )
    {
        TestNotSetUp(LpBkSize, LpBkTxAddr, LpBkRxAddr);
    }
    else if (
        m_mCrcLoopbackState != MCRC_WAITING_FOR_MCRC_ERROR
                &&
        m_mCrcLoopbackState != MCRC_WAITING_FOR_RX_AVAILABLE
    )
    {
        TestNotWaiting(LpBkSize, LpBkTxAddr, LpBkRxAddr);
    }
    // if we are here, this means that we are in one of the WAITING states
    // so look for any kind of response that may have come in...
    //
    else if (IsResponseAvailable())
    {
        TestResponseAvailable(LpBkSize, LpBkRxAddr);
    }
    else
    {
        // To avoid MISRA violation
    }
}

/////////////////////////////////////////////////////////////////////////////
//  ApexDiagmCrcLoopback::HandleState()
///
/// @memberof ApexDiagmCrcLoopback
/// @details This function checks where the current diagnostic test's
/// progress is.  It will determine if a test has completed, failed, or is
/// still ongoing.
///
/// @return         ApexDiagnostic::TestState
///     @retval     TEST_LOOP_COMPLETE   if test has finished. 
///     @retval     TEST_IN_PROGRESS     if test is still in progress.
/////////////////////////////////////////////////////////////////////////////
ApexDiagnostic::TestState ApexDiagmCrcLoopback::HandleState()
{
    // time to decide what to do next based on the loopback state determined in the steps above
    if (m_mCrcLoopbackState == MCRC_FAIL)
    {
        // this invariably means we need to assert the FAULT line to the host
        printl(__func__, "LpBk State=mCRC_Fail AssertMNRF @time=%d %d %d %d",
               HI_ApexReg.SystemTime);

        firmExcept();

        return (ApexDiagnostic::TEST_LOOP_COMPLETE);
    }
    else if (m_mCrcLoopbackState == MCRC_SUCCESS)
    {
        printl(__func__, "LpBk State=mCRC_SUCCESS>IterationDone! @time=%d %d %d %d",
               HI_ApexReg.SystemTime);

        if (DEBUG_LOGGING_ENABLED)
        {
            // for unit tests, host is going to set a new loopback size
            // so make it zero here, we will go back to being NOT_READY
            // & repeat the steps again when host sets a new size
            HI_ApexParam.mCRC.LpBkBufSize = 0;
        }
        // otherwise, we wil automatically select the next WAITING state
        // & wait for another response. this should go on indefinitely...

        return (ApexDiagnostic::TEST_LOOP_COMPLETE);
    }
    // check for timeout
    else if ((INT32)(HI_ApexReg.SystemTime - (m_IterationStartTime + MCRC_LOOPBACK_RESPONSE_TIMEOUT_US)) > 0)
    {
        //  timeout expired...We are here because there was actually NO response
        //  again, something, somewhere has gone wrong in the Comms/CNet VB
        printl(__func__, "!!Timeout!!@time=%d,LpBkState=%d> AssertMNRF...%d %d",
               HI_ApexReg.SystemTime, m_mCrcLoopbackState);

        m_mCrcLoopbackState = MCRC_FAIL;

        if (FAULT_INJECTION_SUPPORT_ENABLED)
        {
            // the fault injection build actually needs a fault here to demonstrate that we are able
            // to detect a timeout
            firmExcept();
        }

        if (!DEBUG_LOGGING_ENABLED)
        {
            // only if not unit testing; because there is no way we can completely stop this test
            // once it has been started; so a timeout is inevitable when the unit test stops on
            // the host side
            firmExcept();
        }

        //  we are here ONLY for a debug build, possibly for which the host does not support loopback
        // say because MAC ID is not set or integration test is running
        // returning LOOP_COMPLETE will avoid the completion check from failing
        return (ApexDiagnostic::TEST_LOOP_COMPLETE);
    }
    else
    {
        // To avoid MISRA violation (unreachable statement on the 'break'),
        // save off test state, then return.
        ApexDiagnostic::TestState testState;
        
        // we are in the timing loop, still waiting for the 1st iteration to start or a response to
        // come back for the current iteration
        switch (m_mCrcLoopbackState)
        {
        case MCRC_WAITING_FOR_RX_AVAILABLE:      // pass-through
        case MCRC_WAITING_FOR_MCRC_ERROR:    // pass-through
        case MCRC_NOT_READY:
            // we are just legitimately waiting for a response to come back from
            // the ENet/CNet CommPort. Check again at a shortened scheduled interval
            //
            printl(__func__, "LpBkState=%d while in timing loop@time=%d %d %d",
                   m_mCrcLoopbackState, HI_ApexReg.SystemTime);

            // re-schedule at a smaller interval to check for loopback response
            ApexDiagnostic::DgnList[ApexDiagnostic::DGN_MCRC_LOOPBACK_NUM].triggerValueTimeslice = ApexDiagnostic::m_SliceNumber + ApexDiagnostic::MsToDgnSlices(MCRC_LOOPBACK_SHORT_STEP_MS);

            printl(__func__, "Re-schedule:DgnSliceNo=%d TrigVal=%d %d %d",
                   ApexDiagnostic::m_SliceNumber, ApexDiagnostic::DgnList[ApexDiagnostic::DGN_MCRC_LOOPBACK_NUM].triggerValueTimeslice);

            testState = ApexDiagnostic::TEST_IN_PROGRESS;
            break;

        default:
            // we should not be in the timing loop if our loopback state does not want us to...
            printl(__func__, "Unexpected LpBkState=%d while waiting..@time=%d %d %d",
                   m_mCrcLoopbackState, HI_ApexReg.SystemTime);

            firmExcept();

            testState = ApexDiagnostic::TEST_LOOP_COMPLETE;
            break;
        }
        return testState;
    }
}

/////////////////////////////////////////////////////////////////////////////
//  ApexDiagmCrcLoopback::RunTest()
///
/// @memberof ApexDiagmCrcLoopback
/// @details This function is the runtime backplane loopback test. It will
/// trigger a loopback frame with mCRC included to be transmitted on the
/// ENet/CNet port & check the response conditions to determine whether the
/// test has passed or failed.  If a failure or a timeout occurs, the test
/// will cause an assert by itself.
///
/// @param <anonymous>  Unused.
///
/// @return         ApexDiagnostic::TestState
///     @retval     TEST_LOOP_COMPLETE   if test has finished. 
///     @retval     TEST_IN_PROGRESS     if test is still in progress.
/////////////////////////////////////////////////////////////////////////////
ApexDiagnostic::TestState ApexDiagmCrcLoopback::RunTest(ApexDiagnostic::DiagControlBlock *)
{
    if (MCRC_LOOPBACK_ENABLED)
    {
        printl(__func__, "Iteration>DgnSliceNo=%d, TrigVal=%d %d %d",
               ApexDiagnostic::m_SliceNumber, ApexDiagnostic::DgnList[ApexDiagnostic::DGN_MCRC_LOOPBACK_NUM].triggerValueTimeslice);

        // read the parameters into local variables
        //
        UINT32 LpBkSize   = HI_ApexParam.mCRC.LpBkBufSize;
        void*  LpBkTxAddr = HI_ApexParam.mCRC.TxBufAddr;
        void*  LpBkRxAddr = HI_ApexParam.mCRC.RxBufAddr;

        // there should be no reason to set the buffer size greater than the max
        if (LpBkSize > MAX_LPBK_BUF_SIZE)
        {
            firmExcept();
        }

        CheckState(LpBkSize, LpBkTxAddr, LpBkRxAddr);

        return (HandleState());
    }
    else
    {
        // mCRC loopback diagnostic is not supported
        return (ApexDiagnostic::TEST_LOOP_COMPLETE);
    }
}

/////////////////////////////////////////////////////////////////////////////
//  ApexDiagmCrcLoopback::ParallelCrc32()
///
/// @memberof ApexDiagmCrcLoopback
/// @details This function continues the input 16-bit (CNet based) CRC over a
/// block of data words anywhere in internal SRAM or shared memory (provided
/// the addresses are word aligned).  Usually not called independently. Call
/// ComputeParallelCrc instead which will call this function, if required.
///
/// @param pData    Pointer to the data to run the CRC on.
/// @param size     Size to run the CRC on.
/// @param pCrc     Pointer to where the CRC is stored.
/////////////////////////////////////////////////////////////////////////////
void ApexDiagmCrcLoopback::ParallelCrc32(UINT32* pData, UINT16 size, UINT32* pCrc)
{
	if (((UINT32)pData & 3) != 0)
    {
		firmExcept();
    }

	UINT32 acc = *pCrc;
	
	for(UINT16 i=0; i<size; i++)
	{
		acc ^= pData[i];		
		// evaluate the parallel CRC expression for 32-bit data
		acc = parCrc32Compute(acc);	
	}
	
	printl((const char *)__func__, "pSrc=0x%08x,Size=%d,acc=0x%08x %d", (UINT32)pData,size,acc);	
	
	*pCrc = acc;
}

/////////////////////////////////////////////////////////////////////////////
//  ApexDiagmCrcLoopback::ParallelCrc8()
///
/// @memberof ApexDiagmCrcLoopback
/// @details This function continues the input 16-bit (CNet based) CRC over a
/// block of data bytes anywhere in internal/shared SRAM. Usually not called
/// independently. Call ComputeParallelCrc instead which will call this
/// function, if required.
///
/// @param pData    Pointer to the data to run the CRC on.
/// @param size     Size to run the CRC on.
/// @param pCrc     Pointer to where the CRC is stored.
/////////////////////////////////////////////////////////////////////////////
void ApexDiagmCrcLoopback::ParallelCrc8(UINT8* pData, UINT16 size, UINT32* pCrc)
{
	UINT32 acc = *pCrc;
		
	for(UINT16 i=0; i<size; i++)
	{
		acc ^= pData[i];		
		// evaluate the parallel CRC expression for 8-bit data
		acc = parCrc8Compute(acc);	
	}
	
	printl((const char *)__func__, "pSrc=0x%08x,Size=%d,acc=0x%08x %d", (UINT32)pData,size,acc);	
	
	*pCrc = acc;
}

/////////////////////////////////////////////////////////////////////////////
//  ApexDiagmCrcLoopback::ComputeParallelCrc()
///
/// @memberof ApexDiagmCrcLoopback
/// @details This function calculates a 16-bit (CNet based) CRC over a block
/// of data anywhere in internal/shared SRAM.
///
/// @param pSrc     Pointer to the data to run the CRC on.
/// @param size     Size to run the CRC on.
/////////////////////////////////////////////////////////////////////////////
UINT16 ApexDiagmCrcLoopback::ComputeParallelCrc(void* pSrc, UINT16 size)
{
	UINT32 crc = 0xffff;
	
	// run the CRC calculation over words
	ParallelCrc32((UINT32*)pSrc, size/4, &crc);
	
	// run the CRC calculation over remaining bytes
	ParallelCrc8 ((UINT8*)((UINT32*)pSrc+(size/4)), size%4, &crc);
	
	printl((const char *)__func__, "pSrc=0x%08x,Size=%d,Crc=0x%08x %d", (UINT32)pSrc, size, (crc ^ 0xffff));		
	
	return (crc ^ 0xffff);
}

/////////////////////////////////////////////////////////////////////////////
//  ApexDiagmCrcLoopback::ContinueParallelCrcComputation()
///
/// @memberof ApexDiagmCrcLoopback
/// @details This function continues the input 16-bit (CNet based) CRC over a
/// block of data.  Note that the final value is not inverted.
///
/// @param pSrc     Pointer to the data to run the CRC on.
/// @param size     Size to run the CRC on.
/// @param pCrc     Pointer to where the CRC is stored.
/////////////////////////////////////////////////////////////////////////////
void ApexDiagmCrcLoopback::ContinueParallelCrc(void* pSrc, UINT16 size, UINT32* pCrc)
{
	UINT32 crc = *pCrc;
	
	// run the CRC calculation over words
	ParallelCrc32((UINT32*)pSrc, size/4, &crc);
	
	// run the CRC calculation over remaining bytes	
	if (size%4)
	{
		// read the last word
		UINT32 lastWord = *((UINT32*)pSrc + (size/4));
							
		// continue crc over remaining bytes
		for (UINT32 idx=0; idx < size%4; idx++)
		{
			crc ^= (lastWord & 0xff);
			lastWord >>= 8;
								
			// invoke the 8-bit parallel CRC expression
			crc = parCrc8Compute(crc);
		}
	}
	
	printl((const char *)__func__, "pSrc=0x%08x,Size=%d,Crc=0x%08x %d", (UINT32)pSrc,size,crc);		
	
	*pCrc = crc;
}

/////////////////////////////////////////////////////////////////////////////
//  ApexDiagmCrcLoopback::VerifyParallelCrc()
///
/// @memberof ApexDiagmCrcLoopback
/// @details This function verifies the parallel crc calculation routines.
///
/// @param pSrc     Pointer to the data to run the CRC on.
/// @param size     Size to run the CRC on.
/// @param HostCrc  Host CRC value.
/////////////////////////////////////////////////////////////////////////////
void ApexDiagmCrcLoopback::VerifyParallelCrc(void* pSrc, UINT32 size, UINT32 HostCrc)
{
	if (DEBUG_LOGGING_ENABLED)
	{
		UINT32 crc;
		
		crc = ComputeParallelCrc(pSrc, size);
		
		if (crc != HostCrc)
        {
			printl (__func__, "!ParCRC FAIL!pSrc=0x%08x,size=%d,CRCs=0x%04x,0x%04x", (UINT32)pSrc, size, crc, HostCrc);
        }
		else
        {
			printl (__func__, "!ParCRC PASS!pSrc=0x%08x,size=%d,CRCs=0x%04x,0x%04x", (UINT32)pSrc, size, crc, HostCrc);
        }
		
		crc = 0xffff;	
		ContinueParallelCrc(pSrc, size, &crc);	
		crc ^= 0xffff;
		
		if (crc != HostCrc)
        {
			printl (__func__, "!ParCrcContFAIL!pSrc=0x%08x,size=%d,CRCs=0x%04x,0x%04x", (UINT32)pSrc, size, crc, HostCrc);
        }
		else
        {
			printl (__func__, "!ParCrcContPASS!pSrc=0x%08x,size=%d,CRCs=0x%04x,0x%04x", (UINT32)pSrc, size, crc, HostCrc);
        }
	}
}

