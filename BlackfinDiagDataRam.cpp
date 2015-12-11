///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @file BlackfinDiagDataRam.cpp
///
/// Namespace that contains the class definitions, attributes and methods for the BlackfinDiagDataRam class. 
///
/// @see BlackfinDiagDataRam.hpp for a detailed description of this class.
///
/// @if REVISION_HISTORY_INCLUDED
/// @par Edit History
/// - [0]  thaley1  01-Dec-2015 Initial revision of file.
/// @endif
///
/// @ingroup Diagnostics
///
/// @par Copyright (c) 2015 Rockwell Automation Technologies, Inc.  All rights reserved.
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// SYSTEM INCLUDES
// (none)
//
// C PROJECT INCLUDES
#include "Defs.h"
#include "Os_iotk.h"             // This file depends on Defs.h.  It should include that file
#include "Hw.h"                  // Ditto 


// C++ PROJECT INCLUDES
#include "BlackfinDiagDataRam.hpp"


// FORWARD REFERENCES

namespace BlackfinDiagnosticTesting 
{
	
	// Linkage to the assembly language subrouting for testing a byte of RAM.
	extern "C" BOOL TestAByteOfRam( BlackfinDiagDataRam::ByteTestParameters * pbtp );

    //***************************************************************************
    // PUBLIC METHODS
    //***************************************************************************
            
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////
    ///	METHOD NAME: BlackfinDiagDataRam: RunTest
    ///
    ///      Provides interface specified by the pure virtual method in the base class.  The scheduler calls 
    ///      this method to run iterations of the diagnostic test..
    ///
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////
    DiagnosticTesting::DiagnosticTest::TestState BlackfinDiagDataRam::RunTest( UINT32 & rErrorCode ) 
    {
	
	    ConfigForAnyNewDiagCycle( this );
					
	    BOOL errorExists = TRUE;
	
		DiagnosticTesting::DiagnosticTest::TestState ts = DiagnosticTesting::DiagnosticTest::TEST_LOOP_COMPLETE;
	
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
		 
		     ts = DiagnosticTesting::DiagnosticTest::TEST_IN_PROGRESS;
		
	    }
	    else if ( !m_BankB.m_TestCompleted ) 
	    { 
		
		    errorExists = !RunRamTest( &m_BankB, offsetFromBankStart, failurePattern );
		 
		    if ( errorExists ) 
		    {
		 	    EncodeErrorInfo( rErrorCode, BANK_B, offsetFromBankStart, failurePattern );
		 	
		 	    OS_Assert( rErrorCode );
		     }
		 
		    ts = DiagnosticTesting::DiagnosticTest::TEST_IN_PROGRESS;
		
	    }
	    else if ( !m_BankC.m_TestCompleted ) 
	    { 

		    errorExists = !RunRamTest( &m_BankC, offsetFromBankStart, failurePattern );
		 
		    if ( errorExists ) 
		    {
		 	    EncodeErrorInfo( rErrorCode, BANK_C, offsetFromBankStart, failurePattern );
		 	
		 	    OS_Assert( rErrorCode );
		    }
		 
		    ts = DiagnosticTesting::DiagnosticTest::TEST_IN_PROGRESS;
	   }

	   return ts;
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////
    ///	METHOD NAME: BlackfinDiagDataRam: RunRamTest
    ///
    ///      Tests an "iternation" amount of RAM.
    ///
    ///
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////
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
		

	
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////
    ///	METHOD NAME: BlackfinDiagDataRam: TestAByte
    ///
    ///      Tests one byte of RAM
    ///
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////
    BOOL BlackfinDiagDataRam::TestAByte( ByteTestParameters * pbtp ) 
    {
        BOOL testPassed = FALSE;

	    DisableInterrupts();
		
	    testPassed = TestAByteOfRam( pbtp );

	    EnableInterrupts();

	    return testPassed; 
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////
    ///	METHOD NAME: BlackfinDiagDataRam: EncodeErrorInfo
    ///
    ///      Encoding information about a test failure
    ///
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////
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

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////
    ///	METHOD NAME: BlackfinDiagDataRam: ConfigureForNextTestCycle
    ///
    ///      Provides interface specified by the pure virtual method in the base class. This method is called 
    ///      at that start of testing for the test during a new diagnostics cycle. The data that needs to be 
    ///      initialized for an individual test is initialized.      
    ///
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////
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

