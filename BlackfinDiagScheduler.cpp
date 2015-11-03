#include "BlackfinDiagScheduler.h"
#include "OS_iotk.h"
#include <time.h>

using namespace BlackfinDiagTests;
using namespace DiagnosticCommon;

BlackfinDiagScheduler::BlackfinDiagScheduler(std::vector <BlackfinDiagTest *> * diagnostics) 
		:	currentSchedulerState_( INITIAL_INSTANTIATION ),
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
		
		firmExcept( errorCode );
		
	
	}

}

BOOL BlackfinDiagScheduler::AreTestIterationsScheduledToRun() {
	
	return StartEnumeratingTestsForThisIterationPeriod();
}	    		

void BlackfinDiagScheduler::ComputeElapsedTime( DiagTimestampTime_t current, DiagTimestampTime_t previous, DiagElapsedTime_t & elapsed  ) {
	
	DiagTimestampTime_t diff       = current - previous; // difference in clock cycles;
	
	// An approximation that is actually very close when CLOCKS_PER_SEC == 600000000
	// avoiding a constant divide in the background
	// in the real code probably will be simpler.
	//
	// Math for the approximation
	//  CLOCKS_PER_SEC == 600000000
	//  CLOCKS_PER_MILLESECOND = CLOCKS_PER_SEC * SECONDS_PER_MILLESECOND = 600000000 / 1000 = 600000
	//  Elapsed time in milleseconds = difference in clock readings / CLOCKS_PER_MILLESECOND = diff / 600000;
	//  600000 == 0x927c0
	//  We're looking for a shift that is less which would be 0x80000 ==  524288.
	//  600000 ~= 524288 * 1.14441
	//  Elapsed time in millesconds ~= diff / (524288 * 1.1441) ~=  ( diff / 0x8000 ) * (1/1.1441) 
	//                                                          ~=  ( diff >> 19 ) * (.8738 )
	//                                                          ~=  ( diff >> 19 ) * ( 7/8 )
	//                                                          ~=  ( diff >> 19 ) ( 1 - 1/8 )
	//                                                          ~=  ( diff >> 19 ) - ( ( diff >> 19 ) * 1/8)
	//                                                          ~=  ( diff >> 19 ) - ( ( diff >> 19 ) >> 3 );
	//                                                        substitute  fast for ( diff >> 19 );
	                                                            
	DiagTimestampTime_t   fast = diff >> 19;
	
	fast -= (fast >> 3 );

	elapsed = fast;  // difference in clock cycles times milleseconds per clock cycle
	                                        // yields elapsed time in milleseconds
//	DiagTimestampTime_t rate = CLOCKS_PER_SEC;
	
//	diff *= 1000;
	
//	diff /= rate;     // milleseconds per clock cycle is what ends up in multiplier
}
	

void BlackfinDiagScheduler::ConfigureErrorCode( UINT32 & returnedErrorCode, BlackfinDiagTest::DiagnosticTestTypes testTypeCurrent ) {
		
	UINT32 ui32        = testTypeCurrent;
	returnedErrorCode &= DiagnosticErrorNumberMask;
	returnedErrorCode |= (ui32 << DiagnosticErrorTestTypeBitPos); 
}

void BlackfinDiagScheduler::DetermineCurrentSchedulerState() {
	
	DiagElapsedTime_t elapsedTimeInTestCycle;
	
	timestampCurrent_ = GetSystemTimestamp();	
	
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
	ComputeElapsedTime( timestampCurrent_, timeTestCycleStarted_, elapsedTimeInTestCycle );
	
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
	
		ComputeElapsedTime( timestampCurrent_, timeLastIterationPeriodExpired_, elapsedTimeForCurrentIteration );
		
		if ( elapsedTimeForCurrentIteration > 50 ) {
			int i;
			
			i++;
		}

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
		
					firmExcept( returnedErrorCode );
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

DiagTimestampTime_t BlackfinDiagScheduler::GetSystemTimestamp( ) {
	return clock();
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
	
	ComputeElapsedTime(timestampCurrent_, startOfIteration, elapsedTime );

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
			
			ConfigureErrorCode( errorCode, BlackfinDiagTests::BlackfinDiagTest::DiagSchedulerTestType );
				
			firmExcept( errorCode );
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





	    		





