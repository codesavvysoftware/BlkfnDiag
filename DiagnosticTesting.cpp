///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @file DiagnosticTest.cpp
///
/// Namespace that contains the class definitions, attributes and methods for the base class of Blackfin Diagnostic 
/// testing. 
///
/// @see DiagnosticTesting.hpp for a detailed description of this class.
///
/// @if REVISION_HISTORY_INCLUDED
/// @par Edit History
/// - [0]  thaley1  01-Dec-2015 Initial revision of file.
/// @endif
///
/// @ingroup Diagnostics
///
/// @par Copyright (c) 2015 Rockwell Automation Technologies, Inc.  All rights reserved.
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
/// speed varies from system to system.  For Blackfin timing, time values to do scheduling are in engineering units.
/// It is not precision timing as in real-time but it is close enough to satisfy the requirements.  
/// 
/////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
// C PROJECT INCLUDES
#include "Defs.h"
#include "Os_iotk.h"
#include "Hw.h"


// C++ PROJECT INCLUDES
#include "DiagnosticTesting.hpp"          // This file contains the Class definition 
                                         // for this class.

// FORWARD REFERENCES




namespace DiagnosticTesting 
{
    //***************************************************************************
    // PUBLIC METHODS
    //***************************************************************************
            
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////
    ///	METHOD NAME: DiagnosticTest: DiagnosticTest
    ///
    ///      Constructor that is used to construct the base class componenent of the inheriting test.
    ///      This the one and only method for instantiating the base class component..
    ///      
    ///
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////
    DiagnosticTest::DiagnosticTest( ExecuteTestData  newTestExecutionData ) 
                                    : m_TestExecutionData ( newTestExecutionData )    
 	{
 	}
	
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////
    ///	METHOD NAME: DiagnosticTest: GetCurrentTestState
    ///
    ///       Get the current TestState for the test
    ///      
    ///
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////
    DiagnosticTest::TestState DiagnosticTest::GetCurrentTestState() 
    {
        return m_TestExecutionData.m_CurrentTestState;
    }	
		
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////
    ///	METHOD NAME: DiagnosticTest: GetIterationCompletedTimestamp
    ///
    ///       Get the timestamp for when the iteration completes
    ///
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////
    UINT64 DiagnosticTest::GetIterationCompletedTimestamp() 
    {
	    return m_TestExecutionData.m_IterationCompleteTimestamp;
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////
    ///	METHOD NAME: DiagnosticTest: GetIterationPeriod
    ///
    ///       Get the period of time between individual test iterations for the test.  Some done run iteratively
    ///       though.
    ///
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////
    UINT32 DiagnosticTest::GetIterationPeriod() 
    {
	    return m_TestExecutionData.m_IterationPeriod;
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////
    ///	METHOD NAME: DiagnosticTest: GetMaxTimeBetweenTestCompletions
    ///
    ///       Get the period of time that has been recorded for the maximum time period the test has run in
    ///
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////
    UINT32 DiagnosticTest::GetMaxTimeBetweenTestCompletions()
    {
        return m_TestExecutionData.m_MaximumTimeBetweenTestCompletions;
    }
    
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////
    ///	METHOD NAME: DiagnosticTest: GetNumberOfTimesToRunPerDiagCycle
    ///
    ///       Get the number of times the test needs to run per diagnostic cycle
    ///
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////
    UINT32 DiagnosticTest::GetNumberOfTimesToRunPerDiagCycle() 
    {
	    return m_TestExecutionData.m_NmbrTimesToRunPerDiagCycle;  
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////
    ///	METHOD NAME: DiagnosticTest: GetNumberOfTimesRanThisDiagCycle
    ///
    ///       Get the number of times the test has completed during the current diagnostic cycle
    ///
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////
    UINT32 DiagnosticTest::GetNumberOfTimesRanThisDiagCycle() 
    {
	    return m_TestExecutionData.m_NmbrTimesRanThisDiagCycle;  
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////
    ///	METHOD NAME: DiagnosticTest: GetTestCompletedTimestamp
    ///
    ///       Get the timestamp of when the test completed for the diagnostic cycle
    ///
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////
    UINT64 DiagnosticTest::GetTestCompletedTimestamp()
    {
        return m_TestExecutionData.m_TestCompleteTimestamp;
    }
    
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////
    ///	METHOD NAME: DiagnosticTest: GetTestStartTime
    ///
    ///       Get the timestamp of when the test started its first iteration in the diagnostic cycle
    ///
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////
    UINT64 DiagnosticTest::GetTestStartTime()
    {
        return m_TestExecutionData.m_TestStartTimestamp;
    }
     
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////
    ///	METHOD NAME: DiagnosticTest: GetTestType
    ///
    ///       Get DiagnosticTestType that is associated with the test
    ///
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////
    DiagnosticTesting::DiagnosticTest::DiagnosticTestTypes DiagnosticTest::GetTestType() 
    {	
	   return m_TestExecutionData.m_TestType; 
    }
	
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////
    ///	METHOD NAME: DiagnosticTest: SetCurrentIterationDuration
    ///
    ///       Set the elapsed time from the start of the test that this iteration has consummend
    ///
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////
    void DiagnosticTest::SetCurrentIterationDuration( UINT32 duration )
    {
        m_TestExecutionData.m_CurrentIterationDuration = duration;
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////
    ///	METHOD NAME: DiagnosticTest: SetCurrentTestState
    ///
    ///       Set the current TestState for the test
    ///
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////
    void DiagnosticTest::SetCurrentTestState( TestState tsCurrent ) 
    {
	    m_TestExecutionData.m_CurrentTestState = tsCurrent;
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////
    ///	METHOD NAME: DiagnosticTest: SetIterationCompletedTimestamp
    ///
    ///        Saves the timestamp for when the iteration completes
    ///
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////
    void DiagnosticTest::SetIterationCompletedTimestamp(UINT64 timestamp) 
    {
	    m_TestExecutionData.m_IterationCompleteTimestamp = timestamp;
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////
    ///	METHOD NAME: DiagnosticTest: SetIterationPeriod
    ///
    ///        Set the period of time between individual test iterations for the test.  Some done run iteratively
    ///        though.
    ///
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////
    void DiagnosticTest::SetIterationPeriod(UINT32 period) 
    {
	    m_TestExecutionData.m_IterationPeriod = period;
    }
			
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////
    ///	METHOD NAME: DiagnosticTest: SetMaxTimeBetweenTestCompletions
    ///
    ///        Set the period of time that has been recorded for the maximum time period the test has run in
    ///
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////
    void DiagnosticTest::SetMaxTimeBetweenTestCompletions( UINT32 period )
    {
        m_TestExecutionData.m_MaximumTimeBetweenTestCompletions = period;
    }
    
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////
    ///	METHOD NAME: DiagnosticTest: SetNumberOfTimesRanThisDiagCycle
    ///
    ///        Set the number of times the test has completed during the current diagnostic cycle
    ///
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////
    void DiagnosticTest::SetNumberOfTimesRanThisDiagCycle(UINT32 nmberOfTimesRan) 
    {
        m_TestExecutionData.m_NmbrTimesRanThisDiagCycle = nmberOfTimesRan;
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////
    ///	METHOD NAME: DiagnosticTest: SetTestCompletedTimestamp
    ///
    ///        Set the timestamp of when the test completed for the diagnostic cycle
    ///
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////
    void DiagnosticTest::SetTestCompletedTimestamp( UINT64 period )
    {
        m_TestExecutionData.m_TestCompleteTimestamp = period;
    }
    
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////
    ///	METHOD NAME: DiagnosticTest: SetTestStartTime
    ///
    ///        Set the timestamp of when the test started its first iteration in the diagnostic cycle
    ///
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////
    void DiagnosticTest::SetTestStartTime( UINT64 time )
    {
        m_TestExecutionData.m_TestStartTimestamp = time;
    }
     
    //***************************************************************************
    // PROTECTED METHODS
    //***************************************************************************

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// METHOD NAME: DiagnosticTest: ConfigForAnyNewDiagCycle
    /// 
    ///       This method is called at that start of testing by the scheduler for each test    
    ///
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////
    void DiagnosticTest::ConfigForAnyNewDiagCycle( DiagnosticTest * btd ) 
    {
		if ( 
		        (DiagnosticTest::TEST_LOOP_COMPLETE == m_TestExecutionData.m_CurrentTestState)
		     || (DiagnosticTest::TEST_IDLE == m_TestExecutionData.m_CurrentTestState)
		   ) 
	    {
			m_TestExecutionData.m_NmbrTimesRanThisDiagCycle = 0;

		    btd->ConfigureForNextTestCycle();
	    }
    }
};


