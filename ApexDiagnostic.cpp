/////////////////////////////////////////////////////////////////////////////
/// @file ApexDiagnostic.cpp
///
/// Implements the diagnostic services for the APEX ASIC which include
/// powerup and runtime diagnostics, initialization, and scheduling.
///
/// @if REVISION_HISTORY_INCLUDED
/// @par Edit History
/// - [0]  WBC  01-MAR-2005 Initial revision of file.
/// - [1]  DJD  13-FEB-2009 Lgx00094867 Clear the scrub indication in the
///                         hardware.
/// - [2]  IFL  19-APR-2012 Added HOST_ICE2 flag for ICE2 chip.
/// - [3]  AKK  28-JUN-2013 Initialize loopback & enable debug logging.
/// - [4]  FiZ  10-JUL-2013 Diagnostics completion time checking.
/// - [5]  SP   29-JUL-2013 ECC diagnostic test enabled on ICE2.
/// - [6]  SP   08-AUG-2013 ECC error scrubbing moved to data RAM diagnostic.
/// - [7]  SP   13-AUG-2013 Checksum diagnostic replaced with CRC
/// - [8]  FiZ  23-AUG-2013 ARM instructions and registers diagnostics.
/// - [9]  WMP  17-SEP-2013 Added Apex2 Timer Diagnostics including changes
///                         per Collaborator code review #25440.
/// - [10] WMP  08-OCT-2013 Moved Fault Injection Point to under DGN_TIMER
///                         initialization in DgnSchedInit() to make it
///                         contiguous.
/// - [11] WMP 21-OCT-2013  Renumbered DGN_TIMER .triggerValueTimeslice and
///                         .stepValueTimeslice fault injection point to
///                         accommodate numbering of DGN_EXE_CRC_NUM
///                         .timeout_slices fault injection point.
/// - [12] FiZ 21-OCT-2013  ARM cache diagnostics.
/// - [13] WMP 22-OCT-2013  Updated the comments for the DGN_TIMER
///                         .triggerValueTimeslice and .stepValueTimeslice
///                         fault injection point.
/// - [14] SP  23-OCT-2013  Io shutdown diagnostics initialization added.
/// - [15] SP  02-NOV-2013  Timeout_slices value for mCRC and timer diags
///                         corrected.
/// - [16] WMP 03-NOV-2013  Added delaying Apex diagnostics startup fault
///                         injection point.
/// - [17] FiZ 05-NOV-2013  Fault injection fixes.
/// - mgrad    25-NOV-2013  File name changed during MISRA fixes.
/// - spolke   27-NOV-2013  MISRA fixes.
/// - pszramo  27-NOV-2013  Refactoring to reduce DgnSched() complexity.
/// - dtstalte 03-DEC-2013  More MISRA/coding standard changes.
/// - pszramo  16-DEC-2013  Updated fault injection points according to
///                         Coverity defects triage.
/// - dtstalte 27-JAN-2014  Remove cache diagnostic completion check since it
///                         is now interrupt driven.
/// - mstasia  05-FEB-2014  Lgx00148467: Added initialization of global variable.
/// @endif
///
/// @par Copyright (c) 2014 Rockwell Automation Technologies, Inc. All rights reserved.
/////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
// <none>

// C PROJECT INCLUDES
// <none>

// C++ PROJECT INCLUDES
#include "ApexDiagnostic.hpp"                       // for ApexDiagnostic
#include "ApexDiagArm.hpp"                          // for ApexDiagArm
#include "ApexDiagArmCache.hpp"                     // for ApexDiagArmCache
#include "ApexDiagBackplaneLoopback.hpp"            // for ApexDiagBackplaneLoopback
#include "ApexDiagBinaryCrc.hpp"                    // for ApexDiagBinaryCrc
#include "ApexDiagmCrcLoopback.hpp"                 // for ApexDiagmCrcLoopback
#include "ApexDiagEcc.hpp"                          // for ApexDiagEcc

#ifdef SIL2
#include "ApexDiagIoShutdown.hpp"                   // for ApexDiagIoShutdown
#else
#include "ApexDiagIoShutdownStub.hpp"               // for ApexDiagIoShutdown
#endif

