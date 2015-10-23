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

	static const UINT32  CorruptedDiagTestVector    = 1;
	
	static const UINT32  CorruptedDiagTestMemory    = 2;
	
	static const UINT32  DiagTestTooLongToComplete  = 3;
	
	static const UINT32  AllDiagTestsDidNotComplete = 4;
	
	std::vector <BlackfinDiagTest *> * RunTimeDiagnostics;
	
	std::vector<BlackfinDiagTest *>::iterator itTestEnumeration; 
		
	// Number of timeslices between diagnostics completion time checks
	// Start Fault Injection Point 3
	// Declaration of DGN_COMPL_CHECK_INTERVAL_TIME_SLICE constant with a smaller value to make
	// completion time diagnostic injected fault happen faster will be injected here.
//	static const DiagSlices_t DGN_COMPL_CHECK_INTERVAL_TIME_SLICE = 15 * DGN_INTERVALS_PER_MINUTE;

    BOOL   bSchedulerInstantiationComplete;
    
    DiagTime_t m_LastDiagTime;
    
    DiagTime_t TimeCycleStarted;
	
	BlackfinDiagScheduler(const BlackfinDiagScheduler &);

	BlackfinDiagScheduler &operator=(const BlackfinDiagScheduler &);

	BOOL AreAnyActiveTestsTakingTooLong( BlackfinDiagTest::DiagnosticTestTypes & TestType );
	
	BOOL DidAllTestsComplete( BlackfinDiagTest::DiagnosticTestTypes & TestType );

	BOOL EnumerateNextScheduledTest( BlackfinDiagTest * & pbdtNextDiag );
	
	BOOL IsTimeForNewDiagCycle();
	
	BOOL IsTimeForNewTimeslicePeriod();
	
    void ConfigureErrorCode( UINT32 & ReturnedErrorCode, BlackfinDiagTest::DiagnosticTestTypes TestTypeCurrent );
    
	void DoMoreDiagnosticTesting();
	
	void RunTestsForCurrentTimeslice();
	
	void SaveDiagCycleStartData();

	void SetDiagTestsReadyForNewCycle();
	
	void StartEnumeratingTestsForThisTimeslice();
	
};



