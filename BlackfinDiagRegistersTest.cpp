#include "BlackfinDiagRegistersTest.h"

TestState BlackfinDiagRegistersTest::RunTest() {
	TestState ts = TEST_IN_PROGRESS;

    UINT32 CurrentResult = BlackfinDiagRegSanityChk(NULL, 0 );
    
    if (CurrentResult) {
    	rfdCurrentFailure.FailureNumber = CurrentResult & 0xff;
    	rfdCurrentFailure.TestType      = (CurrentResult & 0xff00 ) >> 8;
    	
    	// No need to store pattern on sanity test.
    
    	return TEST_FAILURE;
    }
    
    //
    // Test the data registers next.
    //
    TestState  tsReturned = RunRegisterTests( PointerRegisters, NumberOfDataRegTests );
    
    if ( TEST_FAILURE == tsReturned ) return tsReturned;

    //
    // Test the pointer registers next.
    //
    tsReturned = RunRegisterTests( PointerRegisters, NumberOfPointerRegTests );
    
    if ( TEST_FAILURE == tsReturned ) return tsReturned;
    
    //
    // Test the accumulator registers next.
    //
    tsReturned = RunRegisterTests( Accumulators, NumberOfAccumulatorRegTests );
    
    if ( TEST_FAILURE == tsReturned ) return tsReturned;
    
    //
    // Test the modify registers next.
    //
    tsReturned = RunRegisterTests( ModifyRegisters, NumberOfModifyRegTests );
    
    if ( TEST_FAILURE == tsReturned ) return tsReturned;

    //
    // Test the length registers next.
    //
    tsReturned = RunRegisterTests( LengthRegisters, NumberOfLengthRegTests );
    
    if ( TEST_FAILURE == tsReturned ) return tsReturned;

    //
    // Test the index registers next.
    //
    tsReturned = RunRegisterTests( IndexRegisters, NumberOfIndexRegTests );
    
    if ( TEST_FAILURE == tsReturned ) return tsReturned;

    //
    // Test the base registers next.
    //
    return RunRegisterTests( BaseRegisters, NumberOfBaseRegTests );
    
}

TestState BlackfinDiagRegistersTest::RunRegisterTests( REGISTER_TEST * RegTestArray, UINT32 NumberOfRegisterTests )
{
    TestState ts = TEST_LOOP_COMPLETE;
    
    //
    // Test the modify registers next.
    //
    for ( UINT32 ui = 0; ui < NumberOfRegisterTests; ui++ ) {
    	UINT32 CurrentResult = (RegTestArray [ui])(TestPatternsForRegisterTesting,NumberOfRegisterPatterns);
    	
    	if ( CurrentResult ) {
    		DecodeFailureResult( CurrentResult, rfdCurrentFailure );
    		
    		ts = TEST_FAILURE;
    		
    		break;
    	}
    }
    
    return ts;
}


void BlackfinDiagRegistersTest::DecodeFailureResult( UINT32 Result, RegisterFailureData & rfdDecodedData ) {
	
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
 	