#include "ApexDiagRamAddress.hpp"                   // for ApexDiagRamAddress
#include "ApexDiagRamData.hpp"                      // for ApexDiagRamData
#include "ApexDiagSafeRam.hpp"                      // for ApexDiagSafeRam
#include "ApexDiagTimer.hpp"                        // for ApexDiagTimer

#include "ApexStd.hpp"                              // for STATIC_ASSERT
#include "ApexRegisters.hpp"                        // for HI_ApexReg
#include "ApexWatchdog.hpp"                         // for ApexWatchdog

// FORWARD REFERENCES
// (none)

// Class variable definition
ApexDiagnostic::TraceLog ApexDiagnostic::m_TraceLog[ApexDiagnostic::TRACE_LOG_SIZE] = {0};
ApexDiagnostic::DiagControlBlock ApexDiagnostic::DgnList[ApexDiagnostic::DGN_NUM_RUNTIME_TESTS] = {0};

UINT32 ApexDiagnostic::m_LastDiagTime = 0;
UINT64 ApexDiagnostic::m_LastCompletionCheck = 0;
UINT64 ApexDiagnostic::m_SliceNumber = 0;

// Start Fault Injection Point 1
// Definition of global UINT32 variable InjectFaultFlag used to delay
// the injected fault until run-time will be injected here.
// End Fault Injection Point 1

/////////////////////////////////////////////////////////////////////////////
//  ApexDiagnostic::LogDgnTrace
//
/// Logging function storing passed value with the times it was called at.
/// Diagnostic tracer to help debug watchdog problems on CN2/B.
/////////////////////////////////////////////////////////////////////////////
void ApexDiagnostic::LogDgnTrace(const UINT32 val)
{
    static UINT32 index = 0;

    if (DEBUG_LOGGING_ENABLED)
    {
        if ( index > (UINT32)((sizeof( DgnTrace ) / 4) - 4) )
        {
            index = 0;
        }
        DgnTrace[ index++ ] = HI_ApexReg.SystemTime;
        DgnTrace[ index++ ] = val;
    }
    else
    {
        if ( index >= TRACE_LOG_SIZE )
        {
            index = 0;
        }
    
        m_TraceLog[ index ].timestamp = HI_ApexReg.SystemTime;
        m_TraceLog[ index ].value = val;
        index++;
    }
}

/////////////////////////////////////////////////////////////////////////////
//  ApexDiagnostic::FindTestIndexForThisSlice
//
/// Find either a test scheduled for this slice or the last test in the list.
/////////////////////////////////////////////////////////////////////////////
UINT8 ApexDiagnostic::FindTestIndexForThisSlice()
{
    UINT8 dgnIndex = 0;

    while ((dgnIndex < (DGN_NUM_RUNTIME_TESTS - 1))
            && (m_SliceNumber != DgnList[dgnIndex].triggerValueTimeslice))
    {
        dgnIndex++;
    }

    return dgnIndex;
}

/////////////////////////////////////////////////////////////////////////////
//  ApexDiagnostic::RescheduleLowerPriorityDiagnostics
//
/// Reschedule any lower priority diagnostics that were also scheduled for this
/// slice by bumping their trigger values to the next slice number.
/////////////////////////////////////////////////////////////////////////////
void ApexDiagnostic::RescheduleLowerPriorityDiagnostics(UINT8 dgnIndex, DiagControlBlock *dgnCurDiagCB)
{
    dgnIndex++;
    dgnCurDiagCB++;

    while (dgnIndex < DGN_NUM_RUNTIME_TESTS)
    {
        if (m_SliceNumber == dgnCurDiagCB->triggerValueTimeslice)
        {
            dgnCurDiagCB->triggerValueTimeslice++;
        }
        dgnIndex++;
        dgnCurDiagCB++;
    }
}

