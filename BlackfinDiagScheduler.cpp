///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @file DiagnosticScheduler.cpp
///
/// Namespace for scheduling and monitoring diagnostic tests. 
///
/// @see DiagnosticScheduler.hpp for a detailed description of this class.
///
/// @if REVISION_HISTORY_INCLUDED
/// @par Edit History
/// - [0]  thaley1  01-Dec-2015 Initial revision of file.
/// @endif
///
/// @ingroup Diagnostics
///
/// @par Copyright (c) 2013 Rockwell Automation Technologies, Inc.  All rights reserved.
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @defgroup MySubsystem Blackfin Diagnostics
///
/// 
///
/// @par Full Description
/// 
/// Blackfin diagnostics use the Apex diagnostic subsystem as the model architecture.  The architecture incorporates 
/// the concepts of individual diagnostic tests that run on a periodic basis, a runtime environment that is configured
/// per the individual product reaquirements (including processor type), the scheduling of diagnostic tests in a timely
/// manner, the monitoring of test progress, the detection of test failures, and the reporting on failed tests.  The 
/// data structures for Apex and Blackfin are similar but not identical.  The Blackfin diagnostics leverage the common
/// functionality between individual tests by implementing a base class for a diagnostic test type and deriving 
/// individual tests from the base class.  Also an effort was made to have a separate portion of the code that defines 
/// a runtime environment where all changes to the runtime configuration are accomplished in one place.  Finally it was
/// recogninzed that scheduling requirements are should the same for both Blackfin and Apex at a high level.  Thus a 
/// template class was designed to implement the scheduling requirements.  Blackfin diagnostic scheduling is 
/// implemented somewhat differently than Apex diagnostic scheduling.  Apex relies on a concept of a fixed period 
/// timeslice to do timing.  The fixed period is measured in clock cycles which is based on processor speed.  Processor 
/// speed varies from system to system.  For Blackfin timing uses actual time values to do scheduling.  It is not 
/// precision timing as in real-time but it is close enough to satisfy the requirements.  Seconds, milleseconds, and 
/// microseconds are invariant quantities from system to system and I think it makes for more readable, understandable
/// code.
/// 
/////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
#include <time.h>

// C PROJECT INCLUDES

// C++ PROJECT INCLUDES
#include "BlackfinDiagScheduler.hpp"     // This file contains the Class definition 
                                         // for this class.

// FORWARD REFERENCES





namespace DiagnosticScheduling
{	
    //***************************************************************************
    // PUBLIC METHODS
    //***************************************************************************

    /////////////////////////////////////////////////////////////////////////////
    //	METHOD NAME: DiagnosticScheduler<T>::DiagnosticScheduler
    //
    /// Constructor for instantiating the scheduler object 
    ///
    /////////////////////////////////////////////////////////////////////////////
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


    ///////////////////////////////////////////////////////////////////////////
    ///	METHOD NAME: DiagnositcScheduler: RunScheduled
    ///
    /// For determining when and what diagnostics are run.
    ///
    /////////////////////////////////////////////////////////////////////////// 
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

    //***************************************************************************
    // PRIVATE METHODS
    //***************************************************************************

    /////////////////////////////////////////////////////////////////////////////
    ///	METHOD NAME: DiagnositcScheduler: ConfigureErrorCode
    ///
    /// Configure error code to report.
    ///      
    /////////////////////////////////////////////////////////////////////////////
    template <typename T>
    void DiagnosticScheduler<T>::ConfigureErrorCode( UINT32 & returnedErrorCode, UINT32 testTypeCurrent ) 
    {
		
    	UINT32 ui32        = testTypeCurrent;
    	returnedErrorCode &= T::DIAG_ERROR_MASK;
    	returnedErrorCode |= (ui32 << T::DIAG_ERROR_TYPE_BIT_POS); 
    }

    //////////////////////////////////////////////////////////////////////////////
    ///	METHOD NAME: DiagnositcScheduler: DetermineCurrentSchedulerState
    ///
    /// Determine the current state the scheduler is in.
    ///      
    /////////////////////////////////////////////////////////////////////////////
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

    /////////////////////////////////////////////////////////////////////////////
    ///	METHOD NAME: DiagnositcScheduler: DoMoreDiagnosticTesting
    ///
    /// Schedulre more diagnostic testing when there is more testing to do.
    ///      
    /////////////////////////////////////////////////////////////////////////////
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
									
    		T::TestState CurrentState = pCurrentDiagTest->GetCurrentTestState();
    		    
    		if ( CurrentState == T::TEST_LOOP_COMPLETE )
    		{
    		    pCurrentDiagTest->SetTestStartTime( m_TimestampCurrent );
    		}    		    
    		    
    		UINT32	returnedErrorCode;

    		T::TestState testResult = pCurrentDiagTest->RunTest( returnedErrorCode );

        	pCurrentDiagTest->SetCurrentTestState( testResult );
    
        	pCurrentDiagTest->SetIterationCompletedTimestamp( m_TimestampCurrent );
		
    		switch (testResult)
    		{
    		    case T::TEST_LOOP_COMPLETE: 
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
			
    			case T::TEST_IN_PROGRESS:
    				
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
			
    /////////////////////////////////////////////////////////////////////////////
    ///	METHOD NAME: DiagnositcScheduler: IsTestingCompleteForDiagCycle
    ///
    /// Returns true when a testing for test indicated is complete for current
    /// diagnostic cycle.
    ///      
    /////////////////////////////////////////////////////////////////////////////
    template <typename T>
    BOOL DiagnosticScheduler<T>::IsTestingCompleteForDiagCycle(T * & rpPbdt) 
    {
    	UINT32 numberToRun = rpPbdt->GetNumberOfTimesToRunPerDiagCycle();
	
    	UINT32 numberRan   = rpPbdt->GetNumberOfTimesRanThisDiagCycle();
	
    	return ( numberRan >= numberToRun );
	
    }

    /////////////////////////////////////////////////////////////////////////////
    ///	METHOD NAME: DiagnositcScheduler: IsTestScheduledToRun
    ///
    /// Returns true when a test is scheduled to run and returns a pointer to the 
    /// test to be run.
    ///      
    /////////////////////////////////////////////////////////////////////////////
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
};





	    		





