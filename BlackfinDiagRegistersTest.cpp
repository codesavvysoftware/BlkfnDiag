#include "BlackfinDiagRegistersTest.h"

BlackfinDiagTest::TestState BlackfinDiagRegistersTest::RunTest( UINT32 & ErrorCode, DiagTime_t SystemTime ) {
    
	TestState tsReturned = TEST_IN_PROGRESS;

	RegisterTestDescriptor * prtd;
	
	BOOL bFoundTestToRun = FindTestToRun( prtd );
	
	UINT32 FailureInfo = 0;
		
	if ( bFoundTestToRun ) {
		
		tsReturned = RunRegisterTests( prtd, FailureInfo );
		
		prtd->testsCompleted = TRUE;
		
		if ( TEST_FAILURE == tsReturned ) {
			FailureInfo &= DiagnosticErrorNumberMask;
			
			FailureInfo = ( GetTestType() << DiagnosticErrorTestTypeBitPos );
			
			firmExcept( FailureInfo );
		}
	}
	else if (!RegisterTestSuite || !prtd)  {
		FailureInfo  = ( GetTestType() << DiagnosticErrorTestTypeBitPos );
		
		FailureInfo |= CorruptedRegisterTestSuite;
			
		firmExcept( FailureInfo );
	}
	else { 
		
		ConfigureForNextTestCycle();
		
		tsReturned = TEST_LOOP_COMPLETE;
	}
	
	return tsReturned;
}

BlackfinDiagTest::TestState BlackfinDiagRegistersTest::RunRegisterTests( BlackfinDiagTest::RegisterTestDescriptor * rtdTests, 
                                                                         UINT32 &                                   FailureInfo )
{
   	TestState ts = TEST_LOOP_COMPLETE;
    
    //
    // Test the modify registers next.
    //
    for ( UINT32 ui = 0; ui < rtdTests->NumberOfRegisterTests; ui++ ) {
    	if ( !rtdTests->RegisterTests[ui] ) {

       		ts = TEST_FAILURE;
    		
    		FailureInfo = RegisterTestPointerIsNull;
    		
    		break;
    	}
    	
    	FailureInfo = ( rtdTests->RegisterTests[ui])(TestPatternsForRegisterTesting,NumberOfRegisterPatterns);
    	
    	if ( FailureInfo ) {
  		
    		ts = TEST_FAILURE;
    		
    		break;
    	}
    }
    
    return ts;
}

BOOL BlackfinDiagRegistersTest::FindTestToRun( BlackfinDiagTest::RegisterTestDescriptor * & rtdTests ) {

	if ( !RegisterTestSuite ) return FALSE;
    
    BOOL bFoundTest = FALSE;
    
    
    RegisterTestDescriptor * prtd;
    	
	for ( UINT32 ui = 0; ui < NumberOfRegisterTests; ui++ ) {
		prtd = const_cast<RegisterTestDescriptor *>(RegisterTestSuite[ui]);
		
		if ( !prtd ) {			
			break;
		}
		
		if ( !RegisterTestSuite[ui]->testsCompleted ) {
			rtdTests = prtd;
			
			bFoundTest = TRUE;
			
			break;
		}
	}
	
	return bFoundTest;
}

void BlackfinDiagRegistersTest::ConfigureForNextTestCycle() {
	for ( UINT32 ui = 0; ui < NumberOfRegisterTests; ui++ ) {
		if ( !RegisterTestSuite[ui] ) {			
			break;
		}
		
		RegisterTestDescriptor * prtd;
    	
	    prtd = const_cast<RegisterTestDescriptor *>(RegisterTestSuite[ui]);
		
	    prtd->testsCompleted = FALSE;
	}
}
	

BOOL BlackfinDiagRegistersTest::IsTestComplete() {
	BOOL bTestsAreComplete = TRUE;
	
	for ( UINT32 ui = 0; ui < NumberOfRegisterTests; ui++ ) {
		if ( !RegisterTestSuite[ui] ) {	
			
					
			break;
		}
		
		RegisterTestDescriptor * prtd;
    	
	    prtd = const_cast<RegisterTestDescriptor *>(RegisterTestSuite[ui]);
		
	    if ( !prtd->testsCompleted ) {
	    	
	    	bTestsAreComplete = FALSE;

	    }
	}
	
	return bTestsAreComplete;
}
	