/////////////////////////////////////////////////////////////////////////////
//  ApexDiagnostic::Scheduler
//
/// This function handles the scheduling of the various runtime diagnostic
/// functions.
/////////////////////////////////////////////////////////////////////////////
void ApexDiagnostic::ExecuteSlice()
{
    LogDgnTrace( 0xbbbbbbbb );

    // Do hardware watchdog handing
    ApexWatchdog::Service();

    UINT8 testIndex = ApexDiagnostic::FindTestIndexForThisSlice();

    LogDgnTrace( testIndex );

    // Assign pointer to found Diag CB
    DiagControlBlock *pCurrentDiagCB = &DgnList[testIndex];    // Pointer to the currently referenced Diag CB

    // Schedule next trigger slice for this diag. UINT math handles
    // rollover. In some cases, this trigger value will be overwritten
    // from within the test function that is called.  That's why we do the
    // reschedule before we enter the function.
    pCurrentDiagCB->triggerValueTimeslice += pCurrentDiagCB->stepValueTimeslice;

    // Record the entry time before calling diag
    UINT32 entryTimeUs = HI_ApexReg.SystemTime;

    // Call the test function pointed to in Diag CB
    TestState testResult = (TestState)pCurrentDiagCB->diagFunc(pCurrentDiagCB);

    switch (testResult)
    {
        case TEST_LOOP_COMPLETE:
        {
            // If the test has completed all possible iterations, figure out
            // how long it's been since the last total completion.
            UINT32 timeToCompleteTimeslice = m_SliceNumber - pCurrentDiagCB->lastCompleteTimeslice;

            if ((timeToCompleteTimeslice > pCurrentDiagCB->maxTimeslices)
                && (pCurrentDiagCB->oneComplete == true))
            {
                // Store the time if it exceeds the time already stored, and
                // the diagnostic has run at least one previous time
                pCurrentDiagCB->maxTimeslices = timeToCompleteTimeslice;
            }

            pCurrentDiagCB->oneComplete = true;
            pCurrentDiagCB->lastCompleteTimeslice = m_SliceNumber;
        }
        // (Fall through)

        case TEST_IN_PROGRESS:
        {
            // If the test has successfully run to the timelimit within the
            // timeslice, but has not completed all iterations, don't bother
            // with the maximum time calculations.  However, for debugging purposes,
            // save the amount of time that the diagnostic function took to run
            // during this timeslice into its diag CB.
            pCurrentDiagCB->durationUs = HI_ApexReg.SystemTime - entryTimeUs;
            break;
        }

        default:
        {
            // Call HW_HandleException for any other responses
            firmExcept();
            break;
        }
    };

    RescheduleLowerPriorityDiagnostics(testIndex, pCurrentDiagCB);

    LogDgnTrace( 0xeeeeeeee );

    // Update diag slice number.
    m_SliceNumber++;
}


/////////////////////////////////////////////////////////////////////////////
//  ApexDiagnostic::CompletionTimeCheck
//
/// This function checks if all diagnostic tests have been run in time.
///
/////////////////////////////////////////////////////////////////////////////
void ApexDiagnostic::CompletionTimeCheck()
{
    for (UINT8 dgnIndex = 0; dgnIndex < DGN_NUM_RUNTIME_TESTS; ++dgnIndex)
    {
        if ((m_SliceNumber - DgnList[dgnIndex].lastCompleteTimeslice) > DgnList[dgnIndex].timeoutTimeslice)
        {
            firmExcept();
        }
    }

    if (!ApexDiagIoShutdown::CheckCompletionTime())
    {
        firmExcept();
    }
    return;
}

