#include "BlackfinDiagnostic.h"
#include "BlackfinDiagSafeRam.h"                      // for BlackfinDiagSafeRam
#include "BlackfinDiagBinaryCrc.h"
#include "BlackfindiagRamData.h"
#include "BlackfinDiagInstructionRam.h"

static BlackfinDiagInstructionRam bdirInstuctionTest;

// FORWARD REFERENCES
// (none)

// Class variable definition
BlackfinDiagnostic::TraceLog BlackfinDiagnostic::m_TraceLog[BlackfinDiagnostic::TRACE_LOG_SIZE] = {0};
DiagControlBlock BlackfinDiagnostic::DgnList[DGN_NUM_RUNTIME_TESTS] = {0};

UINT32 BlackfinDiagnostic::m_LastDiagTime = 0;
UINT64 BlackfinDiagnostic::m_LastCompletionCheck = 0;
UINT64 BlackfinDiagnostic::m_SliceNumber = 0;

extern "C" bool HI_DisableIrq() {
	return true;
}
extern "C" void HI_EnableIrq() {} 

extern "C" UINT32 BlackfinDiagRamDataRunTestAsm(UINT32 numwords, UINT32 *RAMAddr, UINT32 *shadowRam) {
	return 0;
}

// Start Fault Injection Point 1
// Definition of global UINT32 variable InjectFaultFlag used to delay
// the injected fault until run-time will be injected here.
// End Fault Injection Point 1

/////////////////////////////////////////////////////////////////////////////
//  BlackfinDiagnostic::LogDgnTrace
//
/// Logging function storing passed value with the times it was called at.
/// Diagnostic tracer to help debug watchdog problems on CN2/B.
/////////////////////////////////////////////////////////////////////////////
extern UINT32 DgnTrace[32];

void BlackfinDiagnostic::LogDgnTrace(const UINT32 val)
{
    static UINT32 index = 0;

    UINT32   dsp_time = GetSystemTime();
                
    if (DEBUG_LOGGING_ENABLED)
    {
        if ( index > (UINT32)((sizeof( DgnTrace ) / 4) - 4) )
        {
            index = 0;
        }

        DgnTrace[ index++ ] = dsp_time;//HI_ApexReg.SystemTime;
        DgnTrace[ index++ ] = val;
    }
    else
    {
        if ( index >= TRACE_LOG_SIZE )
        {
            index = 0;
        }
    
        m_TraceLog[ index ].timestamp = dsp_time;//HI_ApexReg.SystemTime;
        m_TraceLog[ index ].value = val;
        index++;
    }
}

/////////////////////////////////////////////////////////////////////////////
//  BlackfinDiagnostic::FindTestIndexForThisSlice
//
/// Find either a test scheduled for this slice or the last test in the list.
/////////////////////////////////////////////////////////////////////////////
UINT8 BlackfinDiagnostic::FindTestIndexForThisSlice()
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
//  BlackfinDiagnostic::RescheduleLowerPriorityDiagnostics
//
/// Reschedule any lower priority diagnostics that were also scheduled for this
/// slice by bumping their trigger values to the next slice number.
/////////////////////////////////////////////////////////////////////////////
void BlackfinDiagnostic::RescheduleLowerPriorityDiagnostics(UINT8 dgnIndex, DiagControlBlock *dgnCurDiagCB)
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
//  BlackfinDiagnostic::Scheduler
//
/// This function handles the scheduling of the various runtime diagnostic
/// functions.
/////////////////////////////////////////////////////////////////////////////
void BlackfinDiagnostic::ExecuteSlice()
{
    LogDgnTrace( 0xbbbbbbbb );

    // Do hardware watchdog handing
    //ApexWatchdog::Service();

    UINT8 testIndex = BlackfinDiagnostic::FindTestIndexForThisSlice();

    LogDgnTrace( testIndex );

    // Assign pointer to found Diag CB
    DiagControlBlock *pCurrentDiagCB = &DgnList[testIndex];    // Pointer to the currently referenced Diag CB

    // Schedule next trigger slice for this diag. UINT math handles
    // rollover. In some cases, this trigger value will be overwritten
    // from within the test function that is called.  That's why we do the
    // reschedule before we enter the function.
    pCurrentDiagCB->triggerValueTimeslice += pCurrentDiagCB->stepValueTimeslice;

    // Record the entry time before calling diag
    UINT32 entryTimeUs = GetSystemTime();//HI_ApexReg.SystemTime;

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
            pCurrentDiagCB->durationUs = GetSystemTime() - entryTimeUs;//HI_ApexReg.SystemTime - entryTimeUs;
            break;
        }

        default:
        {
            // Call HW_HandleException for any other responses
            //firmExcept();
            break;
        }
    };

    RescheduleLowerPriorityDiagnostics(testIndex, pCurrentDiagCB);

    LogDgnTrace( 0xeeeeeeee );

    // Update diag slice number.
    m_SliceNumber++;
}


