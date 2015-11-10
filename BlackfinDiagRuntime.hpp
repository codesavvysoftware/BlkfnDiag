#pragma once
#include "BlackfinDiagTest.hpp"
#include "BlackfinDiagInstructionRam.hpp"
#include "BlackfinDiagDataRam.hpp"
#include "BlackfinDiagRegistersTest.hpp"
#include "BlackfinDiagTimerTest.hpp"

namespace BlackfinDiagRuntimeEnvironment 
{  
    //
    // For linkage to c callable assembly language register tests
    //
    extern "C" UINT32  BlackfinDiagRegSanityChk( const UINT32 *, UINT32);
    extern "C" UINT32  BlackfinDiagRegDataReg7Chk( const UINT32 *, UINT32 );
    extern "C" UINT32  BlackfinDiagRegDataReg6Chk( const UINT32 *, UINT32 );
    extern "C" UINT32  BlackfinDiagRegDataReg5Chk( const UINT32 *, UINT32 );
    extern "C" UINT32  BlackfinDiagRegDataReg4Chk( const UINT32 *, UINT32 );
    extern "C" UINT32  BlackfinDiagRegDataReg3Chk( const UINT32 *, UINT32 );
    extern "C" UINT32  BlackfinDiagRegPointerReg5Chk( const UINT32 *, UINT32 );
    extern "C" UINT32  BlackfinDiagRegPointerReg4Chk( const UINT32 *, UINT32 );
    extern "C" UINT32  BlackfinDiagRegPointerReg3Chk( const UINT32 *, UINT32 );
    extern "C" UINT32  BlackfinDiagRegPointerReg2Chk( const UINT32 *, UINT32 );
    extern "C" UINT32  BlackfinDiagRegPointerReg0Chk( const UINT32 *, UINT32 );
    extern "C" UINT32  BlackfinDiagAccum0Chk( const UINT32 *, UINT32 );
    extern "C" UINT32  BlackfinDiagAccum1Chk( const UINT32 *, UINT32 );
    extern "C" UINT32  BlackfinDiagRegModifyReg3Chk( const UINT32 *, UINT32 );
    extern "C" UINT32  BlackfinDiagRegModifyReg2Chk( const UINT32 *, UINT32 );
    extern "C" UINT32  BlackfinDiagRegModifyReg1Chk( const UINT32 *, UINT32 );
    extern "C" UINT32  BlackfinDiagRegModifyReg0Chk( const UINT32 *, UINT32 );
    extern "C" UINT32  BlackfinDiagRegLengthReg3Chk( const UINT32 *, UINT32 );
    extern "C" UINT32  BlackfinDiagRegLengthReg2Chk( const UINT32 *, UINT32 );
    extern "C" UINT32  BlackfinDiagRegLengthReg1Chk( const UINT32 *, UINT32 );
    extern "C" UINT32  BlackfinDiagRegLengthReg0Chk( const UINT32 *, UINT32 );
    extern "C" UINT32  BlackfinDiagRegIndexReg3Chk( const UINT32 *, UINT32 );
    extern "C" UINT32  BlackfinDiagRegIndexReg2Chk( const UINT32 *, UINT32 );
    extern "C" UINT32  BlackfinDiagRegIndexReg1Chk( const UINT32 *, UINT32 );
    extern "C" UINT32  BlackfinDiagRegIndexReg0Chk( const UINT32 *, UINT32 );
    extern "C" UINT32  BlackfinDiagRegBaseReg3Chk( const UINT32 *, UINT32 );
    extern "C" UINT32  BlackfinDiagRegBaseReg2Chk( const UINT32 *, UINT32 );
    extern "C" UINT32  BlackfinDiagRegBaseReg1Chk( const UINT32 *, UINT32 );
    extern "C" UINT32  BlackfinDiagRegBaseReg0Chk( const UINT32 *, UINT32 );	

    template<typename T, size_t N>
    T * end(T (&ra)[N]) 
    {
    	return ra + N;
    }
    class BlackfinDiagRuntime 
    {
        private:
        	//***********************************************************************************************************
        	//                                                                                                          *
        	// Data RAM testing parameters, structures and definitions.                                                 *
        	//                                                                                                          *
        	//***********************************************************************************************************
        	static const UINT32                                                           NMBR_DATA_RAM_BYTES_TESTED_PER_ITERATION;
	
        	static const DiagnosticCommon::DiagElapsedTime                                DATA_RAM_TEST_ITERATION_PERIOD_MS;
	
        	static const BlackfinDiagTesting::BlackfinDiagTest::BlackfinDataRamTestSuite  DATA_RAM_TEST_TEST_SUITE;

        	static const UINT8                                                            DATA_RAM_TEST_TEST_PATTERNS[];

        	static const UINT32                                                           DATA_RAM_TEST_NMBR_TEST_PATTERNS;
	
	        static const UINT32                                                           MEMORY_BANK_FAILURE_BIT_POS;
	
	        static const UINT32                                                           TEST_PATTERNS_ERROR_BIT_POS;

        	static const BlackfinDiagTesting::BlackfinDiagTest::BlackfinExecTestData      DATA_RAM_TEST_TEST_DATA;

            static BlackfinDiagTesting::BlackfinDiagDataRam                               m_DataRamTest;
    
    
    
        	//***********************************************************************************************************
        	//                                                                                                          *
        	// Register testing parameters, structures and definitions.                                                 *
        	//                                                                                                          *
        	//***********************************************************************************************************
        	//
         	static const UINT32                                                           REGISTER_TEST_TEST_PATTERNS[];
        	static const UINT32                                                           REGISTER_TEST_NMBR_OF_TEST_PATTERNS;
	