/////////////////////////////////////////////////////////////////////////////
// ApexDiagnostic::SchedulerInit
//
/// This function initializes runtime diagnostic scheduler variables and
/// structures.
/////////////////////////////////////////////////////////////////////////////
void ApexDiagnostic::SchedulerInit()
{
    m_SliceNumber = 0;
    m_LastCompletionCheck = 0;

    /* Initialize global runtime diagnostic control blocks */
    /* Set all triggerValueTimeslices to 0 to schedule a test ASAP for each */
    DgnList[DGN_RAM_ADDR_NUM].diagFunc = ApexDiagRamAddress::RunTest;
    DgnList[DGN_RAM_ADDR_NUM].triggerValueTimeslice = 0;
    DgnList[DGN_RAM_ADDR_NUM].stepValueTimeslice = MsToDgnSlices(DGN_RAM_ADDR_STEP_MS);
    DgnList[DGN_RAM_ADDR_NUM].lastCompleteTimeslice = 0;
    DgnList[DGN_RAM_ADDR_NUM].durationUs = 0;
    DgnList[DGN_RAM_ADDR_NUM].maxTimeslices = 0;
    DgnList[DGN_RAM_ADDR_NUM].oneComplete = false;
    DgnList[DGN_RAM_ADDR_NUM].timeoutTimeslice = DGN_INTERVALS_PER_HOUR * 8;

    DgnList[DGN_SAFE_RAM_NUM].diagFunc = ApexDiagSafeRam::Check;
    DgnList[DGN_SAFE_RAM_NUM].triggerValueTimeslice = 0;
    // Start Fault Injection Point 4
    // Code which sets DgnList[DGN_SAFE_RAM_NUM].stepValueTimeslice to a smaller value
    // to make the test execute earlier will be injected here.
    DgnList[DGN_SAFE_RAM_NUM].stepValueTimeslice = MsToDgnSlices(DGN_SAFE_RAM_STEP_MS);
    // End Fault Injection Point 4
    DgnList[DGN_SAFE_RAM_NUM].lastCompleteTimeslice = 0;
    DgnList[DGN_SAFE_RAM_NUM].durationUs = 0;
    DgnList[DGN_SAFE_RAM_NUM].maxTimeslices = 0;
    DgnList[DGN_SAFE_RAM_NUM].oneComplete = false;
    DgnList[DGN_SAFE_RAM_NUM].timeoutTimeslice = DGN_INTERVALS_PER_HOUR * 8;

    DgnList[DGN_TIMER].diagFunc = ApexDiagTimer::RunTest;
    // Start Fault Injection Point 7
    // Detects Apex2 Timer Diagnostic bad elapsed time fault when Apex compares wrong values.
    // Set the DGN_TIMER .stepValueTimeslice to a different value than DGN_TIMER_STEP_MS.
    // Apex2 Timer Diagnostic expects an elapsed time corresponding to DGN_TIMER_STEP_MS within
    // DGN_TIMER_TOLERANCE_PERCENT so changing DGN_TIMER .stepValueTimeslice to be different from
    // DGN_TIMER_STEP_MS by more than DGN_TIMER_TOLERANCE_PERCENT will cause this fault.
    // The triggerValueTimeslice is also changed to speed up fault injection testing.
    DgnList[DGN_TIMER].triggerValueTimeslice = MsToDgnSlices(DGN_TIMER_INITIAL_TRIG_VAL_MS);
    DgnList[DGN_TIMER].stepValueTimeslice = MsToDgnSlices(DGN_TIMER_STEP_MS);
    // End Fault Injection Point 7
    DgnList[DGN_TIMER].lastCompleteTimeslice = 0;
    DgnList[DGN_TIMER].durationUs = 0;
    DgnList[DGN_TIMER].maxTimeslices = 0;
    DgnList[DGN_TIMER].oneComplete = false;
    DgnList[DGN_TIMER].timeoutTimeslice = DGN_INTERVALS_PER_HOUR * 8;

    DgnList[DGN_EXE_CRC_NUM].diagFunc = ApexDiagBinaryCrc::RunTest;
    DgnList[DGN_EXE_CRC_NUM].triggerValueTimeslice = 0;
    DgnList[DGN_EXE_CRC_NUM].stepValueTimeslice = MsToDgnSlices(DGN_EXE_CRC_STEP_MS);
    DgnList[DGN_EXE_CRC_NUM].lastCompleteTimeslice = 0;
    DgnList[DGN_EXE_CRC_NUM].durationUs = 0;
    DgnList[DGN_EXE_CRC_NUM].maxTimeslices = 0;
    DgnList[DGN_EXE_CRC_NUM].oneComplete = false;
    // Start Fault Injection Point 5
    // Code which sets DgnList[DGN_EXE_CRC_NUM].timeoutTimeslice to zero to make
    // completion time check report fault at its next run will be injected here.
    DgnList[DGN_EXE_CRC_NUM].timeoutTimeslice = DGN_INTERVALS_PER_HOUR * 8;
    // End Fault Injection Point 5

    DgnList[DGN_ECC_NUM].diagFunc = ApexDiagEcc::RunTest;
    DgnList[DGN_ECC_NUM].triggerValueTimeslice = 0;
    DgnList[DGN_ECC_NUM].stepValueTimeslice = DGN_ECC_STEP_TIMESLICE;
    DgnList[DGN_ECC_NUM].lastCompleteTimeslice = 0;
    DgnList[DGN_ECC_NUM].durationUs = 0;
    DgnList[DGN_ECC_NUM].maxTimeslices = 0;
    DgnList[DGN_ECC_NUM].oneComplete = false;
    DgnList[DGN_ECC_NUM].timeoutTimeslice = DGN_INTERVALS_PER_HOUR * 8;

    DgnList[DGN_RAM_DATA_NUM].diagFunc = ApexDiagRamData::RunTest;
    DgnList[DGN_RAM_DATA_NUM].triggerValueTimeslice = 0;
    DgnList[DGN_RAM_DATA_NUM].stepValueTimeslice = MsToDgnSlices(DGN_RAM_DATA_STEP_MS);
    DgnList[DGN_RAM_DATA_NUM].lastCompleteTimeslice = 0;
    DgnList[DGN_RAM_DATA_NUM].durationUs = 0;
    DgnList[DGN_RAM_DATA_NUM].maxTimeslices = 0;
    DgnList[DGN_RAM_DATA_NUM].oneComplete = false;
    DgnList[DGN_RAM_DATA_NUM].timeoutTimeslice = DGN_INTERVALS_PER_HOUR * 8;

    DgnList[DGN_ARM_NUM].diagFunc = ApexDiagArm::RunTest;
    DgnList[DGN_ARM_NUM].triggerValueTimeslice = 0;
    // Start Fault Injection Point 6
    // Code which sets DgnList[DGN_ARM_NUM].stepValueTimeslice to a smaller value
    // to make the test execute earlier will be injected here.
    DgnList[DGN_ARM_NUM].stepValueTimeslice = MsToDgnSlices(DGN_ARM_STEP_MS);
    // End Fault Injection Point 6
    DgnList[DGN_ARM_NUM].lastCompleteTimeslice = 0;
    DgnList[DGN_ARM_NUM].durationUs = 0;
    DgnList[DGN_ARM_NUM].maxTimeslices = 0;
    DgnList[DGN_ARM_NUM].oneComplete = false;
    DgnList[DGN_ARM_NUM].timeoutTimeslice = DGN_INTERVALS_PER_HOUR * 8;

    DgnList[DGN_BP_LOOPBACK_NUM].diagFunc = ApexDiagBackplaneLoopback::RunTest;
    DgnList[DGN_BP_LOOPBACK_NUM].triggerValueTimeslice = 0;
    DgnList[DGN_BP_LOOPBACK_NUM].stepValueTimeslice = MsToDgnSlices(DGN_BP_LOOPBACK_STEP_MS);
    DgnList[DGN_BP_LOOPBACK_NUM].lastCompleteTimeslice = 0;
    DgnList[DGN_BP_LOOPBACK_NUM].durationUs = 0;
    DgnList[DGN_BP_LOOPBACK_NUM].maxTimeslices = 0;
    DgnList[DGN_BP_LOOPBACK_NUM].oneComplete = false;
    DgnList[DGN_BP_LOOPBACK_NUM].timeoutTimeslice = DGN_INTERVALS_PER_HOUR * 8;

    DgnList[DGN_MCRC_LOOPBACK_NUM].diagFunc = ApexDiagmCrcLoopback::RunTest;
    DgnList[DGN_MCRC_LOOPBACK_NUM].triggerValueTimeslice = 0;
    DgnList[DGN_MCRC_LOOPBACK_NUM].stepValueTimeslice = MsToDgnSlices(DGN_MCRC_LOOPBACK_STEP_MS);
    DgnList[DGN_MCRC_LOOPBACK_NUM].lastCompleteTimeslice = 0;
    DgnList[DGN_MCRC_LOOPBACK_NUM].durationUs = 0;
    DgnList[DGN_MCRC_LOOPBACK_NUM].maxTimeslices = 0;
    DgnList[DGN_MCRC_LOOPBACK_NUM].oneComplete = false;
    DgnList[DGN_MCRC_LOOPBACK_NUM].timeoutTimeslice = DGN_INTERVALS_PER_HOUR * 8;

    DgnList[DGN_ARM_CACHE_NUM].diagFunc = ApexDiagArmCache::RunTest;
    DgnList[DGN_ARM_CACHE_NUM].triggerValueTimeslice = 0;
    DgnList[DGN_ARM_CACHE_NUM].stepValueTimeslice = MsToDgnSlices(DGN_ARM_CACHE_STEP_MS);
    DgnList[DGN_ARM_CACHE_NUM].lastCompleteTimeslice = 0;
    DgnList[DGN_ARM_CACHE_NUM].durationUs = 0;
    DgnList[DGN_ARM_CACHE_NUM].maxTimeslices = 0;
    DgnList[DGN_ARM_CACHE_NUM].oneComplete = false;
    DgnList[DGN_ARM_CACHE_NUM].timeoutTimeslice = DGN_INTERVALS_PER_HOUR * 8;

    // Start Fault Injection Point 3
    // Delaying Apex diagnostics startup fault injection point.
    // An individual diagnostic fault injection script file uses this fault injection
    // point to make sure that its diagnostic fault injection testing does not
    // fail before ICE2 checks the fault line for Apex2 failures.
    // End Fault Injection Point 3

    return;
}

