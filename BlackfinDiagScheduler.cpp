#include "BlackfinDiagScheduler.hpp"
#include "OS_iotk.h"
#include <time.h>

using namespace BlackfinDiagTesting;
using namespace DiagnosticCommon;

namespace DiagnosticScheduling
{	
    BlackfinDiagScheduler::BlackfinDiagScheduler( std::vector <BlackfinDiagTest *> * pDiagnostics,
                                                  UINT32                             corruptedVectorErr,
                                                  UINT32                             corruptedTestMemoryErr,
                                                  UINT32                             allDiagnosticsNotCompletedErr,
                                                  UINT32                             schedulerTestType ) 
    		  : m_CurrentSchedulerState          ( INITIAL_INSTANTIATION ),
    		 	m_pRunTimeDiagnostics            ( pDiagnostics ),
    		 	m_CorruptedVectorErr             ( corruptedVectorErr ),
    		 	m_CorruptedTestMemoryErr         ( corruptedTestMemoryErr ),
    		 	m_AllDiagnosticsNotCompletedErr  ( allDiagnosticsNotCompletedErr ),
    		 	m_SchedulerTestType              ( schedulerTestType ),
    			m_TestEnumeration                ( pDiagnostics->end() ),
    			m_TimestampCurrent               ( DEFAULT_INITIAL_TIMESTAMP ),
    			m_TimeTestCycleStarted           ( DEFAULT_INITIAL_TIMESTAMP ),
    			m_TimeLastIterationPeriodExpired ( DEFAULT_INITIAL_TIMESTAMP ) 
    {

    	// Number of timeslices between diagnostics completion time checks
    	// Start Fault Injection Point 3
    	// Declaration of DGN_COMPL_CHECK_INTERVAL_TIME_SLICE constant with a smaller value to make
    	// completion time diagnostic injected fault happen faster will be injected here.
    	//	static const DiagSlices_t DGN_COMPL_CHECK_INTERVAL_TIME_SLICE = 15 * DGN_INTERVALS_PER_MINUTE;

    	if ( !pDiagnostics ) 
    	{
		
    		UINT32 errorCode = m_CorruptedVectorErr;
		
    		ConfigureErrorCode( errorCode, m_SchedulerTestType );
		
    		OS_Assert( errorCode );	
    	}
    }

    BOOL BlackfinDiagScheduler::AreTestIterationsScheduledToRun() 
    {	
    	return StartEnumeratingTestsForThisIterationPeriod();
    }	
    		

	

    void BlackfinDiagScheduler::ConfigureErrorCode( UINT32 & returnedErrorCode, UINT32 testTypeCurrent ) 
    {
		
    	UINT32 ui32        = testTypeCurrent;
    	returnedErrorCode &= DIAG_ERROR_MASK;
    	returnedErrorCode |= (ui32 << DIAG_ERROR_TYPE_BIT_POS); 
    }

