#include "BlackfinDiagDataRam.h"
using namespace DiagnosticCommon;

namespace BlackfinDiagTests {extern "C" BOOL TestAByteOfRam( BlackfinDiagDataRam::ByteTestParameters * pbtp );

BlackfinDiagTest::TestState BlackfinDiagDataRam::RunTest( UINT32 & errorCode ) {
	
	ConfigForAnyNewDiagCycle( this );
					
	BOOL errorExists = TRUE;
	
	BlackfinDiagTest::TestState ts = TEST_LOOP_COMPLETE;
	
	UINT32 offsetFromBankStart = 0;
	
	UINT32 failurePattern = 0;
	
	if ( !dataRamTestSuite_->BankA.testCompleted ) { 
		
		 errorExists = !RunRamTest( &dataRamTestSuite_->BankA, offsetFromBankStart, failurePattern );
		 
		 if ( errorExists ) {
		 	EncodeErrorInfo( errorCode, BankA, offsetFromBankStart, failurePattern );
		 	
		 	firmExcept( errorCode );
		 }
		 
		 ts = TEST_IN_PROGRESS; 			
		
	}
	else if ( !dataRamTestSuite_->BankB.testCompleted ) { 
		
		 errorExists = !RunRamTest( &dataRamTestSuite_->BankB, offsetFromBankStart, failurePattern );
		 
		 if ( errorExists ) {
		 	EncodeErrorInfo( errorCode, BankB, offsetFromBankStart, failurePattern );
		 	
		 	firmExcept( errorCode );
		 }
		 
		 ts = TEST_IN_PROGRESS; 			
		
	}
	else if ( !dataRamTestSuite_->BankC.testCompleted ) { 

		 errorExists = !RunRamTest( &dataRamTestSuite_->BankC, offsetFromBankStart, failurePattern );
		 
		 if ( errorExists ) {
		 	EncodeErrorInfo( errorCode, BankC, offsetFromBankStart, failurePattern );
		 	
		 	firmExcept( errorCode );
		 }
		 
		 ts = TEST_IN_PROGRESS; 			
	}

	return ts;
}

BOOL  BlackfinDiagDataRam::RunRamTest( DataRamTestDescriptor * testRAMDescriptor, 
                                       UINT32 &                offsetFromBankStart, 
                                       UINT32 &                failurePattern ) {
		 
	UINT8 testPattern = 0x77;
		
	UINT32 nmbrBytesTestedThisIteration = 0;
	
	UINT32 nmbrBytesToTestThisIteration = 0;
	
	UINT32 nmbrBytesLeftToTest = testRAMDescriptor->nmbrContiguousBytesToTest - testRAMDescriptor->nmbrBytesTested;

	if ( nmbrBytesLeftToTest >= nmbrBytesToTestPerIteration_ ) {
		nmbrBytesToTestThisIteration = nmbrBytesToTestPerIteration_;
	}
	else {
		nmbrBytesToTestThisIteration = nmbrBytesLeftToTest;
	}
		
	UINT8 * testStartAddr = testRAMDescriptor->dataRamAddressStart + testRAMDescriptor->nmbrBytesTested;
		
	UINT8 * crrntRAMAddr = testStartAddr;
	
	BOOL hadSuccess    = TRUE;

	BOOL testNotPassed = FALSE;

	ByteTestParameters         btp;
	
	while ( nmbrBytesTestedThisIteration < nmbrBytesToTestPerIteration_) {

		btp.ptrByteToTest          = crrntRAMAddr;
		btp.ptrPatternThatFailed   = &testPattern;
		btp.ptrTestPatterns        = testPatternsRAM_;
		btp.nmbrTestPatterns       = nmbrTestPatterns_;
		
		testNotPassed = !TestAByte(&btp);
		
		if (testNotPassed) break;
		
		++nmbrBytesTestedThisIteration;
		
		++crrntRAMAddr;
		
		if (!(nmbrBytesTestedThisIteration & 0xff))
		{
			int i = 0;
			
			++i;
		}
		else if (!(nmbrBytesTestedThisIteration & 0xf)) {
			int i = 0;
			
			++i;
		}
	};

	if (testNotPassed) {
		offsetFromBankStart = crrntRAMAddr - testStartAddr;
		
		failurePattern = testPattern;
	
		hadSuccess = FALSE;
	}
	else {
		
		testRAMDescriptor->nmbrBytesTested += nmbrBytesTestedThisIteration;
		
		if ( testRAMDescriptor->nmbrBytesTested >= testRAMDescriptor->nmbrContiguousBytesToTest ) {
			
			testRAMDescriptor->testCompleted = TRUE;
		}
	}
	
	return hadSuccess;
}
		

	
BOOL BlackfinDiagDataRam::TestAByte( ByteTestParameters * pbtp ) {
	BOOL testPassed = FALSE;

	DisableInterrupts();

		
	testPassed = TestAByteOfRam( pbtp );
	
	//TestByteForAllTestPatterns(pbtp);

	EnableInterrupts();

	return testPassed; 
}

void BlackfinDiagDataRam::EncodeErrorInfo( UINT32 &             errorInfo, 
                                           DataRamMemoryBanks   memoryBank, 
                                           UINT32               offsetFromBankStart, 
                                           UINT32               failurePattern ) {
	
    errorInfo  = GetTestType() << DiagnosticErrorTestTypeBitPos;
    
    errorInfo |= memoryBank << memoryBankFailureBitPos_;
	
	errorInfo |= (failurePattern << testPatternErrorBitPos_ );
	
	errorInfo |= offsetFromBankStart; 
}

void BlackfinDiagDataRam::ConfigureForNextTestCycle() {
	dataRamTestSuite_->BankA.testCompleted     = FALSE;
	
	dataRamTestSuite_->BankA.nmbrBytesTested   = 0;
		
	dataRamTestSuite_->BankB.testCompleted     = FALSE;
		
	dataRamTestSuite_->BankB.nmbrBytesTested   = 0;
		
	dataRamTestSuite_->BankC.testCompleted     = FALSE;

	dataRamTestSuite_->BankC.nmbrBytesTested   = 0;		
}
};

