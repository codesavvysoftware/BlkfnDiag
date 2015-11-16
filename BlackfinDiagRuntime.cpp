#include "BlackfinDiagScheduler.hpp"
#include "BlackfinDiagRuntime.hpp"
#include "Os_iotk.h"
#include "Hw.h"

using namespace DiagnosticCommon;
using namespace BlackfinDiagTesting;


namespace BlackfinDiagRuntimeEnvironment 
{

    #define DFLT_INITIAL_TIMESTAMP                0 
    #define DFLT_INITIAL_ELAPSED_TIME             0           
    #define DFLT_NBR_TIMES_TO_RUN_PER_DIAG_CYCLE  1 
    #define DFLT_NBR_TIMES_RAN_THIS_DIAG_CYCLE    0
    #define DFLT_INITIAL_TEST_EXECUTION_STATE     TEST_IDLE


//***********************************************************************************************************
//                                                                                                          *
// Register testing parameters, structures and definitions.                                                 *
//                                                                                                          *
//***********************************************************************************************************



void BlackfinDiagRuntime::ExecuteDiagnostics() 
{ 
    static BOOL Initialized;  // FALSE by default but why not
    
    static DiagnosticScheduling::DiagnosticScheduler<BlackfinDiagTesting::BlackfinDiagTest> * pSchedule;
    
    if ( !Initialized ) 
    {

        //***********************************************************************************************************
        //                                                                                                          *
        // Data RAM testing parameters, structures and definitions.                                                 *
        //                                                                                                          *
        //***********************************************************************************************************

        #define NMBR_DATA_RAM_BYTES_TESTED_PER_ITERATION  0x400 // Test 1k at a time for now
        #define MEMORY_BANK_FAILURE_BIT_POS               30
        #define TEST_PATTERNS_ERROR_BIT_POS               16
        #define DATA_RAM_TEST_ITERATION_PERIOD_MS         1000 // 1 second for now

        static UINT8 DATA_RAM_TEST_TEST_PATTERNS[]  = 
                                                { 
                                                    0xff,0, 0x55, 0xaa, 0xf, 0xf0, 0xa0, 0xa, 0x50, 0x5, 0x5a, 0xa5 
                                                };


        BlackfinDiagTest::BlackfinExecTestData execTestData = 
                                                            {
                                                                0,
                                                                DFLT_INITIAL_TIMESTAMP,
                                                                DFLT_INITIAL_TIMESTAMP,
                                                                DFLT_INITIAL_ELAPSED_TIME,
                                                                DFLT_INITIAL_ELAPSED_TIME,
                                                                DFLT_INITIAL_TIMESTAMP,
                                                                DFLT_NBR_TIMES_TO_RUN_PER_DIAG_CYCLE,
                                                                DFLT_NBR_TIMES_RAN_THIS_DIAG_CYCLE,
                                                                BlackfinDiagTest::DIAG_NO_TEST_TYPE,
                                                                DFLT_INITIAL_TEST_EXECUTION_STATE
                                                            };	

        
        execTestData.m_IterationPeriod                   = DATA_RAM_TEST_ITERATION_PERIOD_MS;
   		execTestData.m_TestType                          = BlackfinDiagTest::DIAG_DATA_RAM_TEST_TYPE;
 
											
        BlackfinDiagDataRam::DataRamTestDescriptor BANK_A =
                { 
				   reinterpret_cast<UINT8 *>(0xff800000), 
				   0x8000, 
				   0, 
				   FALSE 
				}; // Bank A

		BlackfinDiagDataRam::DataRamTestDescriptor BANK_B =
                { 
				   reinterpret_cast<UINT8 *>(0xff900000), 
				   0x8000, 
				   0, 
				   FALSE 
				}; // Bank B
                                                                             
        BlackfinDiagDataRam::DataRamTestDescriptor BANK_C =
                { 
				   reinterpret_cast<UINT8 *>(0xffb00000), 
				   0x1000, 
				   0, 
				   FALSE 
				}; // Bank C
                                                                        
        static BlackfinDiagDataRam m_DataRamTest(  BANK_A, 
                                                   BANK_B,
                                                   BANK_C, 
                                                   DATA_RAM_TEST_TEST_PATTERNS, 
                                                   ( sizeof( DATA_RAM_TEST_TEST_PATTERNS ) / sizeof( UINT8 ) ), 
                                                   NMBR_DATA_RAM_BYTES_TESTED_PER_ITERATION,
                                                   MEMORY_BANK_FAILURE_BIT_POS,
                                                   TEST_PATTERNS_ERROR_BIT_POS,
                                                   execTestData ); 
    
        static const UINT32 REGISTER_TEST_TEST_PATTERNS[]  = 
                                            { 
                                                0xffffffff, 
                                                0xaaaaaaaa, 
                                                0x55555555, 
                                                0 
                                            };
    
        //
        // Only one test but has flexibility to add more and we may break up current test.
        //
        static BlackfinDiagRegistersTest::pRegisterTest m_SanityCheck[] = 
    	                                        { 
    	                                            BlackfinDiagRegSanityChk 
    	                                        };
    	                                        

        static BlackfinDiagRegistersTest::pRegisterTest m_DataRegisters[]   =  
                                                    {     
                                            		    BlackfinDiagRegDataReg7Chk, 
                                                        BlackfinDiagRegDataReg6Chk,
                                                        BlackfinDiagRegDataReg5Chk,
                                                        BlackfinDiagRegDataReg4Chk,
                                                        BlackfinDiagRegDataReg3Chk
                                                    };

        static BlackfinDiagRegistersTest::pRegisterTest m_PointerRegisters[]    = 
                                                        {
    		                                                BlackfinDiagRegPointerReg5Chk, 
                                                            BlackfinDiagRegPointerReg4Chk,
                                                            BlackfinDiagRegPointerReg3Chk,
                                                            BlackfinDiagRegPointerReg2Chk,
                                                            BlackfinDiagRegPointerReg0Chk
                                                        };
                                                        
        static BlackfinDiagRegistersTest::pRegisterTest m_Accumulators[]    = 
                                                    {
                                                        BlackfinDiagAccum0Chk, 
    		                                            BlackfinDiagAccum1Chk
    	                                            };

        static BlackfinDiagRegistersTest::pRegisterTest m_ModifyRegisters[] = 
                                                    {
			                                            BlackfinDiagRegModifyReg3Chk,
                                                        BlackfinDiagRegModifyReg2Chk,
                                                        BlackfinDiagRegModifyReg1Chk,
                                                        BlackfinDiagRegModifyReg0Chk
                                                    };
        static BlackfinDiagRegistersTest::pRegisterTest m_LengthRegisters[] =
                                                    {
                                        			    BlackfinDiagRegLengthReg3Chk,
                                                        BlackfinDiagRegLengthReg2Chk,
                                                        BlackfinDiagRegLengthReg1Chk,
                                                        BlackfinDiagRegLengthReg0Chk
                                                    };

        static BlackfinDiagRegistersTest::pRegisterTest m_IndexRegisters[]  = 
                                                    {
                                                        BlackfinDiagRegIndexReg3Chk,
                                                        BlackfinDiagRegIndexReg2Chk,
                                                        BlackfinDiagRegIndexReg1Chk,
                                                        BlackfinDiagRegIndexReg0Chk
                                                    };
        static BlackfinDiagRegistersTest::pRegisterTest m_BaseRegisters[]   = 
                                                    {
                                                        BlackfinDiagRegBaseReg3Chk,
                                                        BlackfinDiagRegBaseReg2Chk,
                                                        BlackfinDiagRegBaseReg1Chk,
                                                        BlackfinDiagRegBaseReg0Chk
                                                    };

        static const BlackfinDiagRegistersTest::RegisterTestDescriptor REGISTER_TEST_TEST_DESCRIPTORS[] = 
								{
									{ 
										m_SanityCheck,      
										sizeof(m_SanityCheck)/sizeof( BlackfinDiagRegistersTest::pRegisterTest ),        
										FALSE 
									},
									{ 
										m_DataRegisters,
										sizeof(m_DataRegisters)/sizeof( BlackfinDiagRegistersTest::pRegisterTest ),        
										FALSE  
									},
									{ 
										m_PointerRegisters, 
										sizeof(m_PointerRegisters)/sizeof( BlackfinDiagRegistersTest::pRegisterTest ),     
										FALSE 
									},
									{ 
										m_Accumulators,     
										sizeof(m_Accumulators)/sizeof( BlackfinDiagRegistersTest::pRegisterTest ), 
										FALSE  
									},
									{ 
										m_BaseRegisters,    
										sizeof(m_BaseRegisters)/sizeof( BlackfinDiagRegistersTest::pRegisterTest ),       
										FALSE 
									},
									{ 
										m_IndexRegisters,   
										sizeof(m_IndexRegisters)/sizeof( BlackfinDiagRegistersTest::pRegisterTest ),
										FALSE  
									},
									{ 
										m_LengthRegisters,  
										sizeof(m_LengthRegisters)/sizeof( BlackfinDiagRegistersTest::pRegisterTest ),      
										FALSE 
									},
									{ 
										m_ModifyRegisters,  
										sizeof(m_ModifyRegisters)/sizeof( BlackfinDiagRegistersTest::pRegisterTest ),      
										FALSE  
									}
								};
	 

        #define CORRUPTED_REG_TST_SUITE 0xff
	
        #define  REGISTER_TEST_ITERATION_PERIOD_MS 2000          // Every Two Seconds	
	
        execTestData.m_IterationPeriod                   = REGISTER_TEST_ITERATION_PERIOD_MS;
   		execTestData.m_TestType                          = BlackfinDiagTest::DIAG_REGISTER_TEST_TEST_TYPE;


        static BlackfinDiagRegistersTest m_RegisterTest( REGISTER_TEST_TEST_DESCRIPTORS, 
                                                         sizeof( REGISTER_TEST_TEST_DESCRIPTORS ) / sizeof(BlackfinDiagRegistersTest::RegisterTestDescriptor), 
                                                         REGISTER_TEST_TEST_PATTERNS, 
                                                         sizeof(REGISTER_TEST_TEST_PATTERNS)/sizeof(UINT32),
                                                         CORRUPTED_REG_TST_SUITE,
                                                         execTestData ); 
 

        //***********************************************************************************************************
        //                                                                                                          *
        // Instruction RAM testing parameters, structures and definitions.                                          *
        //                                                                                                          *
        //***********************************************************************************************************
        #define INSTRCTN_RAM_TEST_ITERATION_PERIOD_MS 2000          // 2 second for now
        #define BAD_BOOTSTREAM_ERR                    0xffd00000
        #define EMULATION_ACTIVE                      TRUE
        #define UNABLE_TO_START_ERR                   0xfff00000
        #define MISMATCH_ERR                          0xffe00000
        #define BOOT_STREAM_START                     reinterpret_cast<UINT8 *>(0x20040000L)
        #define INSTR_START_ADDR                      reinterpret_cast<void *>(0xffa00000)
    
        execTestData.m_IterationPeriod                   = INSTRCTN_RAM_TEST_ITERATION_PERIOD_MS;
        execTestData.m_TestType                          = BlackfinDiagTest::DIAG_INTRUCTION_RAM_TEST_TYPE;

        static BlackfinDiagInstructionRam m_InstructionRamTest( execTestData,
                                                                 BAD_BOOTSTREAM_ERR,
                                                                 UNABLE_TO_START_ERR,
                                                                 MISMATCH_ERR,
                                                                 BOOT_STREAM_START,
                                                                 INSTR_START_ADDR,
                                                                 EMULATION_ACTIVE );

        //***********************************************************************************************************
        //                                                                                                          *
        // Timer testing parameters, structures and definitions.                                          *
        //                                                                                                          *
        //***********************************************************************************************************
        #define TIMER_TEST_ITERATION_PERIOD_MS   1000  // Every second.
	
        #define TIMER_TEST_APEX_TIMER_ERR        1
        #define TIMER_TEST_HOST_TIMER_ERR        2
        #define MAX_TIMER_TEST_ELAPSED_TIME_APEX 950
        #define MAX_TIMER_TEST_ELAPSED_TIME_HOST 1050
        #define MIN_TIMER_TEST_ELAPSED_TIME_APEX 950
        #define MIN_TIMER_TEST_ELAPSED_TIME_HOST 1050 

        execTestData.m_IterationPeriod                   = TIMER_TEST_ITERATION_PERIOD_MS;
   		execTestData.m_TestType                          = BlackfinDiagTest::DIAG_TIMER_TEST_TYPE;

        static BlackfinDiagTesting::BlackfinDiagTimerTest    m_TimerTest( TIMER_TEST_APEX_TIMER_ERR,
     	                                                                  TIMER_TEST_HOST_TIMER_ERR,
	    												                  MAX_TIMER_TEST_ELAPSED_TIME_APEX,
														                  MAX_TIMER_TEST_ELAPSED_TIME_HOST,
					                                                      MIN_TIMER_TEST_ELAPSED_TIME_APEX,
					                                                      MIN_TIMER_TEST_ELAPSED_TIME_HOST,
					                                                      execTestData );
					                                                 
			
        //***********************************************************************************************************
        //                                                                                                          *
        // Instructions testing parameters, structures and definitions.                                             *
        //                                                                                                          *
        //***********************************************************************************************************
        #define INSTRUCTIONS_TEST_ITERATION_PERIOD_MS 5000
	
        execTestData.m_IterationPeriod                   = INSTRUCTIONS_TEST_ITERATION_PERIOD_MS;
   		execTestData.m_TestType                          = BlackfinDiagTest::DIAG_INSTRUCTIONS_TEST_TYPE;

    
        static BlackfinDiagTesting::BlackfinDiagInstructionsTest       m_InstructionsTest( execTestData );

        #define CORRUPTED_DIAG_TEST_VECTOR_ERR  1
	
        #define CORRUPTED_DIAG_TEST_MEMORY_ERR 2
        	
        #define TEST_TOOK_TOO_LONG_ERR         3
	
        #define ALL_DIAG_DID_NOT_COMPLETE_ERR  4
        	
        //
        // Requirement:  All Diagnostic Tests Complete in 4 Hours.
        //
        #define PERIOD_FOR_ALL_DIAGNOSTICS_COMPLETED_MS     4 * 60 * 60 * 1000 // 4 hours, number of milleseconds in 4 hours
    
        #define PERIOD_FOR_ONE_DIAGNOSTIC_TEST_ITERATION_MS 50 // Milleseconds

        static BlackfinDiagTest * pDiagnosticTests[]    = 
                                                 {
                                                     &m_RegisterTest,
                                                     &m_DataRamTest, 
                                                     &m_TimerTest,
                                                     &m_InstructionRamTest,
                                                     &m_InstructionsTest,
                                                 };
     
        DiagnosticScheduling::DiagnosticRunTimeParameters drtp   = 
	                                    {
											DiagnosticTiming::GetTimestamp,
											DiagnosticTiming::CalcElapsedTimeMS,
											DiagnosticCommon::BlackfinExceptions,
											PERIOD_FOR_ALL_DIAGNOSTICS_COMPLETED_MS,
											PERIOD_FOR_ONE_DIAGNOSTIC_TEST_ITERATION_MS,
											FALSE,    // m_MonitorIndividualTotalTestingTime
											FALSE,    // m_MonitorIndividualTestIterationTimes
											BlackfinDiagTest::DIAG_SCHEDULER_TEST_TYPE,
											DIAG_ERROR_MASK,
                                            DIAG_ERROR_TYPE_BIT_POS,
                                            CORRUPTED_DIAG_TEST_VECTOR_ERR,
                                            CORRUPTED_DIAG_TEST_MEMORY_ERR,
                                            TEST_TOOK_TOO_LONG_ERR,
                                            ALL_DIAG_DID_NOT_COMPLETE_ERR
	                                    }; 
	    
        static DiagnosticScheduling::DiagnosticScheduler<BlackfinDiagTesting::BlackfinDiagTest> Schedule( pDiagnosticTests, 
                                                                                                          sizeof( pDiagnosticTests ) / sizeof(BlackfinDiagTest *),
                                                                                                          drtp );
    
        pSchedule = &Schedule;
        
        Initialized = TRUE;
    }
    
	pSchedule->RunScheduled(); 
}

BlackfinDiagRuntime::BlackfinDiagRuntime() 
{

}


};


