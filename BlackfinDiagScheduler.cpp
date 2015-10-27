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
	
	
	BOOL newDiagTestCycle = IsTimeForNewDiagCycle();
	
	if ( newDiagTestCycle )
	{
		BlackfinDiagTest::DiagnosticTestTypes testType;
				
		BOOL testsNotCompleted = !DidAllTestsComplete( testType );
		
		if ( testsNotCompleted ) {
			
			UINT32 errorCode = allDiagTestsDidNotComplete_;
			
			ConfigureErrorCode( errorCode, testType );
				
//			firmExcept( errorCode );
		}			

		SaveDiagCycleStartData();
		
		SetDiagTestsReadyForNewCycle();		
	}
	
	RunTestsForCurrentTimeslice();
}

BOOL BlackfinDiagScheduler::DidAllTestsComplete( BlackfinDiagTest::DiagnosticTestTypes & testType ) {

	BOOL allTestsCompleted = TRUE;
	
	for (std::vector<BlackfinDiagTest *>::iterator it = runTimeDiagnostics_->begin(); it != runTimeDiagnostics_->end(); ++it) {
		
		BlackfinDiagTest * pdt = (*it);
			
        BOOL testDidNotComplete = !pdt->IsTestingCompleteForDiagCycle();
        
        if ( testDidNotComplete ) {
        	
        	testType = pdt->GetTestType();
        	
        	allTestsCompleted = FALSE;
		
			break;
        }
	}
	
	return allTestsCompleted;
}
		
BOOL BlackfinDiagScheduler::IsTimeForNewDiagCycle() {

	BOOL timeForNewCycle = FALSE;
	
	if ( schedulerInstantiationComplete_ ) { 
		
		DiagTime_t currentTime = GetSystemTime();
		
		DiagTime_t elapsedTimeForThisCycle = currentTime - timeCycleStarted_;
		
		if ( elapsedTimeForThisCycle >= PeriodForAllDiagnosticsCompleted_milleseconds ) {
			
			timeForNewCycle = TRUE;
		}
	}
	else {

		schedulerInstantiationComplete_ = TRUE;
		
		timeForNewCycle = TRUE;
	}
	
	return timeForNewCycle;
}

void BlackfinDiagScheduler::SaveDiagCycleStartData() {
	m_LastDiagTime_ = GetSystemTime();
}

void BlackfinDiagScheduler::SetDiagTestsReadyForNewCycle() {
	
	for (std::vector<BlackfinDiagTest *>::iterator it = runTimeDiagnostics_->begin(); it != runTimeDiagnostics_->end(); ++it) {
		
		BlackfinDiagTest * pdt = (*it);
		
		pdt->ResetTestsCompletedForCycle();
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
	
	DiagTime_t elapsedTime = GetSystemTime();
	
	DiagTime_t newTime = elapsedTime;
	
	elapsedTime  -= m_LastDiagTime_;
	
	if ( elapsedTime > BlackfinDiagTest::GetTimeslicePeriod_milleseconds() ) {
	
		m_LastDiagTime_ = newTime;
		
		timeForNewTimeslicePeriod = TRUE;
	}
	
	return timeForNewTimeslicePeriod;
}

BOOL BlackfinDiagScheduler::AreAnyActiveTestsTakingTooLong( BlackfinDiagTest::DiagnosticTestTypes & TestType ) {
	
	BOOL thereAreTestsTakingTooLong = FALSE;
	
	for (std::vector<BlackfinDiagTest *>::iterator it = runTimeDiagnostics_->begin(); it != runTimeDiagnostics_->end(); ++it) {
		
		BlackfinDiagTest * pdt = (*it);
			
		BOOL testExecutionTimeLimitExceeded = 	pdt->HasTestTakenTooLongToRun();
		
		if ( testExecutionTimeLimitExceeded )
		{
			UINT32 errorCode = diagTestTooLongToComplete_;
				
			ConfigureErrorCode( errorCode, (*it)->GetTestType() );
				
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
    		
    		// Record the entry time before calling diag
    		UINT32 entryTime_ms = GetSystemTime();

			UINT32	returnedErrorCode;
	
			BlackfinDiagTest::TestState testResult = pCurrentDiagTest->RunTest( returnedErrorCode, entryTime_ms );

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
				
		BOOL testingNotCompleteForThisDiagCycle = !pdt->IsTestingCompleteForDiagCycle();
		
		if ( testingNotCompleteForThisDiagCycle ) {
			
			BOOL testIsScheduledToRun = pdt->IsTestScheduledToRun();
			
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
				
		BOOL testingNotCompleteForThisDiagCycle = !pdt->IsTestingCompleteForDiagCycle();
		
		if ( testingNotCompleteForThisDiagCycle ) {
			
			BOOL testIsScheduledToRun = pdt->IsTestScheduledToRun();
			
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



