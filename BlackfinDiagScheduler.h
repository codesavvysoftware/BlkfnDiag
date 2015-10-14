#pragma once
#include <algorithm>


// FORWARD REFERENCES
// (none)

// Start Fault Injection Point 1
// Declaration of global UINT32 variable InjectFaultFlag used to delay
// the injected fault until run-time will be injected here.
// End Fault Injection Point 1

////////////////////////////////////////////////////////////////////////////////
/// @class DiagnosticExecution
///
/// @ingroup Diagnostics
///
/// @brief This class implements the main work flow for SIL-2 diagnostics.
////////////////////////////////////////////////////////////////////////////////    
#include "BlackfinDiagTest.h"

class BlackfinDiagScheduler
{
public:

	BlackfinDiagScheduler(std::vector <BlackfinDiagTest *> * Diagnostics);

	/// Default destructor.
	~BlackfinDiagScheduler() {}

	/// Initializes Apex diagnostics.
	void Initialize(void) {}

	/// Runs power-up diagnostic tests.
	void PowerUp(void) {}

	/// Performs run-time tests according to the schedule.
	void RunScheduled();


private:

	static const UINT32  CorruptedDiagTestVector = 1;
	
	static const UINT32  CorruptedDiagTestMemory = 2;
	
	static const UINT32  DiagTestTooLongToComplete = 3;
	
    static const UINT32 DiagnosticErrorNumberMask          = 0x7fffffff;
    
    static const UINT32 DiagnosticErrorTestTypeBitPos      = 26;
    
	struct TraceLog
	{
		DiagTime_t   timestamp;
		DiagSlices_t value;
	};
	
	std::vector <BlackfinDiagTest *> * RunTimeDiagnostics;

	static const UINT32 TRACE_LOG_SIZE = 4;
	TraceLog m_TraceLog[4]; //C++ 11 constexpr would handle this situation very nicely TRACE_LOG_SIZE];

							// Timeslice that increments each time that runtime diagnostics run
	static const UINT32 DGN_INTERVALS_PER_MINUTE           = 12000;      // (60 * 1000000) / 50000;

	// Number of timeslices between diagnostics completion time checks
	// Start Fault Injection Point 3
	// Declaration of DGN_COMPL_CHECK_INTERVAL_TIME_SLICE constant with a smaller value to make
	// completion time diagnostic injected fault happen faster will be injected here.
	static const DiagSlices_t DGN_COMPL_CHECK_INTERVAL_TIME_SLICE = 15 * DGN_INTERVALS_PER_MINUTE;

	// End Fault Injection Point 3
	DiagSlices_t m_SliceNumber;
	/// Last time a diagnostic test was run.
	DiagTime_t   m_LastDiagTime;
	
	/// Last time diagnostic completion time
	DiagSlices_t  m_LastCompletionCheck;

	
	UINT32 DgnTrace[32];

	BlackfinDiagScheduler(const BlackfinDiagScheduler &);

	BlackfinDiagScheduler &operator=(const BlackfinDiagScheduler &);

	void CompletionTimeCheck();

    BOOL FindTestIndexForThisSlice(BlackfinDiagTest * & pbdtNextDiag);

	void ExecuteSlice();

	void SchedulerInit(void);

	void LogDgnTrace(const UINT32 val);
	
	void RescheduleLowerPriorityDiagnostics();

    void ConfigureErrorCode( UINT32 & ReturnedErrorCode, BlackfinDiagTest::DiagnosticTestTypes TestTypeCurrent );
    
};



