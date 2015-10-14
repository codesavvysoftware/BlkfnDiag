#include "BlackfinDiagScheduler.h"
#include "OS_iotk.h"

BlackfinDiagScheduler::BlackfinDiagScheduler(std::vector <BlackfinDiagTest *> * Diagnostics) 
	: RunTimeDiagnostics(Diagnostics) {

	if ( !Diagnostics ) {
		
		UINT32 ErrorCode = CorruptedDiagTestVector;
		
		ConfigureErrorCode( ErrorCode, BlackfinDiagTest::DiagSchedulerTestType );
		
		firmExcept( ErrorCode );
	}

	UINT32 ui = 0;
		
	for (std::vector<BlackfinDiagTest *>::iterator it = RunTimeDiagnostics->begin(); it != RunTimeDiagnostics->end(); ++it) {

		BlackfinDiagTest * pdt = (*it);
			
		pdt->SetTimesliceForNextTestIteration( ui++ );
	}

	m_LastDiagTime = GetSystemTime();
		
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
	if ( (GetSystemTime() - m_LastDiagTime) > BlackfinDiagTest::GetTimeslicePeriod_microseconds() ) {
	
		// Execute a runtime diagnostic if it's time for one. Scheduling of the various tests is
		// handled within ExecuteSlice(), and is guided by the m_LastDiagTime variable, which
		// increments each time through.
		m_LastDiagTime = GetSystemTime();
		
		ExecuteSlice();
	}
	// Finally check if it's time to run diagnostic completion time check.
	else if ((m_SliceNumber - m_LastCompletionCheck) > DGN_COMPL_CHECK_INTERVAL_TIME_SLICE)
	{
		m_LastCompletionCheck = m_SliceNumber;
	
		CompletionTimeCheck();
	}
	else
	{
		// nothing to do
	}
}

void BlackfinDiagScheduler::CompletionTimeCheck()
{
	for (std::vector<BlackfinDiagTest *>::iterator it = RunTimeDiagnostics->begin(); it != RunTimeDiagnostics->end(); ++it) {
		
		BlackfinDiagTest * pdt = (*it);
			
		BOOL bTestExecutionTimeLimitExceeded = 	pdt->HasTestTakenTooLongToRun( m_SliceNumber );
		
		if (bTestExecutionTimeLimitExceeded)
		{
			UINT32 ErrorCode = DiagTestTooLongToComplete;
				
			ConfigureErrorCode( ErrorCode, (*it)->GetTestType() );
				
			firmExcept( ErrorCode );
		}
	}
}

BOOL BlackfinDiagScheduler::FindTestIndexForThisSlice(BlackfinDiagTest * & pbdtNextDiag)
{
	BOOL bSuccess = FALSE;

	for (std::vector<BlackfinDiagTest *>::iterator it = RunTimeDiagnostics->begin(); it != RunTimeDiagnostics->end(); ++it) {
		
		BlackfinDiagTest * pdt = (*it);
				
		BOOL bTimeToRunNextTest = pdt->IsItTimeToScheduleNextTest( m_SliceNumber );
		
		if ( bTimeToRunNextTest ) {

			pbdtNextDiag = pdt;
					
			bSuccess = TRUE;
					
			break;
		}
	}
	
	return bSuccess;
}
void BlackfinDiagScheduler::ExecuteSlice()
{
	//LogDgnTrace(0xbbbbbbbb);

	// Do hardware watchdog handing
	//ApexWatchdog::Service();

	BlackfinDiagTest * pCurrentDiagTest = NULL;
			
	BOOL bNotTimeToRunADiagnostic = !FindTestIndexForThisSlice(pCurrentDiagTest);

	if (bNotTimeToRunADiagnostic) {
		// Update diag slice number.
		m_SliceNumber++;

		return;
	}
		
	//LogDgnTrace(testIndex);

	BlackfinDiagTest::DiagnosticTestTypes TestTypeCurrent = pCurrentDiagTest->GetTestType();
	
	pCurrentDiagTest->UpdateTimesliceForNextTestIteration();

	// Record the entry time before calling diag
	UINT32 entryTimeUs = GetSystemTime();

	UINT32	ReturnedErrorCode;
	
	BlackfinDiagTest::TestState testResult = pCurrentDiagTest->RunTest( ReturnedErrorCode, GetSystemTime() );

	switch (testResult)
	{
	case BlackfinDiagTest::TEST_LOOP_COMPLETE: 
	{
		pCurrentDiagTest->SetMaxTestExecutionPeriod_Timeslices( m_SliceNumber );  
		
		pCurrentDiagTest->SetTestComplete( m_SliceNumber );
		
		pCurrentDiagTest->SetCompletedTestDuration_microseconds( GetSystemTime() );
	
		break;
	}

	case BlackfinDiagTest::TEST_IN_PROGRESS:
	{
		// If the test has successfully run to the timelimit within the
		// timeslice, but has not completed all iterations, don't bother
		// with the maximum time calculations.  However, for debugging purposes,
		// save the amount of time that the diagnostic function took to run
		// during this timeslice into its diag CB.
		pCurrentDiagTest->SetCompletedTestDuration_microseconds( GetSystemTime() );
		
		break;
	}

	default:
	{
		ConfigureErrorCode( ReturnedErrorCode, TestTypeCurrent );
		
		firmExcept( ReturnedErrorCode );
		
		break;
	}
	};

	RescheduleLowerPriorityDiagnostics();

	//LogDgnTrace(0xeeeeeeee);

	// Update diag slice number.
	m_SliceNumber++;
}

void BlackfinDiagScheduler::SchedulerInit(void) {
	
	m_SliceNumber = 0;
	m_LastCompletionCheck = 0;
}

//static  const BOOL DEBUG_LOGGING_ENABLED = TRUE;

void BlackfinDiagScheduler::LogDgnTrace(const UINT32 val) {
		
	static UINT32 index = 0;

	static const bool DEBUG_LOGGING_ENABLED
	#if defined(DEBUG)
	= true;
	#else
	= false;
	#endif
	if (DEBUG_LOGGING_ENABLED) {
		
		if (index > (UINT32)((sizeof(DgnTrace) / 4) - 4))
		{
			index = 0;
		}
	
		DgnTrace[index++] = GetSystemTime();
		DgnTrace[index++] = val;
	}
	else
	{
		if (index >= TRACE_LOG_SIZE)
		{
			index = 0;
		}

		m_TraceLog[index].timestamp = GetSystemTime();
		m_TraceLog[index].value = val;
		index++;
	}
}


void BlackfinDiagScheduler::RescheduleLowerPriorityDiagnostics()
{
	BlackfinDiagTest * pdbtPrev = NULL;

	BlackfinDiagTest * pdbtCurrent = NULL;

	for (std::vector<BlackfinDiagTest *>::iterator it = (RunTimeDiagnostics->begin() + 1); it != RunTimeDiagnostics->end(); ++it) {
			
		pdbtPrev = *(&(*(it-1)));

		pdbtCurrent = *(&(*it));

		pdbtCurrent->AdjustForAnyScheduleTimeConflict( pdbtPrev );
	}
}

void BlackfinDiagScheduler::ConfigureErrorCode( UINT32 & ReturnedErrorCode, BlackfinDiagTest::DiagnosticTestTypes TestTypeCurrent ) {
		
	UINT32 ui32        = TestTypeCurrent;
	ReturnedErrorCode &= DiagnosticErrorNumberMask;
	ReturnedErrorCode |= (ui32 << DiagnosticErrorTestTypeBitPos); 
}



