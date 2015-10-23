#include "BlackfinDiagRegistersTest.h"

BlackfinDiagTest::TestState BlackfinDiagRegistersTest::RunTest( UINT32 & ErrorCode, DiagTime_t SystemTime ) {
    
	ConfigForAnyNewDiagCycle( this );
					
	TestState tsReturned = TEST_IN_PROGRESS;

	RegisterTestDescriptor * prtd;
	
	BOOL bFoundTestToRun = FindTestToRun( prtd );
	
	UINT32 FailureInfo = 0;
		
	if ( bFoundTestToRun ) {
		
		BOOL bErrorDetected = !RunRegisterTests( prtd, FailureInfo );
		
		prtd->testsCompleted = TRUE;
		
		if ( bErrorDetected ) {
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
		
		SetTestsCompletedForCycle();

		tsReturned = TEST_LOOP_COMPLETE;
	}
	
	return tsReturned;
}

BOOL BlackfinDiagRegistersTest::RunRegisterTests( BlackfinDiagTest::RegisterTestDescriptor * rtdTests, 
                                                  UINT32 &                                   FailureInfo )
{
   	BOOL	bNoErrorsDetected = TRUE;
    
    //
    // Test the modify registers next.
    //
    for ( UINT32 ui = 0; ui < rtdTests->NumberOfRegisterTests; ui++ ) {
    	
    	FailureInfo = ( rtdTests->RegisterTests[ui])(TestPatternsForRegisterTesting,NumberOfRegisterPatterns);
    	
    	if ( FailureInfo ) {
  		
    		bNoErrorsDetected = FALSE;
    		
    		break;
    	}
    }
    
    return bNoErrorsDetected;
}

BOOL BlackfinDiagRegistersTest::FindTestToRun( BlackfinDiagTest::RegisterTestDescriptor * & rtdTests ) {

	if ( !RegisterTestSuite ) return FALSE;
    
    BOOL bFoundTest = FALSE;
    
    
    RegisterTestDescriptor * prtd;
    	
	for ( UINT32 ui = 0; ui < NumberOfRegisterTests; ui++ ) {
		prtd = const_cast<RegisterTestDescriptor *>(&RegisterTestSuite[ui]);
		
		if ( !RegisterTestSuite[ui].testsCompleted ) {
			rtdTests = prtd;
			
			bFoundTest = TRUE;
			
			break;
		}
	}
	
	return bFoundTest;
}

void BlackfinDiagRegistersTest::ConfigureForNextTestCycle() {
	UINT FailureInfo;
	
	if (!RegisterTestSuite ) {
		FailureInfo  = ( GetTestType() << DiagnosticErrorTestTypeBitPos );
		
		FailureInfo |= CorruptedRegisterTestSuite;
			
		firmExcept( FailureInfo );
	}
	
	
	for ( UINT32 ui = 0; ui < NumberOfRegisterTests; ui++ ) {
		
		RegisterTestDescriptor * prtd;
    	
	    prtd = const_cast<RegisterTestDescriptor *>(&RegisterTestSuite[ui]);
		
		if (!prtd)  {
			FailureInfo  = ( GetTestType() << DiagnosticErrorTestTypeBitPos );
			
			FailureInfo |= CorruptedRegisterTestSuite;
			
			firmExcept( FailureInfo );
		}
	    
		prtd->testsCompleted = FALSE;
	}
}
	

BOOL BlackfinDiagRegistersTest::IsTestComplete() {
	BOOL bTestsAreComplete = TRUE;
	
	for( UINT32 ui = 0; ui < NumberOfRegisterTests; ui++ ) {
	
		RegisterTestDescriptor * prtd;
    	
	    prtd = const_cast<RegisterTestDescriptor *>(&RegisterTestSuite[ui]);
		
	    if ( !prtd->testsCompleted ) {
	    	
	    	bTestsAreComplete = FALSE;

	    }
	}
	
	return bTestsAreComplete;
}
	
