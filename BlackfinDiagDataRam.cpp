#include "BlackfinDiagDataRam.h"

extern "C" BOOL TestAByteOfRam( BlackfinDiagDataRam::ByteTestParameters * pbtp );

BlackfinDiagTest::TestState BlackfinDiagDataRam::RunTest( UINT32 & ErrorCode, DiagTime_t StartTime ) {
	
	ConfigForAnyNewDiagCycle( this );
					
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
	else {

		SetTestsCompletedForCycle();
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

		btp.pByteToTest          = pCurrentRamAddress;
		btp.pPatternThatFailed   = &TestPattern;
		btp.pTestPatterns        = &RamTestPatterns[0];
		btp.NumberOfTestPatterns = NumberOfTestPatterns;
		
		bTestNotPassed = !TestAByte(&btp);
		
		if (bTestNotPassed) break;
		
		NumberOfBytesTestedThisIteration++;
		
		pCurrentRamAddress++;
		
		if (!(NumberOfBytesTestedThisIteration & 0xff))
		{
			int i = 0;
			
			i++;
		}
		else if (!(NumberOfBytesTestedThisIteration & 0xf)) {
			int i = 0;
			
			i++;
		}
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
	
	
BOOL BlackfinDiagDataRam::TestAByte( ByteTestParameters * pbtp ) {
	BOOL bTestPassed = FALSE;

	DisableInterrupts();

		
	bTestPassed = TestAByteOfRam( pbtp );
	
	//TestByteForAllTestPatterns(pbtp);

	EnableInterrupts();

	return bTestPassed; 
}
BOOL BlackfinDiagDataRam::TestByteForAllTestPatterns(ByteTestParameters * btp) {

	BOOL bTestPassed = TRUE;

	UINT8 * pTestByte = btp->pByteToTest;
	
	UINT8 * pFailurePattern = btp->pPatternThatFailed;
	
	const UINT8 * pPatterns = btp->pTestPatterns;

	UINT8   SavedValue = *pTestByte;
	
	UINT32  Patterns   = btp->NumberOfTestPatterns;
		
	for (UINT32 ui = 0; ui < Patterns; pPatterns++, ui++) {
		*pTestByte = *pPatterns;
		
		UINT8 pByteRead = *pTestByte;
		
		if (pByteRead != *pPatterns) {
			*pFailurePattern = *pPatterns;
			
			bTestPassed = FALSE;
			
			break;
		}
	}
	
	*pTestByte = SavedValue;

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

void BlackfinDiagDataRam::ConfigureForNextTestCycle() {
	DataRamTestSuite->BankA.testCompleted     = FALSE;
	
	DataRamTestSuite->BankA.NumberOfBytesTested = 0;
		
	DataRamTestSuite->BankB.testCompleted = FALSE;
		
	DataRamTestSuite->BankB.NumberOfBytesTested = 0;
		
	DataRamTestSuite->BankC.testCompleted = FALSE;

	DataRamTestSuite->BankC.NumberOfBytesTested = 0;		
}
