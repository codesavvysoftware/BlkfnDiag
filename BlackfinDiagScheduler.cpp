#include "BlackfinDiagScheduler.hpp"
#include <time.h>

namespace DiagnosticScheduling
{	
    template <typename T>
    DiagnosticScheduler<T>::DiagnosticScheduler( T **                          ppDiagnostics,
                                                 UINT32                        numberOfDiagnosticTests,
                                                 DiagnosticRunTimeParameters  runTimeData )
    		  : m_CurrentSchedulerState          ( INITIAL_INSTANTIATION ),
    		 	m_ppRunTimeDiagnostics           ( ppDiagnostics ),
    		 	m_NumberOfDiagTests              ( numberOfDiagnosticTests ),
    		 	m_RuntimeData                    ( runTimeData ),
    			m_ppTestEnumeration                ( ppDiagnostics + numberOfDiagnosticTests ),
    			m_TimestampCurrent               ( DEFAULT_INITIAL_TIMESTAMP )//,
    {

    	// Number of timeslices between diagnostics completion time checks
    	// Start Fault Injection Point 3
    	// Declaration of DGN_COMPL_CHECK_INTERVAL_TIME_SLICE constant with a smaller value to make
    	// completion time diagnostic injected fault happen faster will be injected here.
    	//	static const DiagSlices_t DGN_COMPL_CHECK_INTERVAL_TIME_SLICE = 15 * DGN_INTERVALS_PER_MINUTE;

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
	
    	BOOL allTestsCompleted = TRUE;
	
    	UINT32 ui = 0;
    		
    	for ( ui = 0; ui < m_NumberOfDiagTests; ui++ )
    	{
            if ( !IsTestingCompleteForDiagCycle(m_ppRunTimeDiagnostics[ ui ]) )
            {	
            	allTestsCompleted = FALSE;
		
    			break;
            }
    	}
	
    	m_TimestampCurrent = (*m_RuntimeData.m_SysTimestamp)();
	
    	// No state determination needed will get changed by caller
    	if ( m_CurrentSchedulerState == INITIAL_INSTANTIATION ) 
    	{		
    		//
    		// Sync everything to the same timestamp upon initial instantiation.
    		//
    		m_TimeTestCycleStarted           = m_TimestampCurrent;
		
    		m_TimeLastIterationPeriodExpired = m_TimestampCurrent;
		
    		UINT32 ui = 0;
    		
    		for ( ui = 0; ui < m_NumberOfDiagTests; ui++ )
    		{
    			m_ppRunTimeDiagnostics[ ui ]->SetIterationCompletedTimestamp( m_TimestampCurrent );		
            }		
		
            return;
    	}

    	// Compute Elapsed Time in Current Diagnostic Test Period
    	elapsedTimeInTestCycle = (*m_RuntimeData.m_CalcElapsedTime)( m_TimestampCurrent, m_TimeTestCycleStarted );
	
    	if ( elapsedTimeInTestCycle >= m_RuntimeData.m_PeriodForAllDiagnosticsToCompleteInMS ) 
    	{	
    		m_TimeTestCycleStarted = m_TimestampCurrent;
		
    		if ( allTestsCompleted ) 
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
    	    if ( NO_TESTS_TO_RUN_ALL_COMPLETED == m_CurrentSchedulerState ) 
    	    {
	    	    return;
    	    	
    	    }
   	    	
    	    UINT32 elapsedTimeForCurrentIteration = (*m_RuntimeData.m_CalcElapsedTime)( m_TimestampCurrent, m_TimeLastIterationPeriodExpired );
	
    	    if ( elapsedTimeForCurrentIteration > m_RuntimeData.m_PeriodForOneDiagnosticIteration ) 
    	    {
    	    	m_TimeLastIterationPeriodExpired = m_TimestampCurrent;
	    	
    	    	if ( allTestsCompleted ) 
    	    	{
	    		    m_CurrentSchedulerState = NO_TESTS_TO_RUN_ALL_COMPLETED;
    	    	}
    	    	else 
    	    	{
   	    			m_CurrentSchedulerState = NO_TEST_ITERATIONS_SCHEDULED;
                    
   	    			UINT32 ui = 0;
   	    			
   	    			for ( ui = 0; ui < m_NumberOfDiagTests; ui++ )
    	            {
			
    		            if ( 
    		                    !IsTestingCompleteForDiagCycle(m_ppRunTimeDiagnostics[ ui ])
    		                 && IsTestScheduledToRun(m_ppRunTimeDiagnostics[ ui ]) 
    		               )
    		            {
		
                            m_CurrentSchedulerState = TEST_ITERATIONS_SCHEDULED;
                            
                            break;
    		            }
    	            }
    	            
    	            m_ppTestEnumeration = m_ppRunTimeDiagnostics + ui;
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
	
    	T * pCurrentDiagTest = NULL;
			
        while( m_ppTestEnumeration != ( m_ppRunTimeDiagnostics + m_NumberOfDiagTests ) ) 
        {
            pCurrentDiagTest = (*m_ppTestEnumeration);
				
    		++m_ppTestEnumeration;
				
    		if ( IsTestingCompleteForDiagCycle(pCurrentDiagTest) ) continue;
		
    		if ( !IsTestScheduledToRun(pCurrentDiagTest) ) continue;
									
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
        			{
        			    if ( m_RuntimeData.m_MonitorIndividualTotalTestingTime ) 
        			    {
        			        UINT64  clk = pCurrentDiagTest->GetTestCompletedTimestamp();
    			        
                            UINT32 elapsedTime = (*m_RuntimeData.m_CalcElapsedTime)( m_TimestampCurrent, clk );
                            
                            UINT32 prevElapsedTime = pCurrentDiagTest->GetMaxTimeBetweenTestCompletions();
                            
                            if ( elapsedTime > prevElapsedTime )
                            {
                                pCurrentDiagTest->SetMaxTimeBetweenTestCompletions( elapsedTime );
                            }
                            
                            pCurrentDiagTest->SetTestCompletedTimestamp( m_TimestampCurrent );
                            
        		        }      
    				
                        UINT32 numberOfTimesRan = pCurrentDiagTest->GetNumberOfTimesRanThisDiagCycle();
	
        	            ++numberOfTimesRan;
	
        	            pCurrentDiagTest->SetNumberOfTimesRanThisDiagCycle( numberOfTimesRan );
        			}
    					
        			break;
			
    			case TEST_IN_PROGRESS:
    				
    			    if ( m_RuntimeData.m_MonitorIndividualTestIterationTimes )
    				{
    				    UINT64 clk = pCurrentDiagTest->GetTestStartTime();
    				        
    				    UINT32 elapsedTime = (*m_RuntimeData.m_CalcElapsedTime)( m_TimestampCurrent, clk );
    				        
    				    pCurrentDiagTest->SetCurrentIterationDuration( elapsedTime );
    				}
	
    				break;

    			default:
    				
    				ConfigureErrorCode( returnedErrorCode, pCurrentDiagTest->GetTestType() );
		
    				(*m_RuntimeData.m_ExceptionError)( returnedErrorCode );
			
    				break;
    		}
    	}
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

    	UINT64 startOfIteration = rpPbdt->GetIterationCompletedTimestamp();

    	UINT32 elapsedTime = (*m_RuntimeData.m_CalcElapsedTime)( m_TimestampCurrent, startOfIteration );
	
    	UINT32 iterationPeriod = rpPbdt->GetIterationPeriod();

    	if (elapsedTime >= iterationPeriod) 
        {	
    		timeToRun = TRUE;
    	}

    	return timeToRun;
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
			
    	        for ( UINT32 ui = 0; ui < m_NumberOfDiagTests; ui++ )
    	        {			
        		    m_ppRunTimeDiagnostics[ ui ]->SetNumberOfTimesRanThisDiagCycle(0);
    	        }
			}
		
    		break;
		
    		case MAX_PERIOD_EXPIRED_INCOMPLETE_TESTING:
    		{
			
    			UINT32 errorCode = m_RuntimeData.m_AllDiagnosticsNotCompletedErr;
			
    			ConfigureErrorCode( errorCode, m_RuntimeData.m_SchedulerTestType );
				
                (*m_RuntimeData.m_ExceptionError)( errorCode );	
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

};





	    		





