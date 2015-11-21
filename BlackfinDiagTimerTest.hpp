#pragma once
#include "BlackfinDiagTest.hpp"

namespace BlackfinDiagTesting 
{
	
    #define TIMER_TEST_APEX_TIMER_ERR        1
    #define TIMER_TEST_HOST_TIMER_ERR        2
    #define TIMER_MARGIN_OF_ERROR 30 * 60 * 1000000 / 20  //5% of 30 minutes in microseconds 
    #define MAX_TIMER_TEST_ELAPSED_TIME_APEX (30 * 60 * 1000000) + TIMER_MARGIN_OF_ERROR
    #define MAX_TIMER_TEST_ELAPSED_TIME_HOST (30 * 60 * 1000000) + TIMER_MARGIN_OF_ERROR
    #define MIN_TIMER_TEST_ELAPSED_TIME_APEX (30 * 60 * 1000000) - TIMER_MARGIN_OF_ERROR
    #define MIN_TIMER_TEST_ELAPSED_TIME_HOST (30 * 60 * 1000000) - TIMER_MARGIN_OF_ERROR

    class BlackfinDiagTimerTest : public BlackfinDiagTest 
    {
        public:

        	BlackfinDiagTimerTest( BlackfinExecTestData & rTestData ) 
           					   	    :  BlackfinDiagTest      ( rTestData ), 
        						       m_BeingInstantiated   ( TRUE ),
        						       m_MaxElapsedTimeApex  ( MAX_TIMER_TEST_ELAPSED_TIME_APEX ),
        						       m_MaxElapsedTimeHost  ( MAX_TIMER_TEST_ELAPSED_TIME_HOST ),
        						       m_MinElapsedTimeApex  ( MIN_TIMER_TEST_ELAPSED_TIME_APEX ),
        						       m_MinElapsedTimeHost  ( MIN_TIMER_TEST_ELAPSED_TIME_HOST ),
                                       m_ApexTimerValueStart ( 0 ),
        						       m_HostTimerValueStart ( 0 )
        	{
        	}

        	virtual TestState RunTest( UINT32 & rErrorCode  );
	
        protected:

        	virtual void ConfigureForNextTestCycle();
	
        private:

        	//
        	// Inhibit copy construction and assignments of this class by putting the declarations in private portion.
        	// If using C++ 11 and later use the delete keyword to do this.
        	//
        	BlackfinDiagTimerTest(const BlackfinDiagTimerTest &);
	
        	const BlackfinDiagTimerTest & operator = (const BlackfinDiagTimerTest & );
		
            BlackfinDiagTimerTest();
                        
        	BOOL                                  m_BeingInstantiated;
 
        	UINT32                                m_MaxElapsedTimeApex;
	
        	UINT32                                m_MaxElapsedTimeHost;	// Read the current Apex2 System Time Register value.
    
        	UINT32                                m_MinElapsedTimeApex;
	
        	UINT32                                m_MinElapsedTimeHost;	// Read the current Apex2 System Time Register value.
    
        	UINT32                                m_ApexTimerValueStart;
        
            UINT32                                m_HostTimerValueStart;
    };
};
