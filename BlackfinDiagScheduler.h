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

	BlackfinDiagScheduler(std::vector <BlackfinDiagTests::BlackfinDiagTest *> * Diagnostics);

	/// Default destructor.
	~BlackfinDiagScheduler() {}

	/// Initializes Apex diagnostics.
	void Initialize(void) {}

	/// Runs power-up diagnostic tests.
	void PowerUp(void) {}

	/// Performs run-time tests according to the schedule.
	void RunScheduled();


private:

	static const UINT32  corruptedDiagTestVector_    = 1;
	
	static const UINT32  corruptedDiagTestMemory_    = 2;
	
	static const UINT32  diagTestTooLongToComplete_  = 3;
	
	static const UINT32  allDiagTestsDidNotComplete_ = 4;
	
	std::vector <BlackfinDiagTests::BlackfinDiagTest *> * runTimeDiagnostics_;
	
	std::vector<BlackfinDiagTests::BlackfinDiagTest *>::iterator itTestEnumeration_; 
		
	// Number of timeslices between diagnostics completion time checks
	// Start Fault Injection Point 3
	// Declaration of DGN_COMPL_CHECK_INTERVAL_TIME_SLICE constant with a smaller value to make
	// completion time diagnostic injected fault happen faster will be injected here.
	//	static const DiagSlices_t DGN_COMPL_CHECK_INTERVAL_TIME_SLICE = 15 * DGN_INTERVALS_PER_MINUTE;

    BOOL   schedulerInstantiationComplete_;
    
    DiagnosticCommon::DiagTime_t m_LastDiagTime_;
    
    DiagnosticCommon::DiagTime_t timeCycleStarted_;
    
    BlackfinDiagScheduler();
	
	BlackfinDiagScheduler(const BlackfinDiagScheduler &);

	BlackfinDiagScheduler &operator=(const BlackfinDiagScheduler &);

	BOOL AreAnyActiveTestsTakingTooLong( BlackfinDiagTests::BlackfinDiagTest::DiagnosticTestTypes & TestType );
	
	BOOL DidAllTestsComplete( BlackfinDiagTests::BlackfinDiagTest::DiagnosticTestTypes & TestType );

	BOOL EnumerateNextScheduledTest( BlackfinDiagTests::BlackfinDiagTest * & pbdtNextDiag );
	
	BOOL IsTimeForNewDiagCycle();
	
	BOOL IsTimeForNewTimeslicePeriod();
	
    void ConfigureErrorCode( UINT32 & ReturnedErrorCode, BlackfinDiagTests::BlackfinDiagTest::DiagnosticTestTypes TestTypeCurrent );
    
	void DoMoreDiagnosticTesting();
	
	void RunTestsForCurrentTimeslice();
	
	void SaveDiagCycleStartData();

	void SetDiagTestsReadyForNewCycle();
	
	void StartEnumeratingTestsForThisTimeslice();
	
};



