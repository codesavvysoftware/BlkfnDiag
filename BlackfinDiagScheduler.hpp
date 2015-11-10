#pragma once
//#include <algorithm>


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
#include "BlackfinDiagTest.hpp"
#include <vector>

namespace DiagnosticScheduling
{
    class BlackfinDiagScheduler
    {
        public:

        	BlackfinDiagScheduler( std::vector <BlackfinDiagTesting::BlackfinDiagTest *> * pDiagnostics,
                                   UINT32                                                  corruptedVectorErr,
                                   UINT32                                                  corruptedTestMemoryErr,
                                   UINT32                                                  allDiagnosticsNotCompletedErr,
                                   UINT32                                                  schedulerTestType );

        	/// Default destructor.
        	~BlackfinDiagScheduler() 
        	{
        	}

        	/// Initializes diagnostics.
        	void Initialize(void) 
        	{
        	}

        	/// Runs power-up diagnostic tests.
        	void PowerUp(void) 
        	{
        	}
	
        	/// Performs run-time tests according to the schedule.
        	void RunScheduled();


        private:

            static const DiagnosticCommon::DiagTimestampTime  DEFAULT_INITIAL_TIMESTAMP = 0;	

            typedef enum 
        	{
        		INITIAL_INSTANTIATION,
        		MAX_PERIOD_EXPIRED_ALL_TESTS_COMPLETE,
        		MAX_PERIOD_EXPIRED_INCOMPLETE_TESTING,
        		NO_NEW_SCHEDULING_PERIOD,
        		NO_TESTS_TO_RUN_ALL_COMPLETED,
        		NO_TEST_ITERATIONS_SCHEDULED,
        		TEST_ITERATIONS_SCHEDULED,
        	} 
        	SchedulerStates;
	
        	SchedulerStates        m_CurrentSchedulerState;
	
        	std::vector <BlackfinDiagTesting::BlackfinDiagTest *> * m_pRunTimeDiagnostics;
	
        	std::vector<BlackfinDiagTesting::BlackfinDiagTest *>::iterator m_TestEnumeration; 
		
		 	UINT32                             m_CorruptedVectorErr;

		 	UINT32                             m_CorruptedTestMemoryErr;
		 	
		 	UINT32                             m_AllDiagnosticsNotCompletedErr;
		 	
		 	UINT32                             m_SchedulerTestType;
		 	
        	// Number of timeslices between diagnostics completion time checks
        	// Start Fault Injection Point 3
        	// Declaration of DGN_COMPL_CHECK_INTERVAL_TIME_SLICE constant with a smaller value to make
        	// completion time diagnostic injected fault happen faster will be injected here.
        	//	static const DiagSlices_t DGN_COMPL_CHECK_INTERVAL_TIME_SLICE = 15 * DGN_INTERVALS_PER_MINUTE;

        	DiagnosticCommon::DiagTimestampTime m_TimestampCurrent;	
	
            DiagnosticCommon::DiagTimestampTime m_TimeTestCycleStarted;
    
            DiagnosticCommon::DiagTimestampTime m_TimeLastIterationPeriodExpired;

            BlackfinDiagScheduler();
	
        	BlackfinDiagScheduler(const BlackfinDiagScheduler &);

        	BlackfinDiagScheduler &operator=(const BlackfinDiagScheduler &);

        	BOOL AreTestIterationsScheduledToRun();

        	void DetermineCurrentSchedulerState();

        	void ConfigureErrorCode( UINT32 & returnedErrorCode, UINT32 testTypeCurrent );

        	void DoMoreDiagnosticTesting();

        	BOOL DidAllTestsComplete();

        	BOOL EnumerateNextScheduledTest( BlackfinDiagTesting::BlackfinDiagTest * & pbdtNextDiag );

        	BOOL HasCompleteDiagTestPeriodExpired( DiagnosticCommon::DiagElapsedTime elapsed_time );

        	BOOL HasNewTestIterationPeriodStarted( DiagnosticCommon::DiagElapsedTime elapsedTimeForCurrentIteration );

        	BOOL IsTestScheduledToRun(BlackfinDiagTesting::BlackfinDiagTest * & rpPbdt);

            BOOL IsTestingCompleteForDiagCycle(BlackfinDiagTesting::BlackfinDiagTest * & rpPbdt);
    
        	void ResetTestsCompletedForCycle(BlackfinDiagTesting::BlackfinDiagTest * & rpPbdt);

        	void SetAnotherTestCompletedForCycle(BlackfinDiagTesting::BlackfinDiagTest * & rpPbdt);

        	void SetDiagTestsReadyForNewTestCycle();

        	BOOL StartEnumeratingTestsForThisIterationPeriod();
	
    };
};



