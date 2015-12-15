///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @file BlackfinDiagTimerTest.cpp
///
/// Namespace that contains the class definitions, attributes and methods for the BlackfinDiagTimerTest class. 
///
/// @see BlackfinDiagTimerTest.hpp for a detailed description of this class.
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
// SYSTEM INCLUDES
// (none)

// C PROJECT INCLUDES
#include "Defs.h"
#include "Os_iotk.h"             // This file depends on Defs.h.  It should include that file
#include "Hw.h"                  // Ditto 
#include "Apex.h"

// C++ PROJECT INCLUDES
#include "BlackfinDiagTimerTest.hpp"


// FORWARD REFERENCES
// (none)
//

extern "C" ULINT Apex_GetTime();

namespace BlackfinDiagnosticTesting 
{
	UINT32 biasForTest = 0xff700000;
	
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////
    ///	METHOD NAME: BlackfinDiagTimerTest: RunTest
    ///
    ///      Provides interface specified by the pure virtual method in the base class.  The scheduler calls 
    ///      this method to run iterations of the diagnostic test..
    ///
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////
    DiagnosticTesting::DiagnosticTest::TestState BlackfinDiagTimerTest::RunTest( UINT32 & errorCode ) 
    {
	
    	ConfigForAnyNewDiagCycle( this );
					
        // If we have not completed this diagnostic yet, get the start times
        // and return DGN_TEST_IN_PROG.
        if ( m_BeingInstantiated ) 
        {

            m_BeingInstantiated = FALSE;
        
            m_ElapsedTimeHost = 0;
            
            m_ElapsedTimeApex = 0;
            
            m_HostTimerValueStart = HostGetTime();
        
            m_ApexTimerValueStart = ApexGetTime();
            
            m_ApexValStart = Apex_GetTime();
            
            m_ApexValStop  = 0;
            
            SetIterationPeriod( TIMER_TIMING_PERIOD_MS );  //
        
            return ( DiagnosticTesting::DiagnosticTest::TEST_IN_PROGRESS );
        }
        
        ++m_TestExecutions;
        
        DiagnosticTesting::DiagnosticTest::DiagnosticTestTypes ts = GetTestType();

        // Read the current Apex2 System Time Register value.
        UINT32 apexTimerValueStop = ApexGetTime();

        m_ApexValStop  = Apex_GetTime();
            
        UINT32 hostTimerValueStop = HostGetTime();
        
        // Calculate the time elapsed according to the previous and current host timer values read.
        m_ElapsedTimeHost = hostTimerValueStop - m_HostTimerValueStart;
           
        // Check if either of the elapsed times are bad.
        if (
                ( m_ElapsedTimeHost < m_MinElapsedTimeHost ) 
             || ( m_ElapsedTimeHost > m_MaxElapsedTimeHost )
           ) 
        {
       	
           	UINT32 errorCode = (ts << DiagnosticTesting::DiagnosticTest::DIAG_ERROR_TYPE_BIT_POS) | TIMER_TEST_HOST_TIMER_ERR;

        	OS_Assert( errorCode );
        }
       
        m_ElapsedTimeApex = apexTimerValueStop - m_ApexTimerValueStart;

        if (
                ( m_ElapsedTimeApex < m_MinElapsedTimeApex ) 
             || ( m_ElapsedTimeApex > m_MaxElapsedTimeApex ) 
          )
        {
           	UINT32 errorCode = (ts << DIAG_ERROR_TYPE_BIT_POS) | TIMER_TEST_APEX_TIMER_ERR;

        	OS_Assert( errorCode );
        }

        // Set the start values equal to the stop values to prepare for the next time this diagnostic is run.
        m_ApexTimerValueStart = apexTimerValueStop;
        m_HostTimerValueStart = hostTimerValueStop;

        SetIterationPeriod( m_PeriodAfterStartToBeginTiming );  
        
        // If we get this far, the diagnostic has completed so return DGN_TEST_LOOP_COMPLETE.
        return ( DiagnosticTesting::DiagnosticTest::TEST_LOOP_COMPLETE );
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////
    ///	METHOD NAME: BlackfinDiagTimerTest: ConfigureForNextTestCycle
    ///
    ///      Provides interface specified by the pure virtual method in the base class. This method is called 
    ///      at that start of testing for the test during a new diagnostics cycle. The data that needs to be 
    ///      initialized for an individual test is initialized.      
    ///
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////
    void BlackfinDiagTimerTest::ConfigureForNextTestCycle() 
    {
        m_BeingInstantiated = TRUE;
    }
    
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////
    ///	METHOD NAME: BlackfinDiagTimerTest: HostGetTime
    ///
    /// @par Full Description
    ///      Get system time for Blackfin Host in milleseconds
    ///      
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////
    UINT32 BlackfinDiagTimerTest::HostGetTime()
    {
        UINT64 ullCurrentDspCycles;
            
        _GET_CYCLE_COUNT( ullCurrentDspCycles );
             
        return ( CCLK_TO_US( ullCurrentDspCycles ) / MICRO_TO_MILLESECOND_CONV_FACTOR );
    }
    
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////
    ///	METHOD NAME: BlackfinDiagTimerTest: HostGetTime
    ///
    ///      Get Apex system time in milleseconds
    ///      
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////
    UINT32 BlackfinDiagTimerTest::ApexGetTime()
    {     
        return ( Apex_GetTime() / MICRO_TO_MILLESECOND_CONV_FACTOR );
    }
    
};
