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

rivate:

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
