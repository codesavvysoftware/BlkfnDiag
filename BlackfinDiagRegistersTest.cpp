#include "BlackfinDiagRegistersTest.hpp"

using namespace DiagnosticCommon;

namespace BlackfinDiagTesting 
{
	
    TestState BlackfinDiagRegistersTest::RunTest( UINT32 & rErrorCode ) 
    {
        ConfigForAnyNewDiagCycle( this );
					
	    return TEST_LOOP_COMPLETE;
	    
    	TestState result = TEST_IN_PROGRESS;
    	
    	if ( !m_SanityTestRan ) 
    	{
        	rErrorCode = (m_pSanityTest )();
        	
        	m_SanityTestRan = TRUE;
    	
        	if ( rErrorCode ) 
        	{
  		
    			rErrorCode &= DIAG_ERROR_MASK;
			
    			rErrorCode = ( GetTestType() << DIAG_ERROR_TYPE_BIT_POS );
			
    			OS_Assert( rErrorCode );
        	}
        	
       	}
       	else
       	{
       	    rErrorCode = (m_pRegisterTest)();
       	    
       	    m_RegisterTestRan = TRUE;
       	    
        	if ( rErrorCode ) 
        	{
  		
    			rErrorCode &= DIAG_ERROR_MASK;
			
    			rErrorCode = ( GetTestType() << DIAG_ERROR_TYPE_BIT_POS );
			
    			OS_Assert( rErrorCode );
        	}
       
        	result = TEST_LOOP_COMPLETE;
       	}
       	
       	return result;
    }
           	    
    void BlackfinDiagRegistersTest::ConfigureForNextTestCycle() 
    {
        if ( !m_pSanityTest || !m_pRegisterTest )
        {
           UINT32 error = m_CorruptedRegisterTestErr;
           
            error &= DIAG_ERROR_MASK;
			
    		error = ( GetTestType() << DIAG_ERROR_TYPE_BIT_POS );
			
    		OS_Assert( error );
        }
        
        m_SanityTestRan   = FALSE;
        
        m_RegisterTestRan = FALSE;
    } 
	
};



	
