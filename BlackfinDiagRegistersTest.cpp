#include "BlackfinDiagRegistersTest.hpp"
#include "Os_iotk.h"
#include "Hw.h"

namespace BlackfinDiagTesting 
{
	
    BlackfinDiagTest::TestState BlackfinDiagRegistersTest::RunTest( UINT32 & rErrorCode ) 
    {
        ConfigForAnyNewDiagCycle( this );
					
       	BlackfinDiagTest::TestState result = BlackfinDiagTest::TEST_IN_PROGRESS;
    	
    	if ( !m_SanityTestRan ) 
    	{
        	rErrorCode = BlackfinDiagRegSanityChk();
        	
        	m_SanityTestRan = TRUE;
    	
        	if ( rErrorCode ) 
        	{
  		
    			rErrorCode &= BlackfinDiagTest::DIAG_ERROR_MASK;
			
    			rErrorCode |= ( GetTestType() << BlackfinDiagTest::DIAG_ERROR_TYPE_BIT_POS );
			
    			OS_Assert( rErrorCode );
        	}
        	
       	}
       	else
       	{
       	    rErrorCode = BlackfinDiagRegChk();
       	    
       	    m_RegisterTestRan = TRUE;
       	    
        	if ( rErrorCode ) 
        	{
  		
    			rErrorCode &= BlackfinDiagTest::DIAG_ERROR_MASK;
			
    			rErrorCode |= ( GetTestType() << BlackfinDiagTest::DIAG_ERROR_TYPE_BIT_POS );
			
    			OS_Assert( rErrorCode );
        	}
       
        	result = BlackfinDiagTest::TEST_LOOP_COMPLETE;
       	}
       	
       	return result;
    }
           	    
    void BlackfinDiagRegistersTest::ConfigureForNextTestCycle() 
    {
        m_SanityTestRan   = FALSE;
        
        m_RegisterTestRan = FALSE;
    } 
	
};



	