    void BlackfinDiagScheduler::DetermineCurrentSchedulerState() 
    {	
    	DiagElapsedTime elapsedTimeInTestCycle;
	
    	m_TimestampCurrent = SystemTiming.GetSystemTimestamp();	
	
    	// No state determination needed will get changed by caller
    	if ( m_CurrentSchedulerState == INITIAL_INSTANTIATION ) 
    	{		
    		//
    		// Sync everything to the same timestamp upon initial instantiation.
    		//
    		m_TimeTestCycleStarted = m_TimestampCurrent;
		
    		m_TimeLastIterationPeriodExpired = m_TimestampCurrent;
		
    		for (std::vector<BlackfinDiagTest *>::iterator it = m_pRunTimeDiagnostics->begin(); it != m_pRunTimeDiagnostics->end(); ++it) {
			
    			BlackfinDiagTest * pPbdt = (*it);
			
    			pPbdt->SetIterationCompletedTimestamp( m_TimestampCurrent );		
            }		
		
            return;
    	}

    	// Compute Elapsed Time in Current Diagnostic Test Period
    	SystemTiming.ComputeElapsedTimeMS( m_TimestampCurrent, m_TimeTestCycleStarted, elapsedTimeInTestCycle );
	
    	BOOL newDiagTestPeriod = HasCompleteDiagTestPeriodExpired( elapsedTimeInTestCycle );
	
    	if ( newDiagTestPeriod ) 
    	{		
    		m_TimeTestCycleStarted = m_TimestampCurrent;
		
    		BOOL testsCompleted = DidAllTestsComplete();
		
    		if ( testsCompleted ) 
    		{
    			m_CurrentSchedulerState = MAX_PERIOD_EXPIRED_ALL_TESTS_COMPLETE;
    		}
    		else 
    		{
    			m_CurrentSchedulerState = MAX_PERIOD_EXPIRED_INCOMPLETE_TESTING;
    		}
    	}
    	else 
    	{		
    		DiagElapsedTime elapsedTimeForCurrentIteration;
	
    		SystemTiming.ComputeElapsedTimeMS( m_TimestampCurrent, m_TimeLastIterationPeriodExpired, elapsedTimeForCurrentIteration );
		
    	    BOOL newTestIterationPeriod = HasNewTestIterationPeriodStarted( elapsedTimeForCurrentIteration );
	    
    	    if ( newTestIterationPeriod ) 
    	    {	    	
    	    	m_TimeLastIterationPeriodExpired = m_TimestampCurrent;
	    	
    	    	BOOL testsCompleted = DidAllTestsComplete();
	    	
    	    	if ( testsCompleted ) 
    	    	{
	    		
    	    		m_CurrentSchedulerState = NO_TESTS_TO_RUN_ALL_COMPLETED;
    	    	}
    	    	else {
	    		
    	    		BOOL newTestIterationsScheduledToRun = AreTestIterationsScheduledToRun();
	    		
    	    		if ( newTestIterationsScheduledToRun ) 
    	    		{
	    			
    	    			m_CurrentSchedulerState = TEST_ITERATIONS_SCHEDULED;
    	    		}
    	    		else 
    	    		{
    	    			m_CurrentSchedulerState = NO_TEST_ITERATIONS_SCHEDULED;
    	    		}
    	    	}
    	    }
    	    else 
    	    {
    	    	m_CurrentSchedulerState = NO_NEW_SCHEDULING_PERIOD;
    	    }
    	}
    }


			
    void BlackfinDiagScheduler::DoMoreDiagnosticTesting() 
    {
	
    	BOOL testIterationCanRun = TRUE;

        while( testIterationCanRun ) {
    	
        	BlackfinDiagTest * pCurrentDiagTest = NULL;
			
        	testIterationCanRun = EnumerateNextScheduledTest( pCurrentDiagTest );
    	
        	if ( testIterationCanRun ) 
        	{
    		
    			UINT32	returnedErrorCode;

    			TestState testResult = pCurrentDiagTest->RunTest( returnedErrorCode );

        		pCurrentDiagTest->SetCurrentTestState( testResult );
    
        		pCurrentDiagTest->SetIterationCompletedTimestamp( m_TimestampCurrent );
		
    			switch (testResult)
    			{
    				case TEST_LOOP_COMPLETE: 
    				
    					SetAnotherTestCompletedForCycle(pCurrentDiagTest);
			
    					break;
			
    				case TEST_IN_PROGRESS:
	
    					break;

    				default:
    				
    					ConfigureErrorCode( returnedErrorCode, pCurrentDiagTest->GetTestType() );
		
    					OS_Assert( returnedErrorCode );
			
    					break;
    			}
    		}
    	}
    }
    
			
    BOOL BlackfinDiagScheduler::DidAllTestsComplete() 
    {
    	BOOL allTestsCompleted = TRUE;
	
    	for (std::vector<BlackfinDiagTest *>::iterator it = m_pRunTimeDiagnostics->begin(); it != m_pRunTimeDiagnostics->end(); ++it) 
    	{		
    		BlackfinDiagTest * pPdt = (*it);
			
            BOOL testDidNotComplete = !IsTestingCompleteForDiagCycle(pPdt);
        
            if ( testDidNotComplete ) 
            {	
            	allTestsCompleted = FALSE;
		
    			break;
            }
    	}
	
    	return allTestsCompleted;
    }
		
    BOOL BlackfinDiagScheduler::EnumerateNextScheduledTest( BlackfinDiagTest * & rpPbdtNextDiag ) 
    {	
    	BOOL success = FALSE;

        while( m_TestEnumeration != m_pRunTimeDiagnostics->end() ) 
        {
        	BlackfinDiagTest * pPdt = (*m_TestEnumeration);
				
    		++m_TestEnumeration;
				
    		BOOL testingNotCompleteForThisDiagCycle = !IsTestingCompleteForDiagCycle(pPdt);
		
    		if ( testingNotCompleteForThisDiagCycle ) 
    		{	
    			BOOL testIsScheduledToRun = IsTestScheduledToRun(pPdt);
			
    			if ( testIsScheduledToRun ) 
    			{	
    				rpPbdtNextDiag = pPdt;
				
    				success = TRUE;
				
    				break;
    			}
    		}
        }						
    		
    	return success;
    }

    BOOL BlackfinDiagScheduler::HasCompleteDiagTestPeriodExpired( DiagElapsedTime elapsed_time ) 
    {
    	BOOL timeForNewCycle = FALSE;
	
    	if ( elapsed_time >= PERIOD_FOR_ALL_DIAGNOSTICS_COMPLETED_MS ) 
    	{	
    		timeForNewCycle = TRUE;
    	}

    	return timeForNewCycle;

    }

