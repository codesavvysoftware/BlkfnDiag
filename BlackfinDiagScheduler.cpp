#include "BlackfinDiagScheduler.h"
#include "OS_iotk.h"
#include <time.h>

using namespace BlackfinDiagTesting;
using namespace DiagnosticCommon;


	
//DiagnosticCommon::cTiming          SchedulerTiming();
	

BlackfinDiagScheduler::BlackfinDiagScheduler(std::vector <BlackfinDiagTest *> * diagnostics) 
		  : currentSchedulerState_        ( INITIAL_INSTANTIATION ),
		 	runTimeDiagnostics_              ( diagnostics ),
			itTestEnumeration_               ( diagnostics->end() ),
			timestampCurrent_                ( 0 ),
			timeTestCycleStarted_            ( 0 ),
			timeLastIterationPeriodExpired_  ( 0 ) {

	// Number of timeslices between diagnostics completion time checks
	// Start Fault Injection Point 3
	// Declaration of DGN_COMPL_CHECK_INTERVAL_TIME_SLICE constant with a smaller value to make
	// completion time diagnostic injected fault happen faster will be injected here.
	//	static const DiagSlices_t DGN_COMPL_CHECK_INTERVAL_TIME_SLICE = 15 * DGN_INTERVALS_PER_MINUTE;

	if ( !diagnostics ) {
		
		UINT32 errorCode = corruptedDiagTestVector_;
		
		ConfigureErrorCode( errorCode, BlackfinDiagTest::DiagSchedulerTestType );
		
		OS_Assert( errorCode );	
	}
	

	

}

BOOL BlackfinDiagScheduler::AreTestIterationsScheduledToRun() {
	
	return StartEnumeratingTestsForThisIterationPeriod();
}	
    		

	

void BlackfinDiagScheduler::ConfigureErrorCode( UINT32 & returnedErrorCode, BlackfinDiagTest::DiagnosticTestTypes testTypeCurrent ) {
		
	UINT32 ui32        = testTypeCurrent;
	returnedErrorCode &= DiagnosticErrorNumberMask;
	returnedErrorCode |= (ui32 << DiagnosticErrorTestTypeBitPos); 
}

