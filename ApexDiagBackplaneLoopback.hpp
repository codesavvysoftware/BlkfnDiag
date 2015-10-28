////////////////////////////////////////////////////////////////////////////////
/// @file ApexDiagBackplaneLoopback.hpp
///
/// @details Apex backplane loopback diagnostic class diagnostic definition.
///
/// @if REVISION_HISTORY_INCLUDED
/// @par Edit History
/// - dtstalte  03-DEC-2013  Created.
/// - akkudva   16-JAN-2014  Added new class members/constants to synchronize 
///                          the wait for CBus Tx to become available
/// - dtstalte  29-JAN-2014  MISRA fixes after resolving Lgx00147826.
/// @endif
///
/// @par Copyright (c) 2014 Rockwell Automation Technologies, Inc. All rights reserved.
////////////////////////////////////////////////////////////////////////////////
#ifndef APEXDIAGBACKPLANELOOPBACK_HPP
#define APEXDIAGBACKPLANELOOPBACK_HPP

// SYSTEM INCLUDES
// <none>

// C PROJECT INCLUDES
// <none>

// C++ PROJECT INCLUDES
#include "ApexDiagnostic.hpp"       // for DiagControlBlock
#include "ApexStdType.hpp"          // for UINT types
#include "Apex.hpp"                 // for firmExcept()

// FORWARD REFERENCES
// (none)

////////////////////////////////////////////////////////////////////////////////
/// @class ApexDiagBackplaneLoopback
///
/// @ingroup Diagnostics
///
/// @brief This class implements the APEX loopback diagnostic including
/// initialization, power-up, and runtime components.
////////////////////////////////////////////////////////////////////////////////
class ApexDiagBackplaneLoopback
{
public:

    // Keeps track of which loopback image is being used
    enum ImageSelector
    {   
        IMAGE1 = 0,
        IMAGE2 = 1
    };
    
    // The various states for the loopback test
    enum LoopbackState
    {
        LOOPBACK_INIT = 1,
        LOOPBACK_NOT_READY,
        LOOPBACK_WAITING,
        LOOPBACK_SUCCESS,
        LOOPBACK_RETRY,
        LOOPBACK_COMPLETE
    };
    
    // PUBLIC METHODS
    
    /// Initialize the diagnostic.  Called at power up.
    static void Init();
    
    /// The primary body of the run time diagnostic test.
    static ApexDiagnostic::TestState RunTest(ApexDiagnostic::DiagControlBlock *);
    
    // inline functions used by ControlBus to communicate state changes
    inline static LoopbackState GetTestState()                          { return m_LoopbackState; }                   
    inline static void          SetTestState(LoopbackState state)       { m_LoopbackState = state; }
    inline static void          SetCorruptCrc(volatile bool bCorrupt)   { m_bCorruptLoopbackCrc = bCorrupt; }
    inline static void          SetSkipTxRetry(volatile bool bRetry)    { m_bSkipLoopbackTxRetry = bRetry; }
    
    // inline functions to get & clear the state of CBus Tx Not Available Flag
    inline static bool          IsTxNotAv()                             { return m_bIsTxNotAv; }    
    inline static void          ResetTxNotAv()                            
    { 
        if (m_bIsTxNotAv)
        {
            m_bIsTxNotAv = false;
        }
        else
        {
            firmExcept();
        }
    } 

protected:
    // PROTECTED METHODS

private:
    // PRIVATE METHODS
    
    /// Checks if an initiated loopback test has completed yet.
    static ApexDiagnostic::TestState CheckForCompletion();
    
    /// Checks if the loopback test is setup and ready.
    static void CheckForTestReady();
    
    /// Checks if a frame header has been corrupted
    static void CheckFrameHeader();
    
    /// Calculate and store the CRC on a loopback frame.
    static void CrcCopy8();
    
    /// Check if there is a loopback test in progress.
    static ApexDiagnostic::TestState ExecuteLoopback();
    
    /// Setup things for the first loopback test run.
    static void SetupFirstIteration();
    
    /// Called once the loopback frame completes transmission
    static void XmitFrameEnd();
    
    /// Called by ExecuteLoopback() to make sure CBus Tx is available
    static void WaitForTxAvAndProceed();
    
    /// Called by WaitForTxAvAndProceed to actually check Tx
    static void CheckIfTxIsAvailable();

    // called whenever CBus Tx is found to be unavailable
    inline static void SetTxNotAv()                            
    { 
        if (!m_bIsTxNotAv)
        {
            m_bIsTxNotAv = true;
        }
        else
        {
            firmExcept();
        }
    }
    