/////////////////////////////////////////////////////////////////////////////
//  BlackfinDiagnostic::CompletionTimeCheck
//
/// This function checks if all diagnostic tests have been run in time.
///
/////////////////////////////////////////////////////////////////////////////
void BlackfinDiagnostic::CompletionTimeCheck()
{
    for (UINT8 dgnIndex = 0; dgnIndex < DGN_NUM_RUNTIME_TESTS; ++dgnIndex)
    {
        if ((m_SliceNumber - DgnList[dgnIndex].lastCompleteTimeslice) > DgnList[dgnIndex].timeoutTimeslice)
        {
            //firmExcept();
        }
    }

//    if (!ApexDiagIoShutdown::CheckCompletionTime())
    {
        //firmExcept();
    }
    return;
}

/////////////////////////////////////////////////////////////////////////////
// BlackfinDiagnostic::SchedulerInit
//
/// This function initializes runtime diagnostic scheduler variables and
/// structures.
/////////////////////////////////////////////////////////////////////////////
void BlackfinDiagnostic::SchedulerInit()
{
    m_SliceNumber         = 0;
    m_LastCompletionCheck = 0;


    DgnList[DGN_SAFE_RAM_NUM].diagFunc              = BlackfinDiagSafeRam::Check;
    DgnList[DGN_SAFE_RAM_NUM].triggerValueTimeslice = 0;
    // Start Fault Injection Point 4
    // Code which sets DgnList[DGN_SAFE_RAM_NUM].stepValueTimeslice to a smaller value
    // to make the test execute earlier will be injected here.
    DgnList[DGN_SAFE_RAM_NUM].stepValueTimeslice    = MsToDgnSlices(DGN_SAFE_RAM_STEP_MS);
    // End Fault Injection Point 4
    DgnList[DGN_SAFE_RAM_NUM].lastCompleteTimeslice = 0;
    DgnList[DGN_SAFE_RAM_NUM].durationUs            = 0;
    DgnList[DGN_SAFE_RAM_NUM].maxTimeslices         = 0;
    DgnList[DGN_SAFE_RAM_NUM].oneComplete           = false;
    DgnList[DGN_SAFE_RAM_NUM].timeoutTimeslice      = DGN_INTERVALS_PER_HOUR * 8;

//    DgnList[DGN_TIMER].diagFunc = ApexDiagTimer::RunTest;
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
    DgnList[DGN_TIMER].lastCompleteTimeslice        = 0;
    DgnList[DGN_TIMER].durationUs                   = 0;
    DgnList[DGN_TIMER].maxTimeslices                = 0;
    DgnList[DGN_TIMER].oneComplete                  = false;
    DgnList[DGN_TIMER].timeoutTimeslice             = DGN_INTERVALS_PER_HOUR * 8;

    DgnList[DGN_EXE_CRC_NUM].diagFunc = BlackfinDiagBinaryCrc::RunTest;
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


    DgnList[DGN_RAM_DATA_NUM].diagFunc = BlackfinDiagRamData::RunTest;
    DgnList[DGN_RAM_DATA_NUM].triggerValueTimeslice = 0;
    DgnList[DGN_RAM_DATA_NUM].stepValueTimeslice = MsToDgnSlices(DGN_RAM_DATA_STEP_MS);
    DgnList[DGN_RAM_DATA_NUM].lastCompleteTimeslice = 0;
    DgnList[DGN_RAM_DATA_NUM].durationUs = 0;
    DgnList[DGN_RAM_DATA_NUM].maxTimeslices = 0;
    DgnList[DGN_RAM_DATA_NUM].oneComplete = false;
    DgnList[DGN_RAM_DATA_NUM].timeoutTimeslice = DGN_INTERVALS_PER_HOUR * 8;


    // Start Fault Injection Point 3
    // Delaying Apex diagnostics startup fault injection point.
    // An individual diagnostic fault injection script file uses this fault injection
    // point to make sure that its diagnostic fault injection testing does not
    // fail before ICE2 checks the fault line for Apex2 failures.
    // End Fault Injection Point 3

    return;
}