    BOOL BlackfinDiagScheduler::HasNewTestIterationPeriodStarted( DiagElapsedTime elapsedTimeForCurrentIteration ) 
    {	
    	BOOL timeForNewTimeslicePeriod = FALSE;
	
    	if ( elapsedTimeForCurrentIteration > PERIOD_FOR_ONE_DIAGNOSTIC_TEST_ITERATION_MS ) 
    	{
    		timeForNewTimeslicePeriod = TRUE;
    	}
	
    	return timeForNewTimeslicePeriod;
    }

    BOOL BlackfinDiagScheduler::IsTestingCompleteForDiagCycle(BlackfinDiagTest * & rpPbdt) 
    {
    	UINT32 numberToRun = rpPbdt->GetNumberOfTimesToRunPerDiagCycle();
	
    	UINT32 numberRan   = rpPbdt->GetNumberOfTimesRanThisDiagCycle();
	
    	return ( numberRan >= numberToRun );
	
    }

    BOOL BlackfinDiagScheduler::IsTestScheduledToRun(BlackfinDiagTest * & rpPbdt) 
    {
    	BOOL timeToRun = FALSE;

    	DiagTimestampTime startOfIteration = rpPbdt->GetIterationCompletedTimestamp();

    	DiagElapsedTime elapsedTime;
	
    	SystemTiming.ComputeElapsedTimeMS( m_TimestampCurrent, startOfIteration, elapsedTime );

    	DiagElapsedTime iterationPeriod = rpPbdt->GetIterationPeriod();

    	if (elapsedTime >= iterationPeriod) 
        {	
    		timeToRun = TRUE;
    	}

    	return timeToRun;
    }

    void BlackfinDiagScheduler::ResetTestsCompletedForCycle(BlackfinDiagTest * & rpPbdt) 
    {
    	rpPbdt->SetNumberOfTimesRanThisDiagCycle(0);
    }

    void BlackfinDiagScheduler::RunScheduled()
    {	
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

    	switch (m_CurrentSchedulerState) 
        {
    		case INITIAL_INSTANTIATION:
    		case MAX_PERIOD_EXPIRED_ALL_TESTS_COMPLETE:		
    		{
    			m_CurrentSchedulerState = NO_NEW_SCHEDULING_PERIOD;
			
    			SetDiagTestsReadyForNewTestCycle();			
    		}
		
    		break;
		
    		case MAX_PERIOD_EXPIRED_INCOMPLETE_TESTING:
    		{
			
    			UINT32 errorCode = m_AllDiagnosticsNotCompletedErr;
			
    			ConfigureErrorCode( errorCode, m_SchedulerTestType );
				
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


    void BlackfinDiagScheduler::SetAnotherTestCompletedForCycle(BlackfinDiagTest * & rpPbdt) 
    {
    	UINT32 numberOfTimesRan = rpPbdt->GetNumberOfTimesRanThisDiagCycle();
	
    	++numberOfTimesRan;
	
    	rpPbdt->SetNumberOfTimesRanThisDiagCycle( numberOfTimesRan );
    }

    void BlackfinDiagScheduler::SetDiagTestsReadyForNewTestCycle() 
    {	
    	for (std::vector<BlackfinDiagTest *>::iterator it = m_pRunTimeDiagnostics->begin(); it != m_pRunTimeDiagnostics->end(); ++it) 
    	{	
    		BlackfinDiagTest * pPdt = (*it);
		
    		ResetTestsCompletedForCycle( pPdt );
    	}
    }
			
    BOOL BlackfinDiagScheduler::StartEnumeratingTestsForThisIterationPeriod() 
    {	
    	std::vector<BlackfinDiagTest *>::iterator it;
	
    	BOOL testsCanRun = FALSE;
	
    	for (it = m_pRunTimeDiagnostics->begin(); it != m_pRunTimeDiagnostics->end(); ++it) 
    	{	
    		BlackfinDiagTest * pPdt = (*it);
				
    		BOOL testingNotCompleteForThisDiagCycle = !IsTestingCompleteForDiagCycle(pPdt);
		
    		if ( testingNotCompleteForThisDiagCycle ) 
    		{	
    			BOOL testIsScheduledToRun = IsTestScheduledToRun(pPdt);
			
    			if ( testIsScheduledToRun ) 
    			{	
    				testsCanRun = TRUE;
				
    				break;
    			}
						
    		}
    	}
	
    	m_TestEnumeration = it;
	
    	return testsCanRun;
    }				
		
				



    BlackfinDiagScheduler::BlackfinDiagScheduler() 
    {
    }
};





	    		





