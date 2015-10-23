#include "BlackfinDiagScheduler.h"
#include "OS_iotk.h"

BlackfinDiagScheduler::BlackfinDiagScheduler(std::vector <BlackfinDiagTest *> * Diagnostics) 
		:	RunTimeDiagnostics              ( Diagnostics ),
			itTestEnumeration               ( Diagnostics->end() ),
			m_LastDiagTime                  ( 0 ),
			bSchedulerInstantiationComplete ( FALSE ),
			TimeCycleStarted                ( 0 ) {

	if ( !Diagnostics ) {
		
		UINT32 ErrorCode = CorruptedDiagTestVector;
		
		ConfigureErrorCode( ErrorCode, BlackfinDiagTest::DiagSchedulerTestType );
		
		firmExcept( ErrorCode );
	}

	for (std::vector<BlackfinDiagTest *>::iterator it = RunTimeDiagnostics->begin(); it != RunTimeDiagnostics->end(); ++it) {

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
	
	
	BOOL bNewDiagTestCycle = IsTimeForNewDiagCycle();
	
	if ( bNewDiagTestCycle )
	{
		BlackfinDiagTest::DiagnosticTestTypes TestType;
				
		BOOL bTestsNotCompleted = !DidAllTestsComplete( TestType );
		
		if ( bTestsNotCompleted ) {
			
			UINT32 ErrorCode = AllDiagTestsDidNotComplete;
			
			ConfigureErrorCode( ErrorCode, TestType );
				
			firmExcept( ErrorCode );
		}			

		SaveDiagCycleStartData();
		
		SetDiagTestsReadyForNewCycle();		
	}
	
	RunTestsForCurrentTimeslice();
}

BOOL BlackfinDiagScheduler::DidAllTestsComplete( BlackfinDiagTest::DiagnosticTestTypes & TestType ) {

	BOOL bAllTestsCompleted = TRUE;
	
	for (std::vector<BlackfinDiagTest *>::iterator it = RunTimeDiagnostics->begin(); it != RunTimeDiagnostics->end(); ++it) {
		
		BlackfinDiagTest * pdt = (*it);
			
        BOOL bTestDidNotComplete = !pdt->IsTestingCompleteForDiagCycle();
        
        if ( bTestDidNotComplete ) {
        	
        	TestType = pdt->GetTestType();
        	
        	bAllTestsCompleted = FALSE;
		
			break;
        }
	}
	
	return bAllTestsCompleted;
}
		
BOOL BlackfinDiagScheduler::IsTimeForNewDiagCycle() {

	BOOL bTimeForNewCycle = FALSE;
	
	if ( bSchedulerInstantiationComplete ) { 
		
		DiagTime_t CurrentTime = GetSystemTime();
		
		DiagTime_t ElapsedTimeForThisCycle = CurrentTime - TimeCycleStarted;
		
		if ( ElapsedTimeForThisCycle >= PeriodForAllDiagnosticsCompleted_milleseconds ) {
			
			bTimeForNewCycle = TRUE;
		}
	}
	else {

		bSchedulerInstantiationComplete = TRUE;
		
		bTimeForNewCycle = TRUE;
	}
	
	return bTimeForNewCycle;
}

void BlackfinDiagScheduler::SaveDiagCycleStartData() {
	m_LastDiagTime = GetSystemTime();
}

void BlackfinDiagScheduler::SetDiagTestsReadyForNewCycle() {
	
	for (std::vector<BlackfinDiagTest *>::iterator it = RunTimeDiagnostics->begin(); it != RunTimeDiagnostics->end(); ++it) {
		
		BlackfinDiagTest * pdt = (*it);
		
		pdt->ResetTestsCompletedForCycle();
	}
}
			
void BlackfinDiagScheduler::RunTestsForCurrentTimeslice() {
	
	BOOL bNewTimeSlicePeriodStarting = IsTimeForNewTimeslicePeriod();
	
	if ( bNewTimeSlicePeriodStarting ) {
		
		DoMoreDiagnosticTesting();
		
	}
	else {

		BlackfinDiagTest::DiagnosticTestTypes  TestType;

		BOOL bAnActiveTestTakingTooLongToFinish = AreAnyActiveTestsTakingTooLong( TestType );
		
		if ( bAnActiveTestTakingTooLongToFinish ) {
			
			UINT32 ErrorCode = DiagTestTooLongToComplete;
				
			ConfigureErrorCode( ErrorCode, TestType );
				
			firmExcept( ErrorCode );
		}
	}
}


BOOL BlackfinDiagScheduler::IsTimeForNewTimeslicePeriod() {
	
	BOOL bTimeForNewTimeslicePeriod = FALSE;
	
	DiagTime_t ElapsedTime = GetSystemTime();
	
	DiagTime_t NewTime = ElapsedTime;
	
	ElapsedTime  -= m_LastDiagTime;
	
	if ( ElapsedTime > BlackfinDiagTest::GetTimeslicePeriod_milleseconds() ) {
	
		m_LastDiagTime = NewTime;
		
		bTimeForNewTimeslicePeriod = TRUE;
	}
	
	return bTimeForNewTimeslicePeriod;
}

BOOL BlackfinDiagScheduler::AreAnyActiveTestsTakingTooLong( BlackfinDiagTest::DiagnosticTestTypes & TestType ) {
	
	BOOL bThereAreTestsTakingTooLong = FALSE;
	
	for (std::vector<BlackfinDiagTest *>::iterator it = RunTimeDiagnostics->begin(); it != RunTimeDiagnostics->end(); ++it) {
		
		BlackfinDiagTest * pdt = (*it);
			
		BOOL bTestExecutionTimeLimitExceeded = 	pdt->HasTestTakenTooLongToRun();
		
		if (bTestExecutionTimeLimitExceeded)
		{
			UINT32 ErrorCode = DiagTestTooLongToComplete;
				
			ConfigureErrorCode( ErrorCode, (*it)->GetTestType() );
				
			firmExcept( ErrorCode );
		}
	}
	
	return bThereAreTestsTakingTooLong;
}
	
void BlackfinDiagScheduler::DoMoreDiagnosticTesting() {
	
	StartEnumeratingTestsForThisTimeslice();
	
	BOOL bTestIterationCanRun = TRUE;

    while( bTestIterationCanRun ) {
    	
    	BlackfinDiagTest * pCurrentDiagTest = NULL;
			
    	BOOL bTestIterationCanRun = EnumerateNextScheduledTest( pCurrentDiagTest );
    	
    	if ( bTestIterationCanRun ) {
    		
    		// Record the entry time before calling diag
    		UINT32 entryTime_ms = GetSystemTime();

			UINT32	ReturnedErrorCode;
	
			BlackfinDiagTest::TestState testResult = pCurrentDiagTest->RunTest( ReturnedErrorCode, entryTime_ms );

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
					ConfigureErrorCode( ReturnedErrorCode, pCurrentDiagTest->GetTestType() );
		
					firmExcept( ReturnedErrorCode );
				}
			
					break;
			}
		}
	}
}
    


