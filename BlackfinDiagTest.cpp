///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @file BlackfinDiagTest.cpp
///
/// Namespace that contains the class definitions, attributes and methods for the base class of Blackfin Diagnostic 
/// testing. 
///
/// @see BlackfinDiagTest.hpp for a detailed description of this class.
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
// C PROJECT INCLUDES
#include "Defs.h"
#include "Os_iotk.h"
#include "Hw.h"


// C++ PROJECT INCLUDES
#include "BlackfinDiagTest.hpp"          // This file contains the Class definition 
                                         // for this class.

// FORWARD REFERENCES




namespace BlackfinDiagTesting 
{
    //***************************************************************************
    // PUBLIC METHODS
    //***************************************************************************
            
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////
    ///	METHOD NAME: BlackfinDiagTest: BlackfinDiagTest
    ///
    ///      Constructor that is used to construct the base class componenent of the inheriting test.
    ///      This the one and only method for instantiating the base class component..
    ///      
    ///
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////
    BlackfinDiagTest::BlackfinDiagTest( BlackfinExecTestData  newTestExecutionData ) 
                                    : m_TestExecutionData ( newTestExecutionData )    
 	{
 	}
	
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////
    ///	METHOD NAME: BlackfinDiagTest: GetCurrentTestState
    ///
    ///       Get the current TestState for the test
    ///      
    ///
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////
    TestState BlackfinDiagTest::GetCurrentTestState() 
    {
        return m_TestExecutionData.m_CurrentTestState;
    }	
		
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////
    ///	METHOD NAME: BlackfinDiagTest: GetIterationCompletedTimestamp
    ///
    ///       Get the timestamp for when the iteration completes
    ///
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////
    UINT64 BlackfinDiagTest::GetIterationCompletedTimestamp() 
    {
	    return m_TestExecutionData.m_IterationCompleteTimestamp;
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////
    ///	METHOD NAME: BlackfinDiagTest: GetIterationPeriod
    ///
    ///       Get the period of time between individual test iterations for the test.  Some done run iteratively
    ///       though.
    ///
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////
    UINT32 BlackfinDiagTest::GetIterationPeriod() 
    {
	    return m_TestExecutionData.m_IterationPeriod;
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////
    ///	METHOD NAME: BlackfinDiagTest: GetMaxTimeBetweenTestCompletions
    ///
    ///       Get the period of time that has been recorded for the maximum time period the test has run in
    ///
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////
    UINT32 BlackfinDiagTest::GetMaxTimeBetweenTestCompletions()
    {
        return m_TestExecutionData.m_MaximumTimeBetweenTestCompletions;
    }
    
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////
    ///	METHOD NAME: BlackfinDiagTest: GetNumberOfTimesToRunPerDiagCycle
    ///
    ///       Get the number of times the test needs to run per diagnostic cycle
    ///
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////
    UINT32 BlackfinDiagTest::GetNumberOfTimesToRunPerDiagCycle() 
    {
	    return m_TestExecutionData.m_NmbrTimesToRunPerDiagCycle;  
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////
    ///	METHOD NAME: BlackfinDiagTest: GetNumberOfTimesRanThisDiagCycle
    ///
    ///       Get the number of times the test has completed during the current diagnostic cycle
    ///
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////
    UINT32 BlackfinDiagTest::GetNumberOfTimesRanThisDiagCycle() 
    {
	    return m_TestExecutionData.m_NmbrTimesRanThisDiagCycle;  
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////
    ///	METHOD NAME: BlackfinDiagTest: GetTestCompletedTimestamp
    ///
    ///       Get the timestamp of when the test completed for the diagnostic cycle
    ///
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////
    UINT64 BlackfinDiagTest::GetTestCompletedTimestamp()
    {
        return m_TestExecutionData.m_TestCompleteTimestamp;
    }
    
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////
    ///	METHOD NAME: BlackfinDiagTest: GetTestStartTime
    ///
    ///       Get the timestamp of when the test started its first iteration in the diagnostic cycle
    ///
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////
    UINT64 BlackfinDiagTest::GetTestStartTime()
    {
        return m_TestExecutionData.m_TestStartTimestamp;
    }
     
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////
    ///	METHOD NAME: BlackfinDiagTest: GetTestType
    ///
    ///       Get DiagnosticTestType that is associated with the test
    ///
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////
    BlackfinDiagTesting::BlackfinDiagTest::DiagnosticTestTypes BlackfinDiagTest::GetTestType() 
    {	
	   return m_TestExecutionData.m_TestType; 
    }
	
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////
    ///	METHOD NAME: BlackfinDiagTest: SetCurrentIterationDuration
    ///
    ///       Set the elapsed time from the start of the test that this iteration has consummend
    ///
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////
    void BlackfinDiagTest::SetCurrentIterationDuration( UINT32 duration )
    {
        m_TestExecutionData.m_CurrentIterationDuration = duration;
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////
    ///	METHOD NAME: BlackfinDiagTest: SetCurrentTestState
    ///
    ///       Set the current TestState for the test
    ///
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////
    void BlackfinDiagTest::SetCurrentTestState( TestState tsCurrent ) 
    {
	    m_TestExecutionData.m_CurrentTestState = tsCurrent;
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////
    ///	METHOD NAME: BlackfinDiagTest: SetIterationCompletedTimestamp
    ///
    ///        Saves the timestamp for when the iteration completes
    ///
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////
    void BlackfinDiagTest::SetIterationCompletedTimestamp(UINT64 timestamp) 
    {
	    m_TestExecutionData.m_IterationCompleteTimestamp = timestamp;
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////
    ///	METHOD NAME: BlackfinDiagTest: SetIterationPeriod
    ///
    ///        Set the period of time between individual test iterations for the test.  Some done run iteratively
    ///        though.
    ///
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////
    void BlackfinDiagTest::SetIterationPeriod(UINT32 period) 
    {
	    m_TestExecutionData.m_IterationPeriod = period;
    }
			
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////
    ///	METHOD NAME: BlackfinDiagTest: SetMaxTimeBetweenTestCompletions
    ///
    ///        Set the period of time that has been recorded for the maximum time period the test has run in
    ///
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////
    void BlackfinDiagTest::SetMaxTimeBetweenTestCompletions( UINT32 period )
    {
        m_TestExecutionData.m_MaximumTimeBetweenTestCompletions = period;
    }
    
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////
    ///	METHOD NAME: BlackfinDiagTest: SetNumberOfTimesRanThisDiagCycle
    ///
    ///        Set the number of times the test has completed during the current diagnostic cycle
    ///
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////
    void BlackfinDiagTest::SetNumberOfTimesRanThisDiagCycle(UINT32 nmberOfTimesRan) 
    {
        m_TestExecutionData.m_NmbrTimesRanThisDiagCycle = nmberOfTimesRan;
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////
    ///	METHOD NAME: BlackfinDiagTest: SetTestCompletedTimestamp
    ///
    ///        Set the timestamp of when the test completed for the diagnostic cycle
    ///
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////
    void BlackfinDiagTest::SetTestCompletedTimestamp( UINT64 period )
    {
        m_TestExecutionData.m_TestCompleteTimestamp = period;
    }
    
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////
    ///	METHOD NAME: BlackfinDiagTest: SetTestStartTime
    ///
    ///        Set the timestamp of when the test started its first iteration in the diagnostic cycle
    ///
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////
    void BlackfinDiagTest::SetTestStartTime( UINT64 time )
    {
        m_TestExecutionData.m_TestStartTimestamp = time;
    }
     
    //***************************************************************************
    // PROTECTED METHODS
    //***************************************************************************

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// METHOD NAME: BlackfinDiagTest: ConfigForAnyNewDiagCycle
    /// 
    ///       This method is called at that start of testing by the scheduler for each test    
    ///
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////
    void BlackfinDiagTest::ConfigForAnyNewDiagCycle( BlackfinDiagTest * btd ) 
    {
		if ( 
		        (TEST_LOOP_COMPLETE == m_TestExecutionData.m_CurrentTestState)
		     || (TEST_IDLE == m_TestExecutionData.m_CurrentTestState)
		   ) 
	    {
			m_TestExecutionData.m_NmbrTimesRanThisDiagCycle = 0;

		    btd->ConfigureForNextTestCycle();
	    }
    }
};


