#include "BlackfinDiagScheduler.h"
#include "OS_iotk.h"

using namespace BlackfinDiagTests;
using namespace DiagnosticCommon;

BlackfinDiagScheduler::BlackfinDiagScheduler(std::vector <BlackfinDiagTest *> * diagnostics) 
		:	runTimeDiagnostics_              ( diagnostics ),
			itTestEnumeration_               ( diagnostics->end() ),
			m_LastDiagTime_                  ( 0 ),
			schedulerInstantiationComplete_  ( FALSE ),
			timeCycleStarted_                ( 0 ) {

	if ( !diagnostics ) {
		
		UINT32 errorCode = corruptedDiagTestVector_;
		
		ConfigureErrorCode( errorCode, BlackfinDiagTest::DiagSchedulerTestType );
		
		firmExcept( errorCode );
	}

	for (std::vector<BlackfinDiagTest *>::iterator it = runTimeDiagnostics_->begin(); it != runTimeDiagnostics_->end(); ++it) {

		BlackfinDiagTest * pdt = (*it);
			
		pdt->SetInitialSchedule();
	}

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
	timestamp = GetSystemTimestamp();
	
	elapsedTimeFromLastCycle = ComputeElapsedTime(timestamp, lastDiagCycleRanTimestamp);

	elapsedTimeFromLastTimeslice = ComputeElapsedTime(timestamp, lastTimesliceRanTimestamp);

	BOOL newDiagTestCycle = IsTimeForNewDiagCycle( elapsedTimeFromLastCycle );

	if ( newDiagTestCycle )
	{
		lastDiagCycleRanTimestamp = timestamp;

		elapsedTimeFromLastTimeslice = 0;

		BlackfinDiagTest::DiagnosticTestTypes testType;
				
		BOOL testsNotCompleted = !DidAllTestsComplete( testType );
		
		if ( testsNotCompleted ) {
			
			UINT32 errorCode = allDiagTestsDidNotComplete_;
			
			ConfigureErrorCode( errorCode, testType );
				
//			firmExcept( errorCode );
		}			

		SetDiagTestsReadyForNewCycle();		
	}
	
	RunTestsForCurrentTimeslice();
}

BOOL BlackfinDiagScheduler::DidAllTestsComplete( BlackfinDiagTest::DiagnosticTestTypes & testType ) {

	BOOL allTestsCompleted = TRUE;
	
	for (std::vector<BlackfinDiagTest *>::iterator it = runTimeDiagnostics_->begin(); it != runTimeDiagnostics_->end(); ++it) {
		
		BlackfinDiagTest * pdt = (*it);
			
        BOOL testDidNotComplete = !IsTestingCompleteForDiagCycle(pdt);
        
        if ( testDidNotComplete ) {
        	
        	testType = pdt->GetTestType();
        	
        	allTestsCompleted = FALSE;
		
			break;
        }
	}
	
	return allTestsCompleted;
}
		
BOOL BlackfinDiagScheduler::IsTimeForNewDiagCycle( DiagElapsedTime_t elapsed_time ) {

	BOOL timeForNewCycle = FALSE;
	
	if ( schedulerInstantiationComplete_ ) { 
		
		if ( elapsed_time >= PeriodForAllDiagnosticsCompleted_milleseconds ) {
			
			timeForNewCycle = TRUE;
		}
	}
	else {

		schedulerInstantiationComplete_ = TRUE;
		
		timeForNewCycle = TRUE;
	}
	
	return timeForNewCycle;
}


void BlackfinDiagScheduler::SetDiagTestsReadyForNewCycle() {
	
	for (std::vector<BlackfinDiagTest *>::iterator it = runTimeDiagnostics_->begin(); it != runTimeDiagnostics_->end(); ++it) {
		
		BlackfinDiagTest * pdt = (*it);
		
		ResetTestsCompletedForCycle( pdt );
	}
}
			