void BlackfinDiagScheduler::DetermineCurrentSchedulerState() {
	
	DiagElapsedTime_t elapsedTimeInTestCycle;
	
	timestampCurrent_ = SystemTiming.GetSystemTimestamp();	
	
	// No state determination needed will get changed by caller
	if ( currentSchedulerState_ == INITIAL_INSTANTIATION ) {
		
		//
		// Sync everything to the same timestamp upon initial instantiation.
		//
		timeTestCycleStarted_ = timestampCurrent_;
		
		timeLastIterationPeriodExpired_ = timestampCurrent_;
		
		for (std::vector<BlackfinDiagTest *>::iterator it = runTimeDiagnostics_->begin(); it != runTimeDiagnostics_->end(); ++it) {
			
			BlackfinDiagTest * pbdt = (*it);
			
			pbdt->SetIterationCompletedTimestamp( timestampCurrent_ );		
        }		
		
        return;
	}

	// Compute Elapsed Time in Current Diagnostic Test Period
	SystemTiming.ComputeElapsedTimeMS( timestampCurrent_, timeTestCycleStarted_, elapsedTimeInTestCycle );
	
	BOOL newDiagTestPeriod = HasCompleteDiagTestPeriodExpired( elapsedTimeInTestCycle );
	
	if ( newDiagTestPeriod ) {
		
		timeTestCycleStarted_ = timestampCurrent_;
		
		BOOL testsCompleted = DidAllTestsComplete();
		
		if ( testsCompleted ) {
			currentSchedulerState_ = MAX_PERIOD_EXPIRED_ALL_TESTS_COMPLETE;
		}
		else {
			currentSchedulerState_ = MAX_PERIOD_EXPIRED_INCOMPLETE_TESTING;
		}
	}
	else {
		
		DiagElapsedTime_t elapsedTimeForCurrentIteration;
	
		SystemTiming.ComputeElapsedTimeMS( timestampCurrent_, timeLastIterationPeriodExpired_, elapsedTimeForCurrentIteration );
		
	    BOOL newTestIterationPeriod = HasNewTestIterationPeriodStarted( elapsedTimeForCurrentIteration );
	    
	    if ( newTestIterationPeriod ) {
	    	
	    	timeLastIterationPeriodExpired_ = timestampCurrent_;
	    	
	    	BOOL testsCompleted = DidAllTestsComplete();
	    	
	    	if ( testsCompleted ) {
	    		
	    		currentSchedulerState_ = NO_TESTS_TO_RUN_ALL_COMPLETED;
	    	}
	    	else {
	    		
	    		BOOL newTestIterationsScheduledToRun = AreTestIterationsScheduledToRun();
	    		
	    		if ( newTestIterationsScheduledToRun ) {
	    			
	    			currentSchedulerState_ = TEST_ITERATIONS_SCHEDULED;
	    		}
	    		else {
	    			currentSchedulerState_ = NO_TEST_ITERATIONS_SCHEDULED;
	    		}
	    	}
	    }
	    else {
	    	currentSchedulerState_ = NO_NEW_SCHEDULING_PERIOD;
	    }
	}
}


			
void BlackfinDiagScheduler::DoMoreDiagnosticTesting() {
	
	BOOL testIterationCanRun = TRUE;

    while( testIterationCanRun ) {
    	
    	BlackfinDiagTest * pCurrentDiagTest = NULL;
			
    	testIterationCanRun = EnumerateNextScheduledTest( pCurrentDiagTest );
    	
    	if ( testIterationCanRun ) {
    		
			UINT32	returnedErrorCode;

			BlackfinDiagTest::TestState testResult = pCurrentDiagTest->RunTest( returnedErrorCode );

    		pCurrentDiagTest->SetCurrentTestState( testResult );
    
    		pCurrentDiagTest->SetIterationCompletedTimestamp( timestampCurrent_ );
		
			switch (testResult)
			{
				case BlackfinDiagTest::TEST_LOOP_COMPLETE: 
				{
					SetAnotherTestCompletedForCycle(pCurrentDiagTest);
				}
			
					break;
			
				case BlackfinDiagTest::TEST_IN_PROGRESS:
	
					break;

				default:
				{
					ConfigureErrorCode( returnedErrorCode, pCurrentDiagTest->GetTestType() );
		
					OS_Assert( returnedErrorCode );
				}
			
					break;
			}
		}
	}
}
    
			
BOOL BlackfinDiagScheduler::DidAllTestsComplete() {

	BOOL allTestsCompleted = TRUE;
	
	for (std::vector<BlackfinDiagTest *>::iterator it = runTimeDiagnostics_->begin(); it != runTimeDiagnostics_->end(); ++it) {
		
		BlackfinDiagTest * pdt = (*it);
			
        BOOL testDidNotComplete = !IsTestingCompleteForDiagCycle(pdt);
        
        if ( testDidNotComplete ) {
        	
        	allTestsCompleted = FALSE;
		
			break;
        }
	}
	
	return allTestsCompleted;
}
		
BOOL BlackfinDiagScheduler::EnumerateNextScheduledTest( BlackfinDiagTest * & pbdtNextDiag ) {
	
	BOOL success = FALSE;

    while( itTestEnumeration_ != runTimeDiagnostics_->end() ) {

    	BlackfinDiagTest * pdt = (*itTestEnumeration_);
				
		++itTestEnumeration_;
				
		BOOL testingNotCompleteForThisDiagCycle = !IsTestingCompleteForDiagCycle(pdt);
		
		if ( testingNotCompleteForThisDiagCycle ) {
			
			BOOL testIsScheduledToRun = IsTestScheduledToRun(pdt);
			
			if ( testIsScheduledToRun ) {
				
				pbdtNextDiag = pdt;
				
				success = TRUE;
				
				break;
			}
		}
    }						
    		
	return success;
}

BOOL BlackfinDiagScheduler::HasCompleteDiagTestPeriodExpired( DiagElapsedTime_t elapsed_time ) {

	BOOL timeForNewCycle = FALSE;
	
	if ( elapsed_time >= PeriodForAllDiagnosticsCompleted_milleseconds ) {
		
		timeForNewCycle = TRUE;

	}

	return timeForNewCycle;

}

BOOL BlackfinDiagScheduler::HasNewTestIterationPeriodStarted( DiagElapsedTime_t elapsedTimeForCurrentIteration ) {
	
	BOOL timeForNewTimeslicePeriod = FALSE;
	
	if ( elapsedTimeForCurrentIteration > PeriodForOneDiagnosticTestIteration_milleseconds ) {
	
		timeForNewTimeslicePeriod = TRUE;
	}
	
	return timeForNewTimeslicePeriod;
}

