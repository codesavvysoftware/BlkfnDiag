#include "BlackfinDiagDataRam.h"

BlackfinDiagTest::TestState BlackfinDiagDataRam::RunTest( UINT32 & ErrorCode, DiagTime_t StartTime ) {
	
	UINT32 ErrorInfo;
	
	BOOL bError = TRUE;
	
	BlackfinDiagTest::TestState ts = TEST_LOOP_COMPLETE;
	
	UINT32 OffsetFromBankStart = 0;
	
	UINT32 FailurePattern = 0;
	
	if ( !DataRamTestSuite->BankA.testCompleted ) { 
		
		 bError = !RunRamTest( &DataRamTestSuite->BankA, OffsetFromBankStart, FailurePattern );
		 
		 if ( bError ) {
		 	EncodeErrorInfo( ErrorInfo, BankA, OffsetFromBankStart, FailurePattern );
		 	
		 	firmExcept( ErrorInfo );
		 }
		 
		 ts = TEST_IN_PROGRESS; 			
		
	}
	else if ( !DataRamTestSuite->BankB.testCompleted ) { 
		
		 bError = !RunRamTest( &DataRamTestSuite->BankB, OffsetFromBankStart, FailurePattern );
		 
		 if ( bError ) {
		 	EncodeErrorInfo( ErrorInfo, BankB, OffsetFromBankStart, FailurePattern );
		 	
		 	firmExcept( ErrorInfo );
		 }
		 
		 ts = TEST_IN_PROGRESS; 			
		
	}
	else if ( !DataRamTestSuite->BankC.testCompleted ) { 

		 bError = !RunRamTest( &DataRamTestSuite->BankC, OffsetFromBankStart, FailurePattern );
		 
		 if ( bError ) {
		 	EncodeErrorInfo( ErrorInfo, BankC, OffsetFromBankStart, FailurePattern );
		 	
		 	firmExcept( ErrorInfo );
		 }
		 
		 ts = TEST_IN_PROGRESS; 			
	}

	return ts;
}

BOOL  BlackfinDiagDataRam::RunRamTest( DataRamTestDescriptor * RamDescriptor, 
                                       UINT32 &                OffsetFromBankStart, 
                                       UINT32 &                FailurePattern ) {
		 
	UINT8 TestPattern = 0x77;
		
	UINT32 NumberOfBytesTestedThisIteration = 0;
	
	UINT32 NumberOfBytesToTestThisIteration = 0;
	
	UINT32 NumberOfBytesLeftToTest = RamDescriptor->NumberOfContiguousBytesToTest - RamDescriptor->NumberOfBytesTested;

	if ( NumberOfBytesLeftToTest >= NumberOfBytesToTestPerIteration ) {
		NumberOfBytesToTestThisIteration = NumberOfBytesToTestPerIteration;
	}
	else {
		NumberOfBytesToTestThisIteration = NumberOfBytesLeftToTest;
	}
		
	UINT8 * pTestStartAddress = RamDescriptor->pDataRamAddressStart + RamDescriptor->NumberOfBytesTested;
		
	UINT8 * pCurrentRamAddress = pTestStartAddress;
	
	BOOL bSuccess = TRUE;

	BOOL bTestNotPassed = FALSE;


	while ( NumberOfBytesTestedThisIteration < NumberOfBytesToTestPerIteration) {

		bTestNotPassed = !TestAByte(pCurrentRamAddress, TestPattern);
		
		if (bTestNotPassed) break;
		
		NumberOfBytesTestedThisIteration++;
		
		pCurrentRamAddress++;
	};

	if (bTestNotPassed) {
		OffsetFromBankStart = pCurrentRamAddress - pTestStartAddress;
		
		FailurePattern = TestPattern;
	
		bSuccess = FALSE;
	}
	else {
		
		RamDescriptor->NumberOfBytesTested += NumberOfBytesTestedThisIteration;
		
		if ( RamDescriptor->NumberOfBytesTested >= RamDescriptor->NumberOfContiguousBytesToTest ) {
			
			RamDescriptor->testCompleted = TRUE;
		}
	}
	
	return bSuccess;
}
		

BOOL BlackfinDiagDataRam::IsTestComplete() {
	return ( 	DataRamTestSuite->BankA.testCompleted 
			 && DataRamTestSuite->BankB.testCompleted
			 && DataRamTestSuite->BankC.testCompleted );
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

void BlackfinDiagDataRam::EncodeErrorInfo( UINT32 &             ErrorInfo, 
                                           DataRamMemoryRegions Region, 
                                           UINT32               OffsetFromBankStart, 
                                           UINT32 FailurePattern ) {
	
    ErrorInfo  = GetTestType() << DiagnosticErrorTestTypeBitPos;
    
    ErrorInfo |= Region << ErrorRegionBitPos;
	
	ErrorInfo |= (FailurePattern << ErrorTestPatternBitPos );
	
	ErrorInfo |= OffsetFromBankStart; 
}
