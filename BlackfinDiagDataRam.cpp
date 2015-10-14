#include "BlackfinDiagDataRam.h"

BlackfinDiagTest::TestState BlackfinDiagDataRam::RunTest( UINT32 & ErrorCode, DiagTime_t StartTime ) {
	
	UINT32 NumberOfBytesTestedThisIteration = 0;

	BOOL bTestNotPassed = FALSE;

	TestState ts = TEST_IN_PROGRESS;
	
	

	while (    (NumberOfBytesTested < TotalNumberOfRamBytesToTest) 
	        && (NumberOfBytesTestedThisIteration < NumberOfBytesToTestPerIteration)
	      ) {

		UINT8 FailurePattern = 0x77;
		
		bTestNotPassed = !TestAByte(pCurrentRamAddress, FailurePattern);
		
		if (bTestNotPassed) break;
		
		NumberOfBytesTested++;
		
		NumberOfBytesTestedThisIteration++;
		
		pCurrentRamAddress++;
	};

	if (bTestNotPassed) {
		
		
		
		ts = TEST_FAILURE;
	}
	else if (NumberOfBytesTested >= TotalNumberOfRamBytesToTest) {
		pCurrentRamAddress = pFirstRamAddressToTest;
		
		NumberOfBytesTested = 0;
		
		ts = TEST_LOOP_COMPLETE;
	}

	return ts;
}

BOOL BlackfinDiagDataRam::TestAByte(UINT8 * pByteToTest, UINT8 & PatternThatFailed) {
	BOOL bTestPassed = FALSE;

	DisableInterrupts();

	bTestPassed = TestByteForAllTestPatterns(pByteToTest, PatternThatFailed);

	EnableInterrupts();

	return bTestPassed; 
}
BOOL BlackfinDiagDataRam::TestByteForAllTestPatterns(UINT8 * pByteToTest, UINT8 & PatternThatFailed) {
	BOOL bTestPassed = TRUE;

	UINT8 SavedValue = *pByteToTest;

	const UINT8 * pPattern = &RamTestPatterns[0];

	for (UINT32 ui = 0; ui < NumberOfTestPatterns; pPattern++, ui++) {
		*pByteToTest = *pPattern;
		
		UINT8 pByteRead = *pByteToTest;
		
		if (pByteRead != *pPattern) {
			PatternThatFailed = *pPattern;
			
			bTestPassed = FALSE;
			
			break;
		}
	}
	
	*pByteToTest = SavedValue;

	return bTestPassed;		
}
