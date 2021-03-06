/// #pragma once
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @file BlackfinDiagInstructionsTest.hpp
///
/// Namespace for the base class for all diagnostic tests
///
/// @par Full Description
/// 
/// Diagnostic testing for the Blackfin system timers.  The idea is to test the Apex and Blackfin clocks to make sure
/// they are both keeping accurate time and agree on elapsed time.  The way the test is run is that there is an initial
/// reading of timestamps taken a short while into the diagnostic cycle.  After the intitial timestamp is recorded,
/// another timestamp is read 30 minutes later and compared to what was expected for the timestamp.  There is a 
/// an allowance for the timers to be somewhat off.
///
///
/// @if REVISION_HISTORY_INCLUDED
/// @par Edit History
/// - [0]  thaley1  01-Dec-2015 Initial revision of file.
/// @endif
///
/// @ingroup Diagnostics
///
/// @par Copyright (c) 2015 Rockwell Automation Technologies, Inc.  All rights reserved.
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#if !defined(BLACKFIN_DIAG_TIMER_TEST_HPP)
#define BLACKFIN_DIAG_TIMER_TEST_HPP

// SYSTEM INCLUDES

// C PROJECT INCLUDES
// (none)
 
// C++ PROJECT INCLUDES
#include "DiagnosticTesting.hpp"

// FORWARD REFERENCES
// (none)

namespace BlackfinDiagnosticTesting 
{
	
    // Test specific
    static const UINT32 TIMER_TEST_APEX_TIMER_ERR        =  1;
    static const UINT32 TIMER_TEST_HOST_TIMER_ERR        =  2;
    static const UINT32 TIMER_TIMING_PERIOD_MS           =  30 * 60 * 1000; // 30 minutes in milleseconds;
    static const UINT32 TIMER_MARGIN_OF_ERROR            =  TIMER_TIMING_PERIOD_MS / 20;  //5% of 30 minutes in microseconds 
    static const UINT32 MAX_TIMER_TEST_ELAPSED_TIME_APEX =  TIMER_TIMING_PERIOD_MS + TIMER_MARGIN_OF_ERROR;
    static const UINT32 MAX_TIMER_TEST_ELAPSED_TIME_HOST =  TIMER_TIMING_PERIOD_MS + TIMER_MARGIN_OF_ERROR;
    static const UINT32 MIN_TIMER_TEST_ELAPSED_TIME_APEX =  TIMER_TIMING_PERIOD_MS - TIMER_MARGIN_OF_ERROR;
    static const UINT32 MIN_TIMER_TEST_ELAPSED_TIME_HOST =  TIMER_TIMING_PERIOD_MS - TIMER_MARGIN_OF_ERROR;
    static const UINT64 APEX_TIMER_OVERFLOW_THRESHOLD    =  0x100000000;
    static const UINT64 MICRO_TO_MILLESECOND_CONV_FACTOR =  1000;
    
    class BlackfinDiagTimerTest : public DiagnosticTesting::DiagnosticTest 
    {
        public:

	        //***************************************************************************
            // PUBLIC METHODS
            //***************************************************************************
            
            ///////////////////////////////////////////////////////////////////////////////////////////////////////////
            ///	METHOD NAME: BlackfinDiagTimerTest: BlackfinDiagTimerTest
            ///
            /// @par Full Description
            ///      Construction that is used to construct the BlackfinDiagTimerTest object that is used to test Blackfin
            ///      system timers.  It is derived from the DiagnosticTesting::DiagnosticTest base class.
            ///      
            ///
            /// @param ExecuteTestData              Initial runtime data passed to the base for running this test.
            ///                               
            /// @return                             Blackfin registers test diagnostic instance created.
            ///
            ///////////////////////////////////////////////////////////////////////////////////////////////////////////
        	BlackfinDiagTimerTest( DiagnosticTesting::DiagnosticTest::ExecuteTestData & rTestData ) 
           					   	    :  DiagnosticTesting::DiagnosticTest ( rTestData ), 
        						       m_BeingInstantiated               ( TRUE ),
        						       m_MaxElapsedTimeApex              ( MAX_TIMER_TEST_ELAPSED_TIME_APEX ),
        						       m_MaxElapsedTimeHost              ( MAX_TIMER_TEST_ELAPSED_TIME_HOST ),
        						       m_MinElapsedTimeApex              ( MIN_TIMER_TEST_ELAPSED_TIME_APEX ),
        						       m_MinElapsedTimeHost              ( MIN_TIMER_TEST_ELAPSED_TIME_HOST ),
        						       m_PeriodAfterStartToBeginTiming   ( rTestData.m_IterationPeriod ),
                                       m_ApexTimerValueStart             ( 0 ),
        						       m_HostTimerValueStart             ( 0 ),
        						       m_ElapsedTimeApex                 ( 0 ),
        						       m_ElapsedTimeHost                 ( 0 ),
        						       m_TestExecutions                  ( 0 )
        	{
        	}