void BlackfinDiagScheduler::RunTestsForCurrentTimeslice() {
	
	BOOL newTimeSlicePeriodStarting = IsTimeForNewTimeslicePeriod();
	
	if ( newTimeSlicePeriodStarting ) {        
		
		DoMoreDiagnosticTesting();
		
	}
	else {

		BlackfinDiagTest::DiagnosticTestTypes  testType;

		BOOL anActiveTestTakingTooLongToFinish = AreAnyActiveTestsTakingTooLong( testType );
		
		if ( anActiveTestTakingTooLongToFinish ) {
			
			UINT32 errorCode = diagTestTooLongToComplete_;
				
			ConfigureErrorCode( errorCode, testType );
				
			firmExcept( errorCode );
		}
	}
}


BOOL BlackfinDiagScheduler::IsTimeForNewTimeslicePeriod() {
	
	BOOL timeForNewTimeslicePeriod = FALSE;
	
	if ( elapsedTimeFromLastTimeslice > BlackfinDiagTest::GetTimeslicePeriod_milleseconds() ) {
	
		lastTimesliceRanTimestamp = timestamp;

		timeForNewTimeslicePeriod = TRUE;
	}
	
	return timeForNewTimeslicePeriod;
}

BOOL BlackfinDiagScheduler::AreAnyActiveTestsTakingTooLong( BlackfinDiagTest::DiagnosticTestTypes & TestType ) {
	
	BOOL thereAreTestsTakingTooLong = FALSE;
	
	for (std::vector<BlackfinDiagTest *>::iterator it = runTimeDiagnostics_->begin(); it != runTimeDiagnostics_->end(); ++it) {
		
		BlackfinDiagTest * pdt = (*it);
			
		BOOL testExecutionTimeLimitExceeded = 	HasTestTakenTooLongToRun( pdt );
		
		if ( testExecutionTimeLimitExceeded )
		{
			UINT32 errorCode = diagTestTooLongToComplete_;
				
			ConfigureErrorCode( errorCode, pdt->GetTestType() );
				
			firmExcept( errorCode );
		}
	}
	
	return thereAreTestsTakingTooLong;
}
	