/////////////////////////////////////////////////////////////////////////////
// BlackfinDiagnostic::Initialize
//
/// This function initializes variables and structures for runtime diagnostics.
/////////////////////////////////////////////////////////////////////////////
void BlackfinDiagnostic::Initialize()
{
    STATIC_ASSERT(DGN_NUM_RUNTIME_TESTS == (sizeof(DgnList) / sizeof(DgnList[0])));

    //ApexWatchdog::Init();

    if (DEBUG_LOGGING_ENABLED)
    {
//        HI_ApexParam.LogEnable=1;
    }

    BlackfinDiagnostic::SchedulerInit();
    BlackfinDiagSafeRam::Init();

    BlackfinDiagRamData::Init();

    BlackfinDiagBinaryCrc::Init();

    m_LastDiagTime = GetSystemTime();//HI_ApexReg.SystemTime;
}

/////////////////////////////////////////////////////////////////////////////
// BlackfinDiagnostic::PowerUp
//
///  This function performs diagnostics that are done once at powerup.
///
/////////////////////////////////////////////////////////////////////////////
void BlackfinDiagnostic::PowerUp(void)
{
    BlackfinDiagBinaryCrc::PowerupTest(); // CRC test of the executable RAM area
}

/////////////////////////////////////////////////////////////////////////////
// BlackfinDiagnostic::RunScheduled
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
void BlackfinDiagnostic::RunScheduled()
{
    bdirInstuctionTest.RunTest( &DgnList[DGN_SAFE_RAM_NUM] );
    
    return;
   // Start Fault Injection Point 2
    // Code which sets InjectFaultFlag after DGN_FI_DELAY_SEC has passed
    // to trigger an injected fault at run-time will be injected here.
    // End Fault Injection Point 2

    // Check if it's time for Io shutdown test
    if (false)
//    if (ApexDiagIoShutdown::IsTimeToRun(GetSystemTime()))//HI_ApexReg.SystemTime))
    {
//        ApexDiagIoShutdown::RunTest(GetSystemTime())//HI_ApexReg.SystemTime);
    }
    // Then check if it's time for generic diagnostic test (unsigned math handles roll-over)
    else if ((GetSystemTime()       - m_LastDiagTime) > DGN_INTERVAL_US)
          //((HI_ApexReg.SystemTime - m_LastDiagTime) > DGN_INTERVAL_US)
    {
        // Execute a runtime diagnostic if it's time for one. Scheduling of the various tests is
        // handled within ExecuteSlice(), and is guided by the m_LastDiagTime variable, which
        // increments each time through.
        m_LastDiagTime = GetSystemTime();//HI_ApexReg.SystemTime;
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
