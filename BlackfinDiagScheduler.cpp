#include "BlackfinDiagScheduler.hpp"
#include <time.h>

namespace DiagnosticScheduling
{	
    template <typename T>
    DiagnosticScheduler<T>::DiagnosticScheduler( std::vector <T *> *           pDiagnostics,
                                                 DiagnosticRunTimeParameters * pRunTimeData )
    		  : m_CurrentSchedulerState          ( INITIAL_INSTANTIATION ),
    		 	m_pRunTimeDiagnostics            ( pDiagnostics ),
    		 	m_pRuntimeData                   ( pRunTimeData ),
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
		
    		UINT32 errorCode = m_pRuntimeData->m_CorruptedVectorErr;
		
    		ConfigureErrorCode( errorCode, m_pRuntimeData->m_SchedulerTestType );
		
    		(*m_pRuntimeData->m_ExcetionError)( errorCode );	
    	}
    }

    template <typename T>
    BOOL DiagnosticScheduler<T>::AreTestIterationsScheduledToRun() 
    {	
    	return StartEnumeratingTestsForThisIterationPeriod();
    }	
    		

	

    template <typename T>
    void DiagnosticScheduler<T>::ConfigureErrorCode( UINT32 & returnedErrorCode, UINT32 testTypeCurrent ) 
    {
		
    	UINT32 ui32        = testTypeCurrent;
    	returnedErrorCode &= DIAG_ERROR_MASK;
    	returnedErrorCode |= (ui32 << DIAG_ERROR_TYPE_BIT_POS); 
    }

    template <typename T>
    void DiagnosticScheduler<T>::DetermineCurrentSchedulerState() 
    {	
    	UINT32 elapsedTimeInTestCycle;
	
    	m_TimestampCurrent = (*m_pRuntimeData->m_SysTimestamp)();
	
    	// No state determination needed will get changed by caller
    	if ( m_CurrentSchedulerState == INITIAL_INSTANTIATION ) 
    	{		
    		//
    		// Sync everything to the same timestamp upon initial instantiation.
    		//
    		m_TimeTestCycleStarted = m_TimestampCurrent;
		
    		m_TimeLastIterationPeriodExpired = m_TimestampCurrent;
		
    		for (std::vector<T *>::iterator it = m_pRunTimeDiagnostics->begin(); it != m_pRunTimeDiagnostics->end(); ++it) {
			
    			T * pPbdt = (*it);
			
    			pPbdt->SetIterationCompletedTimestamp( m_TimestampCurrent );		
            }		
		
            return;
    	}

    	// Compute Elapsed Time in Current Diagnostic Test Period
    	elapsedTimeInTestCycle = (*m_pRuntimeData->m_CalcElapsedTime)( m_TimestampCurrent, m_TimeTestCycleStarted );
	
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
    		UINT32 elapsedTimeForCurrentIteration = (*m_pRuntimeData->m_CalcElapsedTime)( m_TimestampCurrent, m_TimeLastIterationPeriodExpired );
	
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


			
    template <typename T>
    void DiagnosticScheduler<T>::DoMoreDiagnosticTesting() 
    {
	
    	BOOL testIterationCanRun = TRUE;

        while( testIterationCanRun ) 
        {
    	
        	T * pCurrentDiagTest = NULL;
			
        	testIterationCanRun = EnumerateNextScheduledTest( pCurrentDiagTest );
    	
        	if ( testIterationCanRun ) 
        	{
    		    TestState CurrentState = pCurrentDiagTest->GetCurrentTestState();
    		    
    		    if ( CurrentState == TEST_LOOP_COMPLETE )
    		    {
    		        pCurrentDiagTest->SetTestStartTime( m_TimestampCurrent );
    		    }    		    
    		    
    			UINT32	returnedErrorCode;

    			TestState testResult = pCurrentDiagTest->RunTest( returnedErrorCode );

        		pCurrentDiagTest->SetCurrentTestState( testResult );
    
        		pCurrentDiagTest->SetIterationCompletedTimestamp( m_TimestampCurrent );
		
    			switch (testResult)
    			{
    				case TEST_LOOP_COMPLETE: 
    				
    				    if ( m_pRuntimeData->m_MonitorIndividualTotalTestingTime ) 
    				    {
    				        clock_t  clk = pCurrentDiagTest->GetTestCompletedTimestamp();
    				        
                            UINT32 elapsedTime = (*m_pRuntimeData->m_CalcElapsedTime)( m_TimestampCurrent, clk );
                            
                            UINT32 prevElapsedTime = pCurrentDiagTest->GetMaxTimeBetweenTestCompletions();
                            
                            if ( elapsedTime > prevElapsedTime )
                            {
                                pCurrentDiagTest->SetMaxTimeBetweenTestCompletions( elapsedTime );
                            }
                            
                            pCurrentDiagTest->SetTestCompletedTimestamp( m_TimestampCurrent );
                            
    				    }      
    				
    					SetAnotherTestCompletedForCycle(pCurrentDiagTest);
			
    					break;
			
    				case TEST_IN_PROGRESS:
    				
    				    if ( m_pRuntimeData->m_MonitorIndividualTestIterationTimes )
    				    {
    				        clock_t clk = pCurrentDiagTest->GetTestStartTime();
    				        
    				        UINT32 elapsedTime = (*m_pRuntimeData->m_CalcElapsedTime)( m_TimestampCurrent, clk );
    				        
    				        pCurrentDiagTest->SetCurrentIterationDuration( elapsedTime );
    				    }
	
    					break;

    				default:
    				
    					ConfigureErrorCode( returnedErrorCode, pCurrentDiagTest->GetTestType() );
		
    					(*m_pRuntimeData->m_ExcetionError)( returnedErrorCode );
			
    					break;
    			}
    		}
    	}
    }
    
			
    template <typename T>
    BOOL DiagnosticScheduler<T>::DidAllTestsComplete() 
    {
    	BOOL allTestsCompleted = TRUE;
	
    	for (std::vector<T *>::iterator it = m_pRunTimeDiagnostics->begin(); it != m_pRunTimeDiagnostics->end(); ++it) 
    	{		
    		T * pPdt = (*it);
			
            BOOL testDidNotComplete = !IsTestingCompleteForDiagCycle(pPdt);
        
            if ( testDidNotComplete ) 
            {	
            	allTestsCompleted = FALSE;
		
    			break;
            }
    	}
	
    	return allTestsCompleted;
    }
		
    template <typename T>
    BOOL DiagnosticScheduler<T>::EnumerateNextScheduledTest( T * & rpPbdtNextDiag ) 
    {	
    	BOOL success = FALSE;

        while( m_TestEnumeration != m_pRunTimeDiagnostics->end() ) 
        {
        	T * pPdt = (*m_TestEnumeration);
				
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

    template <typename T>
    BOOL DiagnosticScheduler<T>::HasCompleteDiagTestPeriodExpired( UINT32 elapsed_time ) 
    {
    	BOOL timeForNewCycle = FALSE;
	
    	if ( elapsed_time >= m_pRuntimeData->m_PeriodForAllDiagnosticsToCompleteInMS ) 
    	{	
    		timeForNewCycle = TRUE;
    	}

    	return timeForNewCycle;

    }

    template <typename T>
    BOOL DiagnosticScheduler<T>::HasNewTestIterationPeriodStarted( UINT32 elapsedTimeForCurrentIteration ) 
    {	
    	BOOL timeForNewTimeslicePeriod = FALSE;
	
    	if ( elapsedTimeForCurrentIteration > m_pRuntimeData->m_PeriodForOneDiagnosticIteration ) 
    	{
    		timeForNewTimeslicePeriod = TRUE;
    	}
	
    	return timeForNewTimeslicePeriod;
    }

    template <typename T>
    BOOL DiagnosticScheduler<T>::IsTestingCompleteForDiagCycle(T * & rpPbdt) 
    {
    	UINT32 numberToRun = rpPbdt->GetNumberOfTimesToRunPerDiagCycle();
	
    	UINT32 numberRan   = rpPbdt->GetNumberOfTimesRanThisDiagCycle();
	
    	return ( numberRan >= numberToRun );
	
    }

    template <typename T>
    BOOL DiagnosticScheduler<T>::IsTestScheduledToRun(T * & rpPbdt) 
    {
    	BOOL timeToRun = FALSE;

    	clock_t startOfIteration = rpPbdt->GetIterationCompletedTimestamp();

    	UINT32 elapsedTime = (*m_pRuntimeData->m_CalcElapsedTime)( m_TimestampCurrent, startOfIteration );
	
    	UINT32 iterationPeriod = rpPbdt->GetIterationPeriod();

    	if (elapsedTime >= iterationPeriod) 
        {	
    		timeToRun = TRUE;
    	}

    	return timeToRun;
    }

    template <typename T>
    void DiagnosticScheduler<T>::ResetTestsCompletedForCycle(T * & rpPbdt) 
    {
    	rpPbdt->SetNumberOfTimesRanThisDiagCycle(0);
    }

    template <typename T>
    void DiagnosticScheduler<T>::RunScheduled()
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
			
    			UINT32 errorCode = m_pRuntimeData->m_AllDiagnosticsNotCompletedErr;
			
    			ConfigureErrorCode( errorCode, m_pRuntimeData->m_SchedulerTestType );
				
                (*m_pRuntimeData->m_ExcetionError)( errorCode );	
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


    template <typename T>
    void DiagnosticScheduler<T>::SetAnotherTestCompletedForCycle(T * & rpPbdt) 
    {
    	UINT32 numberOfTimesRan = rpPbdt->GetNumberOfTimesRanThisDiagCycle();
	
    	++numberOfTimesRan;
	
    	rpPbdt->SetNumberOfTimesRanThisDiagCycle( numberOfTimesRan );
    }

    template <typename T>
    void DiagnosticScheduler<T>::SetDiagTestsReadyForNewTestCycle() 
    {	
    	for (std::vector<T *>::iterator it = m_pRunTimeDiagnostics->begin(); it != m_pRunTimeDiagnostics->end(); ++it) 
    	{	
    		T * pPdt = (*it);
		
    		ResetTestsCompletedForCycle( pPdt );
    	}
    }
			
    template <typename T>
    BOOL DiagnosticScheduler<T>::StartEnumeratingTestsForThisIterationPeriod() 
    {	
    	std::vector<T *>::iterator it;
	
    	BOOL testsCanRun = FALSE;
	
    	for (it = m_pRunTimeDiagnostics->begin(); it != m_pRunTimeDiagnostics->end(); ++it) 
    	{	
    		T * pPdt = (*it);
				
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
		
    template <typename T>
    DiagnosticScheduler<T>::DiagnosticScheduler() 
    {
    }
};





	    		





