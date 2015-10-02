////////////////////////////////////////////////////////////////////////////////
/// @file BlackfinDiagnostic.hpp
///
/// @details BlackfinDiagnostic class definition.
///
/// @if REVISION_HISTORY_INCLUDED
/// @par Edit History
/// -   mgrad     27-NOV-2013   Created.
/// -   pszramo   16-DEC-2013   Updated fault injection points according to
///                             Coverity defects triage.
/// -   dtstalte  04-FEB-2014   Increase CRC diagnostic step value to give
///                             wider margain of error to complete in time.
/// @endif
///
/// @par Copyright (c) 2013 Rockwell Automation Technologies, Inc. All rights reserved.
////////////////////////////////////////////////////////////////////////////////
#ifndef BLACKFIN_DIAGNOSTIC_HPP
#define BLACKFIN_DIAGNOSTIC_HPP
#include <time.h>

// SYSTEM INCLUDES
// <none>

// C PROJECT INCLUDES
// <none>

// C++ PROJECT INCLUDES
#include "DEFS.h"              // for UINT types
#include "BlackfinParameters.h"
#include "BlackfinDiag.h"

using namespace BlackfinDiagnosticGlobals;

#define ASSERT(_x) do { if ( !(_x) ) { firmExcept(); } } while(0)
inline bool DebugSession(void)
{
    // Will be 0xdeb1deb0 if the debugger is controlling the
    //  execution of this session
    //extern UINT32 const db_debuggerIsRunning;

    //return db_debuggerIsRunning == 0xdeb1deb0;
    return false;
}

    

#define DGN_INT_DATA_RAM_START  0x30000      	// Internal RAM begin addr
#define DGN_INT_DATA_RAM_END    0x38000      	// Internal RAM end+1
// Thanks to Jon Jagger at http://www.jaggersoft.com/pubs/CVu11_3.html
//  for this slick compile time ASSERT() macro
// Causes a compile-time error is A is not true
#include "Apex.h"


// FORWARD REFERENCES
// (none)

// Start Fault Injection Point 1
// Declaration of global UINT32 variable InjectFaultFlag used to delay
// the injected fault until run-time will be injected here.
// End Fault Injection Point 1

////////////////////////////////////////////////////////////////////////////////
/// @class BlackfinDiagnostic
///
/// @ingroup Diagnostics
///
/// @brief This class implements the main work flow for SIL-2 diagnostics.
////////////////////////////////////////////////////////////////////////////////    
class BlackfinDiagnostic
{
public:

    /// Initializes Apex diagnostics.
    static void Initialize( void );

    /// Runs power-up diagnostic tests.
    static void PowerUp(void);

    /// Performs run-time tests according to the schedule.
    static void RunScheduled();

    /// Check if test are completing on time.
    static void CompletionTimeCheck( void );

/*   enum TestState
    {
        TEST_LOOP_COMPLETE = 0,
        TEST_IN_PROGRESS = 1
    };
*/
/*    // PRIORITIZED LIST OF RUNTIME DIAGNOSTICS.  The top of the list has
    // a higher priority than the bottom.
    enum RuntimeTestList
    {
        DGN_SAFE_RAM_NUM,            // Safe RAM data test
        DGN_TIMER,                   // Timer diagnostic
        DGN_EXE_CRC_NUM,             // Executable RAM checksum
        DGN_RAM_DATA_NUM,            // RAM data test
        DGN_NUM_RUNTIME_TESTS        // Total entries in runtime test list
    };
*/
/*    struct DiagControlBlock
    {
        // Pointer to the relevant diagnostic function
        TestState (*diagFunc) (DiagControlBlock *diagControlBlock);
        UINT64 triggerValueTimeslice;       // Timeslice number for next trigger
        UINT32 stepValueTimeslice;          // Number of diag timeslices between activations
        UINT64 lastCompleteTimeslice;       // Timeslice number of the previous test completion
        UINT32 durationUs;                  // Time spent in diag function
        UINT32 maxTimeslices;               // Max number of timeslices ever seen between triggers
        bool   oneComplete;                 // True if diag has completed at least once. This is used to decide
                                            //      whether or not maxTimeslices is valid, since  maxTimeslices
                                            //      should indicate a completion-to-completion time
        UINT32 timeoutTimeslice;            // Max number of timeslices that test
                                            // can wait to be completed in
    };
*/
    static DiagControlBlock DgnList[DGN_NUM_RUNTIME_TESTS];

    // Timeslice that increments each time that runtime diagnostics run
    static UINT64 m_SliceNumber;

    static const UINT32 MICROSECONDS_IN_MILLISECOND = 1000; // Number of micro seconds in milli second

    // Runtime diag interval (uS)
    // Note: Since watchdog servicing is done at this rate, care
    // must be taken to ensure that this value is not too large.
    //static const UINT32 DGN_INTERVAL_US = 50 * MICROSECONDS_IN_MILLISECOND;

    static const UINT32 MILLISECONDS_IN_SECOND = 1000; // Number of milli seconds in second
    static const UINT32 SECONDS_IN_MINUTE = 60; // Number of seconds in minute
    static const UINT32 MINUTES_IN_HOUR = 60; // Number of minutes in hour

    // Number of diagnostics intervals in one second
    static const UINT32 DGN_INTERVALS_PER_SECOND = (MICROSECONDS_IN_MILLISECOND * MILLISECONDS_IN_SECOND) / DGN_INTERVAL_US;

    // Number of diagnostics intervals in one minute
    static const UINT32 DGN_INTERVALS_PER_MINUTE = DGN_INTERVALS_PER_SECOND * SECONDS_IN_MINUTE;