            //
            // Only one test but has flexibility to add more and we may break up current test.
            //
            static const BlackfinDiagTesting::BlackfinDiagTest::pRegisterTest             m_SanityCheck[]; 
            static const UINT32                                                           NMBR_OF_SANITY_CHECKS;
    		
            static const BlackfinDiagTesting::BlackfinDiagTest::pRegisterTest             m_DataRegisters[];      
            static const UINT32                                                           NMBR_OF_DATA_REG_TESTS;
    
            static const BlackfinDiagTesting::BlackfinDiagTest::pRegisterTest             m_PointerRegisters[];
            static const UINT32                                                           NMBR_OF_POINTER_REG_TESTS;

            static const BlackfinDiagTesting::BlackfinDiagTest::pRegisterTest             m_Accumulators[];
        	static const UINT32                                                           NMBR_OF_ACCUM_REG_TESTS;
	
        	static const BlackfinDiagTesting::BlackfinDiagTest::pRegisterTest             m_ModifyRegisters[];
        	static const UINT32                                                           NMBR_OF_MODIFY_REG_TESTS;

        	static const BlackfinDiagTesting::BlackfinDiagTest::pRegisterTest             m_LengthRegisters_[];
        	static const UINT32                                                           NMBR_OF_LENGTH_REG_TESTS;

        	static const BlackfinDiagTesting::BlackfinDiagTest::pRegisterTest             m_IndexRegisters_[];
        	static const UINT32                                                           NMBR_OF_INDEX_REG_TESTS;

        	static const BlackfinDiagTesting::BlackfinDiagTest::pRegisterTest             m_BaseRegisters_[];   
        	static const UINT32                                                           NMBR_OF_BASE_REG_TESTS;

            static const BlackfinDiagTesting::BlackfinDiagTest::RegisterTestDescriptor    REGISTER_TEST_TEST_DESCRIPTORS[];
    
            static const UINT32                                                           REGISTER_TEST_NUMBER_TEST_DESCRIPTORS;
 
        	static const DiagnosticCommon::DiagElapsedTime                                REGISTER_TEST_ITERATION_PERIOD_MS;
        	
        	static const UINT32                                                           CORRUPTED_REG_TST_SUITE;
	
            static const BlackfinDiagTesting::BlackfinDiagTest::BlackfinExecTestData      REGISTER_TEST_TEST_DATA;
    
            static BlackfinDiagTesting::BlackfinDiagRegistersTest                         m_RegisterTest;
            
            
    
        	//***********************************************************************************************************
        	//                                                                                                          *
        	// Instruction RAM testing parameters, structures and definitions.                                          *
        	//                                                                                                          *
        	//***********************************************************************************************************
        	static const DiagnosticCommon::DiagElapsedTime                              INSTRCTN_RAM_TEST_ITERATION_PERIOD;
            static const UINT8 *                                                        BOOT_STREAM_START;    
            static const void *                                                         INSTR_START_ADDR;    
            static const UINT32                                                         BAD_BOOTSTREAM_ERR;
            static const BOOL                                                           EMULATION_ACTIVE;
            static const UINT32                                                         UNABLE_TO_START_ERR;
        	static const UINT32                                                         MISMATCH_ERR;
	        static const BlackfinDiagTesting::BlackfinDiagTest::BlackfinExecTestData    TEST_DATA_INTSTR_RAM_TEST;
    
        	static BlackfinDiagTesting::BlackfinDiagInstructionRam                      m_InstructionRamTest;
	
        	//***********************************************************************************************************
        	//                                                                                                          *
        	// Timer testing parameters, structures and definitions.                                          *
        	//                                                                                                          *
        	//***********************************************************************************************************
        	static const DiagnosticCommon::DiagElapsedTime                              TIMER_TEST_ITERATION_PERIOD;
	
        	static const UINT32                                                         m_TimerTestApexTimerErr;
        	static const UINT32                                                         m_TimerTestHostTimerErr;
        	static const DiagnosticCommon::DiagElapsedTime                              m_MaxTimerTestElapsedTimeApex_;
        	static const DiagnosticCommon::DiagElapsedTime                              m_MaxTimerTestElapsedTimeHost_;
        	static const DiagnosticCommon::DiagElapsedTime                              m_MinTimerTestElapsedTimeApex_;
        	static const DiagnosticCommon::DiagElapsedTime                              m_MinTimerTestElapsedTimeHost_; 
            static const BlackfinDiagTesting::BlackfinDiagTest::BlackfinExecTestData    TIMER_TEST_TEST_DATA;
    
        	static BlackfinDiagTesting::BlackfinDiagTimerTest                           m_TimerTest;
	
        	//***********************************************************************************************************
        	//                                                                                                          *
        	// Run the complete tests definitions.                                                                      *
        	//                                                                                                          *
        	//***********************************************************************************************************
            // Define the array of tests to run for the diagnostics
            static BlackfinDiagTesting::BlackfinDiagTest *                                m_pDiagnosticTests[];

            // Use a vector, it makes iterating through the tests easier when executing the tests
            static std::vector <BlackfinDiagTesting::BlackfinDiagTest *>                  m_Diagnostics;
    
        	static const UINT32    CORRUPTED_DIAG_TEST_VECTOR_ERR = 1;
	
        	static const UINT32    CORRUPTED_DIAG_TEST_MEMORY_ERR = 2;
	
        	static const UINT32    ALL_DIAG_DID_NOT_COMPLETE_ERR  = 3;
        	
        	static const UINT32    SCHEDULER_TEST_TYPE            = BlackfinDiagTesting::BlackfinDiagTest::DIAG_SCHEDULER_TEST_TYPE;
	
            // Constructor definition
            BlackfinDiagRuntime();
    
        public:
	
            static void ExecuteDiagnostics();

        };

};