void BlackfinDiagScheduler::DoMoreDiagnosticTesting() {
	
	StartEnumeratingTestsForThisTimeslice();
	
	BOOL testIterationCanRun = TRUE;

    while( testIterationCanRun ) {
    	
    	BlackfinDiagTest * pCurrentDiagTest = NULL;
			
    	testIterationCanRun = EnumerateNextScheduledTest( pCurrentDiagTest );
    	
    	if ( testIterationCanRun ) {
    		
			UINT32	returnedErrorCode;

			BOOL testStarting = IsTestStarting(pCurrentDiagTest);

			if (testStarting) {
				pCurrentDiagTest->SetStartOfTestTimestamp(timestamp);
			}

			pCurrentDiagTest->SetStartOfTestIterationTimestamp(timestamp);
	
			BlackfinDiagTest::TestState testResult = pCurrentDiagTest->RunTest( returnedErrorCode );

    		pCurrentDiagTest->SetCurrentTestState( testResult );
    
			switch (testResult)
			{
				case BlackfinDiagTest::TEST_LOOP_COMPLETE: 
				{
					pCurrentDiagTest->SetAnotherTestCompletedForCycle();
				
					pCurrentDiagTest->UpdateMaxDurationTime();
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
    


void BlackfinDiagScheduler::StartEnumeratingTestsForThisTimeslice() {
	
	std::vector<BlackfinDiagTest *>::iterator it;
	
	for (it = runTimeDiagnostics_->begin(); it != runTimeDiagnostics_->end(); ++it) {
		
		BlackfinDiagTest * pdt = (*it);
				
		BOOL testingNotCompleteForThisDiagCycle = !IsTestingCompleteForDiagCycle(pdt);
		
		if ( testingNotCompleteForThisDiagCycle ) {
			
			BOOL testIsScheduledToRun = IsTestScheduledToRun(pdt);
			
			BOOL testIsInProgress = pdt->IsTestInProgress();
			
			if ( testIsScheduledToRun || testIsInProgress ) {
				
				break;
			}
						
		}
	}
	
	itTestEnumeration_ = it;
}				

BOOL BlackfinDiagScheduler::EnumerateNextScheduledTest( BlackfinDiagTest * & pbdtNextDiag ) {
	
	BOOL success = FALSE;

    while( itTestEnumeration_ != runTimeDiagnostics_->end() ) {

    	BlackfinDiagTest * pdt = (*itTestEnumeration_);
				
		++itTestEnumeration_;
				
		BOOL testingNotCompleteForThisDiagCycle = !IsTestingCompleteForDiagCycle(pdt);
		
		if ( testingNotCompleteForThisDiagCycle ) {
			
			BOOL testIsScheduledToRun = IsTestScheduledToRun(pdt);
			
			BOOL testIsInProgress = pdt->IsTestInProgress();
			
			if ( testIsScheduledToRun || testIsInProgress ) {
				
				pbdtNextDiag = pdt;
				
				success = TRUE;
				
				break;
			}
		}
    }						
    		
	return success;
}


void BlackfinDiagScheduler::ConfigureErrorCode( UINT32 & returnedErrorCode, BlackfinDiagTest::DiagnosticTestTypes testTypeCurrent ) {
		
	UINT32 ui32        = testTypeCurrent;
	returnedErrorCode &= DiagnosticErrorNumberMask;
	returnedErrorCode |= (ui32 << DiagnosticErrorTestTypeBitPos); 
}

BlackfinDiagScheduler::BlackfinDiagScheduler() {

}



BOOL BlackfinDiagScheduler::HasTestTakenTooLongToRun(BlackfinDiagTest * & pbdt) {

	BOOL testExecutionTooLong = FALSE;

	DiagTimestampTime_t testStartedTimestamp = pbdt.GetStartOfTestTimestamp();

	DiagElapsedTime_t elapsed_time = ComputeElapsedTime(timestamp, testStartedTimestamp);

	DiagElapsedTime_t max_time_for_test = pbdt.GetMaxTimeForTestToComplete();

	if (elapsedTime > max_time_for_test) {
		testExecutionTooLong = TRUE;
	}

	return testExecutionTooLong;
}

void BlackfinDiagScheduler::ResetTestsCompletedForCycle(BlackfinDiagTest * & pbdt) {

	pbdt.ResetCompleteForDiagCycle();

	pbdt.SetTimesRanThisDiagCycle();
}

void BlackfinDiagScheduler::SetAnotherTestCompletedForCycle(BlackfinDiagTest * & pbdt) {

	pbdt.SetCompleteForDiagCycle();

	pbdt.IncrementTimesRanThisDiagCycle(0);
}

BOOL BlackfinDiagScheduler::IsTestStarting(BlackfinDiagTest * & pbdt ) {
	testCompleteForDiagCycle = pbdt.GetCompeteForDiagCycleStatus();
	
	testRepeatedSpecifiedNumberOfTimes = pbdt.GetTestRanSpecifiedNumberOfTimesStatus();
	
	return (testCompleteForDiagCycle && testRepeatedSpecifiedNumberOfTimes);
}

BOOL BlackfinDiagScheduler::IsTestingCompleteForDiagCycle(BlackfinDiagTest * & pbdt) {

	BOOL testingForCycleCompleted = FALSE;

	BOOL testsHaveCompleteAtLeastOnce = HaveTestsCompletedAtLeastOnce(pbdt);

	BOOL noMoreCompleteTestsToRun = !AreThereMoreCompleteTestsToRunThisDiagCycle();

	BOOL testIsNotRunning = !IsTestInProgress();

	if (testsHaveCompleteAtLeastOnce
		&& noMoreCompleteTestsToRun) {

		testingForCycleCompleted = TRUE;
	}
	else {
		int i = 0;

		i++;
	}

	return testingForCycleCompleted;
}

BOOL BlackfinDiagTest::IsTestScheduledToRun(BlackfinDiagTest * & pbdt) {

	BOOL timeToRun = FALSE;

	DiagTimestampTime_t startOfIteration = pbdt->GetStartOfTestIterationTimestamp(timestamp);

	DiagElapsedTime_t elapsedTime = ComputeElapsedTime(timestamp, startOfIteration);

	DiagElapsedTime_t iterationPeriod = pbdt->GetIterationPeriod();

	if (elapsedTime >= iterationPeriod) {
		timeToRun = TRUE;
	}

	return timeToRun;
}