    // Number of diagnostics intervals in one hour
    static const UINT32 DGN_INTERVALS_PER_HOUR = DGN_INTERVALS_PER_MINUTE * MINUTES_IN_HOUR;

    static const UINT32 MILLISECONDS_IN_MINUTE = SECONDS_IN_MINUTE * MILLISECONDS_IN_SECOND;

    static const UINT32 SECONDS_IN_HOUR = MINUTES_IN_HOUR * SECONDS_IN_MINUTE;
    static const UINT32 MILLISECONDS_IN_HOUR = MINUTES_IN_HOUR * SECONDS_IN_MINUTE * MILLISECONDS_IN_SECOND;
    static const UINT32 milleseconds_per_slice = DGN_INTERVAL_US / 1000;
    
    

    // Number of timeslices between diagnostics completion time checks
    // Start Fault Injection Point 3
    // Declaration of DGN_COMPL_CHECK_INTERVAL_TIME_SLICE constant with a smaller value to make
    // completion time diagnostic injected fault happen faster will be injected here.
    static const UINT32 DGN_COMPL_CHECK_INTERVAL_TIME_SLICE = (DGN_INTERVALS_PER_MINUTE * 15);
    // End Fault Injection Point 3
private:

    friend class ApexWatchdog;      // for access to LogDgnTrace
    friend class ApexDiagTimer;     // for access to DGN_TIMER_STEP_MS
    
    /// Default constructor.
    BlackfinDiagnostic();

    /// Default destructor.
    ~BlackfinDiagnostic();

    /// Copy constructor and assignment operator not implemented.
    BlackfinDiagnostic(const BlackfinDiagnostic &);
    BlackfinDiagnostic &operator=(const BlackfinDiagnostic &);

    static void ExecuteSlice(void);

    static void SchedulerInit( void );

    static void LogDgnTrace( const UINT32 val );
    static UINT8 FindTestIndexForThisSlice( void );
    static void RescheduleLowerPriorityDiagnostics( UINT8 dgnIndex, DiagControlBlock *dgnCurDiagCB );

    struct TraceLog
    {
        UINT32 timestamp;
        UINT32 value;
    };
    static const UINT32 TRACE_LOG_SIZE = 4;
    static TraceLog m_TraceLog[TRACE_LOG_SIZE];

    /// Last time a diagnostic test was run.
    static UINT32  m_LastDiagTime;
    /// Last time diagnostic completion time
    static UINT64  m_LastCompletionCheck;

    // TIME PERIODS BETWEEN SCHEDULED TESTS (in milliseconds)
    // Timer Test
    // Start Fault Injection Point 4
    // Declaration of DGN_TIMER_INITIAL_TRIG_VAL_MS constant with a smaller value to make
    // multiple timer diagnostic injected faults happen faster will be injected here.
    static const UINT32 DGN_TIMER_INITIAL_TRIG_VAL_MS   = 300000; // 5 mins
    // End Fault Injection Point 4

    // Start Fault Injection Point 5
    // Declaration of DGN_TIMER_STEP_MS constant with a smaller value to make
    // multiple timer diagnostic injected faults happen faster will be injected here.
    static const UINT32 DGN_TIMER_STEP_MS               = 1800000;  // 30 mins
    // End Fault Injection Point 5
    
    static const UINT32 DGN_RAM_ADDR_STEP_MS            = 60000;    // 1 min
    static const UINT32 DGN_SAFE_RAM_STEP_MS            = 300000;   // 5 mins
    
    #if defined(SUPPORT_FAULT_INJECTION)
    // Use a much shorter duration for demonstration of these diagnostic tests
    // from the host side
    static const UINT32 DGN_BP_LOOPBACK_STEP_MS         = 5000;     // 5 secs
    static const UINT32 DGN_MCRC_LOOPBACK_STEP_MS       = 120000;   // 2 mins = 120000 msecs
    #else
    static const UINT32 DGN_BP_LOOPBACK_STEP_MS         = 10800000; // 3 hrs = 10800000 msecs
    static const UINT32 DGN_MCRC_LOOPBACK_STEP_MS       = 7200000;  // 2 hrs = 7200000 msecs
    #endif

    static const UINT32 DGN_EXE_CRC_STEP_MS             = 500;      // 500 ms
    static const UINT32 DGN_RAM_DATA_STEP_MS            = 500;      // 500 ms
    static const UINT32 DGN_ARM_STEP_MS                 = 5400000;  // 90 mins
    static const UINT32 DGN_ARM_CACHE_STEP_MS           = 5400000;  // 90 mins
    
    // ECC step is the only one defined in timeslices instead of ms, this is why it is
    // used to fill "empty" timeslices no other test is designated to run
    static const UINT32 DGN_ECC_STEP_TIMESLICE          = 1;        // ASAP
    
    static const UINT32 DGN_FI_SEVENTY_SEC_MS           = 70000;
    static const UINT32 DGN_FI_ONE_MIN_MS               = 60000;
    static const UINT32 DGN_FI_FIVE_SEC_MS              = 5000;
    
    // Start Fault Injection Point 2
    // Declaration of DGN_FI_DELAY_SEC constant which determines
    // when InjectFaultFlag is set will be injected here.
    // End Fault Injection Point 2



/////////////////////////////////////////////////////////////////////////////
// BlackfinDiagnostic::MsToDgnSlices
//
/// Function that converts the desired number of milliseconds between
/// scheduled diagnostics to the proper integer number of diagnostic
/// timeslices.  If zero, then set to one as a minimum.
/////////////////////////////////////////////////////////////////////////////
static inline UINT32 MsToDgnSlices(UINT32 value)
{
    UINT32 slices = value / milleseconds_per_slice;
    
    return ( slices > 1 ? slices : 1 );
    
}
};

#endif // #if !defined(BlackfinDiagnostic_HPP)
