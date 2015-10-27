#include "BlackfinDiagRegistersTest.h"

using namespace DiagnosticCommon;

namespace BlackfinDiagTests {
	
BlackfinDiagTest::TestState BlackfinDiagRegistersTest::RunTest( UINT32 & errorCode, DiagTime_t systemTime ) {
    
	ConfigForAnyNewDiagCycle( this );
					
	TestState result = TEST_IN_PROGRESS;

	RegisterTestDescriptor * prtd;
	
	BOOL foundTestToRun = FindTestToRun( prtd );
	
	UINT32 failureInfo = 0;
		
	if ( foundTestToRun ) {
		
		BOOL errorDetected = !RunRegisterTests( prtd, failureInfo );
		
		prtd->testsCompleted = TRUE;
		
		if ( errorDetected ) {
			failureInfo &= DiagnosticErrorNumberMask;
			
			failureInfo = ( GetTestType() << DiagnosticErrorTestTypeBitPos );
			
			firmExcept( failureInfo );
		}
	}
	else if (!registerTestSuite_ || !prtd)  {
		failureInfo  = ( GetTestType() << DiagnosticErrorTestTypeBitPos );
		
		failureInfo |= corruptedRegisterTestSuite_;
			
		firmExcept( failureInfo );
	}
	else { 
		
		result = TEST_LOOP_COMPLETE;
	}
	
	return result;
}

BOOL BlackfinDiagRegistersTest::RunRegisterTests( BlackfinDiagTest::RegisterTestDescriptor * registerTests, 
                                                  UINT32 &                                   failureInfo )
{
   	BOOL	noErrorsDetected = TRUE;
    
    //
    // Test the modify registers next.
    //
    for ( UINT32 ui = 0; ui < registerTests->nmbrRegisterTests; ++ui ) {
    	
    	failureInfo = ( registerTests->registerTests[ui])(testPatternsForRegisterTesting_,numberOfRegisterPatterns_);
    	
    	if ( failureInfo ) {
  		
    		noErrorsDetected = FALSE;
    		
    		break;
    	}
    }
    
    return noErrorsDetected;
}

BOOL BlackfinDiagRegistersTest::FindTestToRun( BlackfinDiagTest::RegisterTestDescriptor * & registerTests ) {

	if ( !registerTestSuite_ ) return FALSE;
    
    BOOL foundTest = FALSE;
    
    
    RegisterTestDescriptor * prtd;
    	
	for ( UINT32 ui = 0; ui < numberOfRegisterTests_; ++ui ) {
		prtd = const_cast<RegisterTestDescriptor *>(&registerTestSuite_[ui]);
		
		if ( !registerTestSuite_[ui].testsCompleted ) {
			registerTests = prtd;
			
			foundTest = TRUE;
			
			break;
		}
	}
	
	return foundTest;
}

void BlackfinDiagRegistersTest::ConfigureForNextTestCycle() {
	UINT failureInfo;
	
	if (!registerTestSuite_ ) {
		failureInfo  = ( GetTestType() << DiagnosticErrorTestTypeBitPos );
		
		failureInfo |= corruptedRegisterTestSuite_;
			
		firmExcept( failureInfo );
	}
	
	
	for ( UINT32 ui = 0; ui < numberOfRegisterTests_; ++ui ) {
		
		RegisterTestDescriptor * prtd;
    	
	    prtd = const_cast<RegisterTestDescriptor *>(&registerTestSuite_[ui]);
		
		if (!prtd)  {
			failureInfo  = ( GetTestType() << DiagnosticErrorTestTypeBitPos );
			
			failureInfo |= corruptedRegisterTestSuite_;
			
			firmExcept( failureInfo );
		}
	    
		prtd->testsCompleted = FALSE;
	}
}
	
};



	