BOOL BlackfinDiagScheduler::IsTestingCompleteForDiagCycle(BlackfinDiagTest * & pbdt) {

	UINT32 numberToRun = pbdt->GetNumberOfTimesToRunPerDiagCycle();
	
	UINT32 numberRan   = pbdt->GetNumberOfTimesRanThisDiagCycle();
	
	return ( numberRan >= numberToRun );
	
}
BOOL BlackfinDiagScheduler::IsTestScheduledToRun(BlackfinDiagTest * & pbdt) {

	BOOL timeToRun = FALSE;

	DiagTimestampTime_t startOfIteration = pbdt->GetIterationCompletedTimestamp();

	DiagElapsedTime_t elapsedTime;
	
	SystemTiming.ComputeElapsedTimeMS(timestampCurrent_, startOfIteration, elapsedTime );

	DiagElapsedTime_t iterationPeriod = pbdt->GetIterationPeriod();

	if (elapsedTime >= iterationPeriod) {
		
		timeToRun = TRUE;
	}

	return timeToRun;
}

void BlackfinDiagScheduler::ResetTestsCompletedForCycle(BlackfinDiagTest * & pbdt) {

	pbdt->SetNumberOfTimesRanThisDiagCycle(0);
}

void BlackfinDiagScheduler::RunScheduled(){
	
	// Start Fault Injection Point 2
	
	// Code which sets InjectFaultFlag after DGN_FI_DELAY_SEC has passed
	// to trigger an injected fault at run-time will be injected here.
	// End Fault Injection Point 2

	// Check if it's time for Io shutdown test
	//if (false)//ApexDiagIoShutdown::IsTimeToRun(HI_ApexReg.SystemTime))
	//{
		//ApexDiagIoShutdown::RunTest(HI_ApexReg.SystemTime);
	//}
	// Then check if it's time for generic diagnostic test (unsigned math handles roll-over)
	//else if ((GetSystemTime() - m_LastDiagTime) > DiagnosticSlicePeriod_Microseconds)
    
	DetermineCurrentSchedulerState();	

	switch (currentSchedulerState_) {

		case INITIAL_INSTANTIATION:
		case MAX_PERIOD_EXPIRED_ALL_TESTS_COMPLETE:		
		{
			currentSchedulerState_ = NO_NEW_SCHEDULING_PERIOD;
			
			SetDiagTestsReadyForNewTestCycle();			
		}
		
		break;
		
		case MAX_PERIOD_EXPIRED_INCOMPLETE_TESTING:
		{
			
			UINT32 errorCode = allDiagTestsDidNotComplete_;
			
			ConfigureErrorCode( errorCode, BlackfinDiagTesting::BlackfinDiagTest::DiagSchedulerTestType );
				
			OS_Assert( errorCode );
		}
		
		// We won't get here now but in case it ever changes
		break;
		
		case TEST_ITERATIONS_SCHEDULED:
		{
			DoMoreDiagnosticTesting();
		}
		
		break;
		
		case NO_NEW_SCHEDULING_PERIOD:
		case NO_TESTS_TO_RUN_ALL_COMPLETED:
		case NO_TEST_ITERATIONS_SCHEDULED:
		
		break;
	}
}


void BlackfinDiagScheduler::SetAnotherTestCompletedForCycle(BlackfinDiagTest * & pbdt) {

	UINT32 numberOfTimesRan = pbdt->GetNumberOfTimesRanThisDiagCycle();
	
	++numberOfTimesRan;
	
	pbdt->SetNumberOfTimesRanThisDiagCycle( numberOfTimesRan );
}

void BlackfinDiagScheduler::SetDiagTestsReadyForNewTestCycle() {
	
	for (std::vector<BlackfinDiagTest *>::iterator it = runTimeDiagnostics_->begin(); it != runTimeDiagnostics_->end(); ++it) {
		
		BlackfinDiagTest * pdt = (*it);
		
		ResetTestsCompletedForCycle( pdt );
	}
}
			
BOOL BlackfinDiagScheduler::StartEnumeratingTestsForThisIterationPeriod() {
	
	std::vector<BlackfinDiagTest *>::iterator it;
	
	BOOL testsCanRun = FALSE;
	
	for (it = runTimeDiagnostics_->begin(); it != runTimeDiagnostics_->end(); ++it) {
		
		BlackfinDiagTest * pdt = (*it);
				
		BOOL testingNotCompleteForThisDiagCycle = !IsTestingCompleteForDiagCycle(pdt);
		
		if ( testingNotCompleteForThisDiagCycle ) {
			
			BOOL testIsScheduledToRun = IsTestScheduledToRun(pdt);
			
			if ( testIsScheduledToRun ) {
				
				testsCanRun = TRUE;
				
				break;
			}
						
		}
	}
	
	itTestEnumeration_ = it;
	
	return testsCanRun;
}				
		
				



BlackfinDiagScheduler::BlackfinDiagScheduler() {

}





	    		





