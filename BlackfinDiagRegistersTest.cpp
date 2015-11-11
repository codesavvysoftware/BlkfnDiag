#include "BlackfinDiagRegistersTest.hpp"

using namespace DiagnosticCommon;

namespace BlackfinDiagTesting 
{
	
    TestState BlackfinDiagRegistersTest::RunTest( UINT32 & rErrorCode ) 
    {
        ConfigForAnyNewDiagCycle( this );
					
    	TestState result = TEST_IN_PROGRESS;

    	RegisterTestDescriptor * pRtd;
	
    	BOOL foundTestToRun = FindTestToRun( pRtd );
	
    	if ( foundTestToRun ) 
    	{
		
    		BOOL errorDetected = !RunRegisterTests( pRtd, rErrorCode );
		
    		pRtd->m_TestsCompleted = TRUE;
		
    		if ( errorDetected ) 
    		{
    			rErrorCode &= DIAG_ERROR_MASK;
			
    			rErrorCode = ( GetTestType() << DIAG_ERROR_TYPE_BIT_POS );
			
    			OS_Assert( rErrorCode );
    		}
    	}
    	else if (!m_pRegisterTestSuite || !pRtd)  
    	{
    		rErrorCode  = ( GetTestType() << DIAG_ERROR_TYPE_BIT_POS );
		
    		rErrorCode |= m_CorruptedRegisterTestSuiteErr;
			
    		OS_Assert( rErrorCode );
    	}
    	else 
    	{ 
		
    		result = TEST_LOOP_COMPLETE;
    	}
	
    	return result;
    }

    BOOL BlackfinDiagRegistersTest::RunRegisterTests( BlackfinDiagTest::RegisterTestDescriptor * pRegisterTests, 
                                                      UINT32 &                                   rFailureInfo )
    {
       	BOOL	noErrorsDetected = TRUE;
    
        //
        // Test the modify registers next.
        //
        for ( UINT32 ui = 0; ui < pRegisterTests->m_NmbrRegisterTests; ++ui ) {
    	
        	rFailureInfo = ( pRegisterTests->m_pRegisterTests[ui])(m_pTestPatternsForRegisterTesting, m_NmbrOfRegisterPatterns);
    	
        	if ( rFailureInfo ) {
  		
        		noErrorsDetected = FALSE;
    		
        		break;
        	}
        }
    
        return noErrorsDetected;
    }

    BOOL BlackfinDiagRegistersTest::FindTestToRun( BlackfinDiagTest::RegisterTestDescriptor * & rpRegisterTests ) 
    {

    	if ( !m_pRegisterTestSuite ) return FALSE;
    
        BOOL foundTest = FALSE;
    
        RegisterTestDescriptor * pRtd;
    	
    	for ( UINT32 ui = 0; ui < m_NmbrOfRegisterTests; ++ui ) 
    	{
            pRtd = const_cast<RegisterTestDescriptor *>(&m_pRegisterTestSuite[ui]);
		
    		if ( !m_pRegisterTestSuite[ui].m_TestsCompleted ) 
    		{
    			rpRegisterTests = pRtd;
			
    			foundTest = TRUE;
			
    			break;
    		}
    	}
	
    	return foundTest;
    }

    void BlackfinDiagRegistersTest::ConfigureForNextTestCycle() 
    {
    	UINT failureInfo;
	
    	if (!m_pRegisterTestSuite ) 
    	{
    		failureInfo  = ( GetTestType() << DIAG_ERROR_TYPE_BIT_POS );
		
    		failureInfo |= m_CorruptedRegisterTestSuiteErr;
			
    		OS_Assert( failureInfo );
    	}
	
	
    	for ( UINT32 ui = 0; ui < m_NmbrOfRegisterTests; ++ui ) 
    	{
		
    		RegisterTestDescriptor * pRtd = const_cast<RegisterTestDescriptor *>(&m_pRegisterTestSuite[ui]);
		
    		if (!pRtd)  
    		{
    			failureInfo  = ( GetTestType() << DIAG_ERROR_TYPE_BIT_POS );
			
    			failureInfo |= m_CorruptedRegisterTestSuiteErr;
			
    			OS_Assert( failureInfo );
    		}
	    
    		pRtd->m_TestsCompleted = FALSE;
    	}
    }
	
};



	