/////////////////////////////////////////////////////////////////////////////
// ApexDiagnostic::Initialize
//
/// This function initializes variables and structures for runtime diagnostics.
/////////////////////////////////////////////////////////////////////////////
void ApexDiagnostic::Initialize()
{
    STATIC_ASSERT(DGN_NUM_RUNTIME_TESTS == (sizeof(DgnList) / sizeof(DgnList[0])));

    ApexWatchdog::Init();

    if (DEBUG_LOGGING_ENABLED)
    {
        HI_ApexParam.LogEnable=1;
    }

    ApexDiagnostic::SchedulerInit();

    ApexDiagSafeRam::Init();

    ApexDiagRamData::Init();

    ApexDiagBinaryCrc::Init();

    ApexDiagRamAddress::Init();

    ApexDiagEcc::Init();

    ApexDiagIoShutdown::Init();

    ApexDiagBackplaneLoopback::Init();

    ApexDiagmCrcLoopback::Init();

    ApexDiagArm::ArmPowerUp();

    ApexDiagArmCache::ArmCachePowerUp();

    m_LastDiagTime = HI_ApexReg.SystemTime;
}

/////////////////////////////////////////////////////////////////////////////
// ApexDiagnostic::PowerUp
//
///  This function performs diagnostics that are done once at powerup.
///
/////////////////////////////////////////////////////////////////////////////
void ApexDiagnostic::PowerUp(void)
{
    ApexDiagBinaryCrc::PowerupTest(); // CRC test of the executable RAM area

    ApexDiagEcc::PowerupTest();  // ECC circuit test
}

