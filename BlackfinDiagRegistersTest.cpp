#include "BlackfinDiagRegistersTest.h"

BlackfinDiagTest::TestState BlackfinDiagRegistersTest::RunTest( UINT32 & ErrorCode, DiagTime_t SystemTime ) {

	if ( !RegisterTestSuite	) return TEST_FAILURE;
	
	TestState tsReturned = TEST_IN_PROGRESS;

    //
    // Test the data registers next.
    //
    tsReturned = RunRegisterTests(  RegisterTestSuite->SanityRegTestDiag );
    
    if ( TEST_FAILURE == tsReturned ) return tsReturned;
   
    //
    // Test the data registers next.
    //
    tsReturned = RunRegisterTests( RegisterTestSuite->DataRegTestDiag );
    
    if ( TEST_FAILURE == tsReturned ) return tsReturned;

    //
    // Test the pointer registers next.
    //
    tsReturned = RunRegisterTests( RegisterTestSuite->PointerRegTestDiag );
    
    if ( TEST_FAILURE == tsReturned ) return tsReturned;
    
    //
    // Test the accumulator registers next.
    //
    tsReturned = RunRegisterTests( RegisterTestSuite->AccumRegTestDiag );
    
    //
    // Test the modify registers next.
    //
    tsReturned = RunRegisterTests( RegisterTestSuite->ModifyRegTestDiag );
    
    if ( TEST_FAILURE == tsReturned ) return tsReturned;

    //
    // Test the length registers next.
    //
    tsReturned = RunRegisterTests( RegisterTestSuite->LengthRegTestDiag );
    
    if ( TEST_FAILURE == tsReturned ) return tsReturned;

    //
    // Test the index registers next.
    //
    tsReturned = RunRegisterTests( RegisterTestSuite->IndexRegTestDiag );
    
    if ( TEST_FAILURE == tsReturned ) return tsReturned;

    //
    // Test the base registers next.
    //
    return RunRegisterTests( RegisterTestSuite->BaseRegTestDiag );
    
}

BlackfinDiagTest::TestState BlackfinDiagRegistersTest::RunRegisterTests( BlackfinDiagTest::RegisterTestDescriptor rtdTests)
{
    TestState ts = TEST_LOOP_COMPLETE;
    
    //
    // Test the modify registers next.
    //
    for ( UINT32 ui = 0; ui < rtdTests.NumberOfRegisterTests; ui++ ) {
    	if ( !rtdTests.RegisterTests[ui] ) {
    		ts = TEST_FAILURE;
    		
    		break;
    	}
    	
    	UINT32 CurrentResult = ( rtdTests.RegisterTests[ui])(TestPatternsForRegisterTesting,NumberOfRegisterPatterns);
    	
    	if ( CurrentResult ) {
    		DecodeFailureResult( CurrentResult, rfdCurrentFailure );
    		
    		ts = TEST_FAILURE;
    		
    		break;
    	}
    }
    
    return ts;
}


void BlackfinDiagRegistersTest::DecodeFailureResult( UINT32 Result, RegisterFailureData & rfdDecodedData ) {

	static const UINT32 InvalidRegTestPattern              = 1;
	
	
	rfdDecodedData.FailureNumber = Result & 0xff;
	
	rfdDecodedData.TestType      = (Result & 0xff00 ) >> 8;
    
	UINT32 idx = Result & 0xffff0000;
    
	idx >>= 16;
    
	if ( idx < NumberOfRegisterPatterns ) {
		rfdDecodedData.FailurePatternIdx = idx;
	}
	else {
		rfdDecodedData.FailurePatternIdx = InvalidRegTestPattern;
	}
}
 	
