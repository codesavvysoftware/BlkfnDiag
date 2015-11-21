#include "BlackfinDiagDataRam.hpp"
#include "Os_iotk.h"
#include "Hw.h"

namespace BlackfinDiagTesting 
{
	
	extern "C" BOOL TestAByteOfRam( BlackfinDiagDataRam::ByteTestParameters * pbtp );

    TestState BlackfinDiagDataRam::RunTest( UINT32 & rErrorCode ) 
    {
	
	    ConfigForAnyNewDiagCycle( this );
					
	    BOOL errorExists = TRUE;
	
	    TestState ts = TEST_LOOP_COMPLETE;
	
	    UINT32 offsetFromBankStart = 0;
	
	    UINT32 failurePattern = 0;
	
	    if ( !m_BankA.m_TestCompleted ) 
	    { 
		
		    errorExists = !RunRamTest( &m_BankA, offsetFromBankStart, failurePattern );
		 
		    if ( errorExists ) 
		    {
		 	    EncodeErrorInfo( rErrorCode, BANK_A, offsetFromBankStart, failurePattern );
		 	
		 	    OS_Assert( rErrorCode );
		     }
		 
		     ts = TEST_IN_PROGRESS; 			
		
	    }
	    else if ( !m_BankB.m_TestCompleted ) 
	    { 
		
		    errorExists = !RunRamTest( &m_BankB, offsetFromBankStart, failurePattern );
		 
		    if ( errorExists ) 
		    {
		 	    EncodeErrorInfo( rErrorCode, BANK_B, offsetFromBankStart, failurePattern );
		 	
		 	    OS_Assert( rErrorCode );
		     }
		 
		    ts = TEST_IN_PROGRESS; 			
		
	    }
	    else if ( !m_BankC.m_TestCompleted ) 
	    { 

		    errorExists = !RunRamTest( &m_BankC, offsetFromBankStart, failurePattern );
		 
		    if ( errorExists ) 
		    {
		 	    EncodeErrorInfo( rErrorCode, BANK_C, offsetFromBankStart, failurePattern );
		 	
		 	    OS_Assert( rErrorCode );
		    }
		 
		    ts = TEST_IN_PROGRESS; 			
	   }

	   return ts;
    }

    BOOL  BlackfinDiagDataRam::RunRamTest( DataRamTestDescriptor * pTestRAMDescriptor, 
                                           UINT32 &                rOffsetFromBankStart, 
                                           UINT32 &                rFailurePattern ) 
    {
		 
	    UINT8 testPattern = 0x77;
		
	    UINT32 nmbrBytesTestedThisIteration = 0;
	
	    UINT32 nmbrBytesToTestThisIteration = 0;
	
	    UINT32 nmbrBytesLeftToTest = pTestRAMDescriptor->m_NmbrContiguousBytesToTest - pTestRAMDescriptor->m_NmbrBytesTested;

	    if ( nmbrBytesLeftToTest >= m_NmbrBytesToTestPerIteration ) 
	    {
		    nmbrBytesToTestThisIteration = m_NmbrBytesToTestPerIteration;
	    }
	    else 
	    {
		    nmbrBytesToTestThisIteration = nmbrBytesLeftToTest;
	    }
		
		UINT8 * pTestStartAddr = pTestRAMDescriptor->m_pDataRamAddressStart + pTestRAMDescriptor->m_NmbrBytesTested;
		
	    UINT8 * pCrrntRAMAddr = pTestStartAddr;
	
	    BOOL hadSuccess    = TRUE;

	    BOOL testNotPassed = FALSE;

        ByteTestParameters         btp;
	
	    while ( nmbrBytesTestedThisIteration < m_NmbrBytesToTestPerIteration) 
	    {
		    btp.m_pByteToTest          = pCrrntRAMAddr;
		    btp.m_pPatternThatFailed   = &testPattern;
		    btp.m_pTestPatterns        = m_pTestPatternsRAM;
		    btp.m_NmbrTestPatterns     = m_NmbrTestPatterns;
		
		    testNotPassed = !TestAByte(&btp);
		
		    if (testNotPassed) break;
		
		    ++nmbrBytesTestedThisIteration;
		
		    ++pCrrntRAMAddr;
		
		    if (!(nmbrBytesTestedThisIteration & 0xff))
		    {
			    int i = 0;
			
			    ++i;
		    }
		    else if (!(nmbrBytesTestedThisIteration & 0xf)) 
		    {
			    int i = 0;
			
			    ++i;
		    }
	    }

	    if (testNotPassed) 
	    {
		    rOffsetFromBankStart = pCrrntRAMAddr - pTestStartAddr;
		
		    rFailurePattern = testPattern;
	
		    hadSuccess = FALSE;
	    }
	    else 
	    {
		    pTestRAMDescriptor->m_NmbrBytesTested += nmbrBytesTestedThisIteration;
		
		    if ( pTestRAMDescriptor->m_NmbrBytesTested >= pTestRAMDescriptor->m_NmbrContiguousBytesToTest ) 
		    {
			    pTestRAMDescriptor->m_TestCompleted = TRUE;
		    }
	    }
	
	    return hadSuccess;
    }
		

	
    BOOL BlackfinDiagDataRam::TestAByte( ByteTestParameters * pbtp ) 
    {
        BOOL testPassed = FALSE;

	    DisableInterrupts();
		
	    testPassed = TestAByteOfRam( pbtp );

	    EnableInterrupts();

	    return testPassed; 
    }

    void BlackfinDiagDataRam::EncodeErrorInfo( UINT32 &             errorInfo, 
                                               DataRamMemoryBanks   memoryBank, 
                                               UINT32               offsetFromBankStart, 
                                               UINT32               failurePattern ) 
    {
        errorInfo  = GetTestType() << DIAG_ERROR_TYPE_BIT_POS;
    
        errorInfo |= memoryBank << MEMORY_BANK_FAILURE_BIT_POS;
	
	    errorInfo |= (failurePattern << TEST_PATTERNS_ERROR_BIT_POS);
	
	    errorInfo |= offsetFromBankStart; 
    }

    void BlackfinDiagDataRam::ConfigureForNextTestCycle() 
    {
	    m_BankA.m_TestCompleted     = FALSE;
	
	    m_BankA.m_NmbrBytesTested   = 0;
		
	    m_BankB.m_TestCompleted     = FALSE;
		
	    m_BankB.m_NmbrBytesTested   = 0;
		
	    m_BankC.m_TestCompleted     = FALSE;

	    m_BankC.m_NmbrBytesTested   = 0;		
    }

	void BlackfinDiagDataRam::EnableInterrupts() 
	{
		sti(m_Critical);
	}

     void BlackfinDiagDataRam::DisableInterrupts()
	{
		m_Critical = cli();
	}

}