void BlackfinDiagScheduler::StartEnumeratingTestsForThisTimeslice() {
	
	std::vector<BlackfinDiagTest *>::iterator it;
	
	for (it = RunTimeDiagnostics->begin(); it != RunTimeDiagnostics->end(); ++it) {
		
		BlackfinDiagTest * pdt = (*it);
				
		BOOL bTestingNotCompleteForThisDiagCycle = !pdt->IsTestingCompleteForDiagCycle();
		
		if ( bTestingNotCompleteForThisDiagCycle ) {
			
			BOOL bTestIsScheduledToRun = pdt->IsTestScheduledToRun();
			
			BOOL bTestIsInProgress = pdt->IsTestInProgress();
			
			if ( bTestIsScheduledToRun || bTestIsInProgress ) {
				
				break;
			}
						
		}
	}
	
	itTestEnumeration = it;
}				

BOOL BlackfinDiagScheduler::EnumerateNextScheduledTest( BlackfinDiagTest * & pbdtNextDiag ) {
	
	BOOL bSuccess = FALSE;

    while( itTestEnumeration != RunTimeDiagnostics->end() ) {

    	BlackfinDiagTest * pdt = (*itTestEnumeration);
				
		itTestEnumeration++;
				
		BOOL bTestingNotCompleteForThisDiagCycle = !pdt->IsTestingCompleteForDiagCycle();
		
		if ( bTestingNotCompleteForThisDiagCycle ) {
			
			BOOL bTestIsScheduledToRun = pdt->IsTestScheduledToRun();
			
			BOOL bTestIsInProgress = pdt->IsTestInProgress();
			
			if ( bTestIsScheduledToRun || bTestIsInProgress ) {
				
				pbdtNextDiag = pdt;
				
				bSuccess = TRUE;
				
				break;
			}
		}
    }						
    		
	return bSuccess;
}


void BlackfinDiagScheduler::ConfigureErrorCode( UINT32 & ReturnedErrorCode, BlackfinDiagTest::DiagnosticTestTypes TestTypeCurrent ) {
		
	UINT32 ui32        = TestTypeCurrent;
	ReturnedErrorCode &= DiagnosticErrorNumberMask;
	ReturnedErrorCode |= (ui32 << DiagnosticErrorTestTypeBitPos); 
}



