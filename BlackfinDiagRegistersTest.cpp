#include "BlackfinDiagRegistersTest.h"

BlackfinDiagTest::TestState BlackfinDiagRegistersTest::RunTest( UINT32 & ErrorCode, DiagTime_t SystemTime ) {
    
	TestState tsReturned = TEST_IN_PROGRESS;

	RegisterTestDescriptor * prtd;
	
	BOOL bFoundTestToRun = FindTestToRun( prtd );
	
	if ( bFoundTestToRun ) {
		
		UINT32 FailureInfo = 0;
		
		tsReturned = RunRegisterTests( prtd );
		
		prtd->testsCompleted = TRUE;
		
		if ( TEST_FAILURE == tsReturned ) {
			FailureInfo &= DiagnosticErrorNumberMask;
			
			FailureInfo |= ( TestType << DiagnosticErrorTestTypeBitPos );
			
			firmExcept( FailureInfo );
		}
	}
	else {
		ConfigureForNextTestCycle();
		
		tsReturned = TEST_COMPLETE;
	}
	
	return tsReturned;
}

BOOL BlackfinDiagRegistersTest::FindTestToRun( RegisterTestDescriptor * & RegTestDescriptor ) {
	

BlackfinDiagTest::TestState BlackfinDiagRegistersTest::RunRegisterTests( BlackfinDiagTest::RegisterTestDescriptor rtdTests, UINT32 & FailureInfo )
{
    TestState ts = TEST_LOOP_COMPLETE;
    
    //
    // Test the modify registers next.
    //
    for ( UINT32 ui = 0; ui < rtdTests.NumberOfRegisterTests; ui++ ) {
    	if ( !rtdTests.RegisterTests[ui] ) {

       		ts = TEST_FAILURE;
    		
    		FailureInfo = REGISTER_TEST_POINTER_NULL;
    		
    		break;
    	}
    	
    	FailureInfo = ( rtdTests.RegisterTests[ui])(TestPatternsForRegisterTesting,NumberOfRegisterPatterns);
    	
    	if ( FailureInfo ) {
  		
    		ts = TEST_FAILURE;
    		
    		break;
    	}
    }
    
    return ts;
}