            ///////////////////////////////////////////////////////////////////////////////////////////////////////////
            ///	METHOD NAME: BlackfinDiagTimerTest: RunTest
            ///
            /// @par Full Description
            ///      Provides interface specified by the pure virtual method in the base class.  The scheduler calls 
            ///      this method to run iterations of the diagnostic test.
            ///      
            ///
            /// @param                            Reference to a possible error code returned from the scheduler.
            ///                               
            /// @return                           Status of executing a test
            ///
            ///////////////////////////////////////////////////////////////////////////////////////////////////////////
            DiagnosticTesting::DiagnosticTest::TestState RunTest( UINT32 & rErrorCode  );
	
        protected:

        	void ConfigureForNextTestCycle();
	
        private:

        	//
        	// Inhibit copy construction and assignments of this class by putting the declarations in private portion.
        	// If using C++ 11 and later use the delete keyword to do this.
        	//
        	BlackfinDiagTimerTest(const BlackfinDiagTimerTest &);
	
        	const BlackfinDiagTimerTest & operator = (const BlackfinDiagTimerTest & );
		
            BlackfinDiagTimerTest();
                        
             ///////////////////////////////////////////////////////////////////////////////////////////////////////////
            ///	METHOD NAME: BlackfinDiagTimerTest: HostGetTime
            ///
            /// @par Full Description
            ///      Get Apex system time in milleseconds
            ///
            /// @param                            None.
            ///                               
            /// @return                           Apex ystem time in milleseconds
              ///      
            ///////////////////////////////////////////////////////////////////////////////////////////////////////////
            UINT32 ApexGetTime();
            
            ///////////////////////////////////////////////////////////////////////////////////////////////////////////
            ///	METHOD NAME: BlackfinDiagTimerTest: HostGetTime
            ///
            /// @par Full Description
            ///      Get system time for Blackfin Host in microseconds
            ///      
            ///
            /// @param                            None.
            ///                               
            /// @return                           System time in milleseconds
            ///
            ///////////////////////////////////////////////////////////////////////////////////////////////////////////
            UINT32 HostGetTime();
            
            // True when Apex Timer will overflow during the timer test
            BOOL                                  m_ApexTimerWillOverflow;
            
            // True when being instantiated for the current diagnostic cycle.
        	BOOL                                  m_BeingInstantiated;
 
        	// Maximum elapsed time value the Apex can have after 30 minutes.
        	UINT32                                m_MaxElapsedTimeApex;
	
        	// Maximum elapsed time value the Blackfin can have after 30 minutes
        	UINT32                                m_MaxElapsedTimeHost;	// Read the current Apex2 System Time Register value.
    
        	// Minimum elapsed time value the Apex can have after 30 minutes.
        	UINT32                                m_MinElapsedTimeApex;
	
        	// Minimum elapsed time value the Blackfin can have after 30 minutes
        	UINT32                                m_MinElapsedTimeHost;	// Read the current Apex2 System Time Register value.
    
        	// Timestamps for the start of the test
        	UINT32                                m_ApexTimerValueStart;
        
            UINT32                                m_HostTimerValueStart;
            
            // Period after the diagnostic cycle starts to record the initial timestamps.
            UINT32                                m_PeriodAfterStartToBeginTiming;
            
            // Elapsed time for Host when running timer test
            UINT32                                m_ElapsedTimeHost;
            
            // Elapsed time for Apex when running timer test
            UINT32                                m_ElapsedTimeApex;
            
            // Number of times test has run
            UINT32                                m_TestExecutions;
            

    };
};

#endif //#if !defined(BLACKFIN_DIAG_TIMER_TEST_HPP)

