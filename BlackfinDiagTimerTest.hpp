#pragma once
#include "BlackfinDiagTest.hpp"

namespace BlackfinDiagTesting 
{
    class BlackfinDiagTimerTest : public BlackfinDiagTest 
    {
        public:

        	BlackfinDiagTimerTest( UINT32                 errorApexTimer,
        					       UINT32                 errorHostTimer,
        					       UINT32                 maxElapsedTimeApex,
        	                       UINT32                 maxElapsedTimeHost,
        	                       UINT32                 minElapsedTimeApex,
        	                       UINT32                 minElapsedTimeHost, 
        	                       BlackfinExecTestData & rTestData ) 
           					   	    :  BlackfinDiagTest      ( rTestData ), 
           					   	       m_ApexTimerErr        ( errorApexTimer ),
           					   	       m_HostTimerErr        ( errorHostTimer ),  
        						       m_BeingInstantiated   ( TRUE ),
        						       m_MaxElapsedTimeApex  ( maxElapsedTimeApex ),
        						       m_MaxElapsedTimeHost  ( maxElapsedTimeHost ),
        						       m_MinElapsedTimeApex  ( minElapsedTimeApex ),
        						       m_MinElapsedTimeHost  ( minElapsedTimeHost ),
                                       m_ApexTimerValueStart ( 0 ),
        						       m_HostTimerValueStart ( 0 )
        	{
        	}

        	virtual ~BlackfinDiagTimerTest() 
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
		
        	BOOL                                  m_BeingInstantiated;
 
        	UINT32                                m_ApexTimerErr;
	
        	UINT32                                m_HostTimerErr;
	
        	UINT32                                m_MaxElapsedTimeApex;
	
        	UINT32                                m_MaxElapsedTimeHost;	// Read the current Apex2 System Time Register value.
    
        	UINT32                                m_MinElapsedTimeApex;
	
        	UINT32                                m_MinElapsedTimeHost;	// Read the current Apex2 System Time Register value.
    
        	UINT32                                m_ApexTimerValueStart;
        
            UINT32                                m_HostTimerValueStart;
    };
};
