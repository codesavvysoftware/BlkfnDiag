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
#include <vector>

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
	
	typedef enum {
		INITIAL_INSTANTIATION,
		MAX_PERIOD_EXPIRED_ALL_TESTS_COMPLETE,
		MAX_PERIOD_EXPIRED_INCOMPLETE_TESTING,
		NO_NEW_SCHEDULING_PERIOD,
		NO_TESTS_TO_RUN_ALL_COMPLETED,
		NO_TEST_ITERATIONS_SCHEDULED,
		TEST_ITERATIONS_SCHEDULED,
	} SchedulerStates;
	
	SchedulerStates        currentSchedulerState_;
		
	
	std::vector <BlackfinDiagTests::BlackfinDiagTest *> * runTimeDiagnostics_;
	
	std::vector<BlackfinDiagTests::BlackfinDiagTest *>::iterator itTestEnumeration_; 
		
	// Number of timeslices between diagnostics completion time checks
	// Start Fault Injection Point 3
	// Declaration of DGN_COMPL_CHECK_INTERVAL_TIME_SLICE constant with a smaller value to make
	// completion time diagnostic injected fault happen faster will be injected here.
	//	static const DiagSlices_t DGN_COMPL_CHECK_INTERVAL_TIME_SLICE = 15 * DGN_INTERVALS_PER_MINUTE;

	DiagnosticCommon::DiagTimestampTime_t timestampCurrent_;	
	
    DiagnosticCommon::DiagTimestampTime_t timeTestCycleStarted_;
    
    DiagnosticCommon::DiagTimestampTime_t timeLastIterationPeriodExpired_;

    BlackfinDiagScheduler();
	
	BlackfinDiagScheduler(const BlackfinDiagScheduler &);

	BlackfinDiagScheduler &operator=(const BlackfinDiagScheduler &);

	BOOL AreTestIterationsScheduledToRun();

	void DetermineCurrentSchedulerState();

    void ComputeElapsedTime( DiagnosticCommon::DiagTimestampTime_t   current, 
                             DiagnosticCommon::DiagTimestampTime_t   previous, 
                             DiagnosticCommon::DiagElapsedTime_t   & elapsed  );
                             
	void ConfigureErrorCode( UINT32 & returnedErrorCode, BlackfinDiagTests::BlackfinDiagTest::DiagnosticTestTypes testTypeCurrent );

	void DoMoreDiagnosticTesting();

	BOOL DidAllTestsComplete();

	BOOL EnumerateNextScheduledTest( BlackfinDiagTests::BlackfinDiagTest * & pbdtNextDiag );

	DiagnosticCommon::DiagTimestampTime_t GetSystemTimestamp();

	BOOL HasCompleteDiagTestPeriodExpired( DiagnosticCommon::DiagElapsedTime_t elapsed_time );

	BOOL HasNewTestIterationPeriodStarted( DiagnosticCommon::DiagElapsedTime_t elapsedTimeForCurrentIteration );

	BOOL IsTestScheduledToRun(BlackfinDiagTests::BlackfinDiagTest * & pbdt);

    BOOL IsTestingCompleteForDiagCycle(BlackfinDiagTests::BlackfinDiagTest * & pbdt);
    
	void ResetTestsCompletedForCycle(BlackfinDiagTests::BlackfinDiagTest * & pbdt);

	void SetAnotherTestCompletedForCycle(BlackfinDiagTests::BlackfinDiagTest * & pbdt);

	void SetDiagTestsReadyForNewTestCycle();

	BOOL StartEnumeratingTestsForThisIterationPeriod();
	
};