    // non-time dependent delay loop based on CPU cycles
    // call within a time-based loop to avoid reading the 
    // systime register continuously
    inline static void NopDelay()
    {
        // the "nop" is used just as a filler instruction 
        // for the body of a loop; based on however many CPU 
        // clocks we need to delay, the number of NOP asm 
        // instances may be regulated via a loop control variable
        asm ("nop");
    }
    
    /// Default constructor.
    ApexDiagBackplaneLoopback();
    
    /// Default destructor.
    ~ApexDiagBackplaneLoopback();

    /// Copy constructor and assignment operator not implemented.
    ApexDiagBackplaneLoopback(const ApexDiagBackplaneLoopback &);
    ApexDiagBackplaneLoopback &operator=(const ApexDiagBackplaneLoopback &);

    // MEMBER VARIABLES
    
    // Keep track of the current image.
    static ImageSelector m_ImgSelector;
    
    // Keep track of test state.
    static volatile LoopbackState m_LoopbackState;
    
    // Flag for turning ON patch code for fault injection tests
    static volatile bool m_bCorruptLoopbackCrc;
    
    // Indicates whether loopback retry should occur or not.
    static volatile bool m_bSkipLoopbackTxRetry;
    
    // Indicates whether the loopback diagnostic is waiting for TxAv
    static volatile bool m_bIsTxNotAv;
    
    static const UINT32 BP_LOOPBACK_SHORT_STEP_MS = 500;

    // Loopback Test Pattern for non-header frame words in the Tx-FIFO is defined as alternating
    // pattern of 0s & 1s:
    static const UINT32 LOOP_BACK_PATTERN_OF_32_BIT_ZEROES_ONES  = 0x55555555;
    static const UINT32 LOOP_BACK_PATTERN_OF_32_BIT_ONES_ZEROES  = 0xAAAAAAAA;

    // Loopback Test Pattern for the frame header is defined as follows:
    // |e|30|29|.|.|.|.|.|.|.|.|.|.|.|.|.|z|z|z|c|c|c|c|c|z|s|d|d|d|d|d|d|
    // |0 |-----------CB ID--------------|0 0 0 0 1 0 1 0|0 0 0 0 0 0 0 0|
    // |1 |-----------CB ID--------------|0 0 0 1 0 1 0 1|0 1 0 0 0 0 0 0|
    // |--------- 0x5555 or 0xAAAA ------|--0x0A or 0x15-|--0x00 or 0x40-|
    // KEYs
    // e: EoM flag (may be 0 or 1)
    // CBID: (may be 0 or 1)
    // z: Always 0
    // s: SoM flag (may be 0 or 1)
    // c: command (may be 0 or 1)
    // d: destination slot field
    static const UINT32 LOOP_BACK_PATTERN_OF_ZEROES_ONES = 0x55550A00;
    static const UINT32 LOOP_BACK_PATTERN_OF_ONES_ZEROES = 0xAAAA1540;
    
    // Flag to remember if the loopback was ever started. It cannot be
    // started unless the host tells us to, so initialize as false.
    static bool     m_bIsBpLoopBackStarted;
    
    // Storage for remembering the start time of a new test iteration.
    static UINT32   m_IterationStartTime;
    
    // Timeouts -- account for following possibilities:
    // i.  Power-up delay to start these tests from the host side
    // ii. In case of mCRC, delay for the CNet/ENet CommPort to loopback the response
    // iii. In case of BP, delay for the Apex to loopback the response vis-a-vis backplane traffic load
    // 5 mins should be long enough for all of these cases! 
    // If a timeout occurs, Apex will assert the fault line to the host.
    static const UINT32 BACKPLANE_LOOPBACK_RESPONSE_TIMEOUT_US
    
#if defined(SUPPORT_FAULT_INJECTION)
    // Use a longer timeout to allow time for fault injection
    // procedure to complete before the timeout occurs.
    = 60000000;     // 1 mins  
#else
    = 300000000;    // 5 min  
#endif
#if defined(DEBUG)
    static const UINT32 BACKPLANE_TX_AV_TIMEOUT_US = 1000;
#else    
    // timeout while waiting for CBus to become available
    static const UINT32 BACKPLANE_TX_AV_TIMEOUT_US = 500;
#endif    
    
    // spin count for the NOP loop called while waiting for Tx to become available
    static const UINT32 BACKPLANE_TXAV_SPIN_NOP_CNT = 10;
    
    // spin count for the NOP loop called just after firing off Tx on the backplane
    // to wait for TXAV to get cleared
    static const UINT32 BACKPLANE_TXAV_DELAY_NOP_CNT = 1;
};

#endif // #if !defined(APEXDIAGBACKPLANELOOPBACK_HPP)