/////////////////////////////////////////////////////////////////////////////
// ApexDiagnostic::RunScheduled
//
///  This function checks if any of diagnostics should run and executes it if
///  required. To prevent multiple diagnostic calls (i.e. Io shutdown and
///  completion time check) only one "type" of check is done. These are (in
///  highest to lowest priority order):
///  - Io shutdown diagnostic
///  - one of generic diagnostic tests
///  - time completion check
///
/////////////////////////////////////////////////////////////////////////////
void ApexDiagnostic::RunScheduled()
{
    // Start Fault Injection Point 2
    // Code which sets InjectFaultFlag after DGN_FI_DELAY_SEC has passed
    // to trigger an injected fault at run-time will be injected here.
    // End Fault Injection Point 2

    // Check if it's time for Io shutdown test
    if (ApexDiagIoShutdown::IsTimeToRun(HI_ApexReg.SystemTime))
    {
        ApexDiagIoShutdown::RunTest(HI_ApexReg.SystemTime);
    }
    // Then check if it's time for generic diagnostic test (unsigned math handles roll-over)
    else if ((HI_ApexReg.SystemTime - m_LastDiagTime) > DGN_INTERVAL_US)
    {
        // Execute a runtime diagnostic if it's time for one. Scheduling of the various tests is
        // handled within ExecuteSlice(), and is guided by the m_LastDiagTime variable, which
        // increments each time through.
        m_LastDiagTime = HI_ApexReg.SystemTime;
        ExecuteSlice();
    }
    // Finally check if it's time to run diagnostic completion time check.
    else if ((m_SliceNumber - m_LastCompletionCheck) > DGN_COMPL_CHECK_INTERVAL_TIME_SLICE)
    {
        m_LastCompletionCheck = m_SliceNumber;
        CompletionTimeCheck();
    }
    else
    {
        // nothing to do
    }
}
