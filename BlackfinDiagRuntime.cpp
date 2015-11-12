#include "BlackfinDiagScheduler.hpp"
#include "BlackfinDiagRuntime.hpp"

using namespace DiagnosticCommon;
using namespace BlackfinDiagTesting;


namespace BlackfinDiagRuntimeEnvironment 
{

    static const DiagTimestampTime            DFLT_INITIAL_TIMESTAMP               = 0; 
    static const DiagElapsedTime              DFLT_INITIAL_ELAPSED_TIME            = 0;           
    static const UINT32                       DFLT_NBR_TIMES_TO_RUN_PER_DIAG_CYCLE = 1; 
    static const UINT32                       DFLT_NBR_TIMES_RAN_THIS_DIAG_CYCLE   = 0;
    static const TestState  DFLT_INITIAL_TEST_EXECUTION_STATE                      = TEST_IDLE;

//***********************************************************************************************************
//                                                                                                          *
// Data RAM testing parameters, structures and definitions.                                                 *
//                                                                                                          *
//***********************************************************************************************************
BlackfinDiagDataRam::BlackfinDataRamTestSuite  DATA_RAM_TEST_TEST_SUITE    = 
                                                                        { 
																            { 
																                reinterpret_cast<UINT8 *>(0xff800000), 
																                0x8000, 
																                0, 
																                FALSE 
																            }, // Bank A
                                                                            { 
                                                                                reinterpret_cast<UINT8 *>(0xff900000), 
                                                                                0x8000, 
                                                                                0, 
                                                                                FALSE 
                                                                            }, // Bank B
                                                                            { 
                                                                                reinterpret_cast<UINT8 *>(0xffb00000), 
                                                                                0x1000, 
                                                                                0, 
                                                                                FALSE
                                                                            }  // Bank C
                                                                        };

const UINT8      DATA_RAM_TEST_TEST_PATTERNS[]  = 
                                                { 
                                                    0xff,0, 0x55, 0xaa, 0xf, 0xf0, 0xa0, 0xa, 0x50, 0x5, 0x5a, 0xa5 
                                                };

UINT32           DATA_RAM_TEST_NMBR_TEST_PATTERNS             = sizeof( DATA_RAM_TEST_TEST_PATTERNS ) / sizeof( UINT8 );
UINT32           NMBR_DATA_RAM_BYTES_TESTED_PER_ITERATION     = 0x400; // Test 1k at a time for now
UINT32           MEMORY_BANK_FAILURE_BIT_POS                  = 30;
UINT32           TEST_PATTERNS_ERROR_BIT_POS                  = 16;
DiagElapsedTime  DATA_RAM_TEST_ITERATION_PERIOD_MS            = 1000; // 1 second for now

BlackfinDiagTest::BlackfinExecTestData DATA_RAM_TEST_TEST_DATA  = 
                                                                {
																   DATA_RAM_TEST_ITERATION_PERIOD_MS,
																   DFLT_INITIAL_TIMESTAMP,
																   DFLT_INITIAL_TIMESTAMP,
																   DFLT_INITIAL_ELAPSED_TIME,
																   DFLT_INITIAL_ELAPSED_TIME,
                                                                   DFLT_INITIAL_TIMESTAMP,
																   DFLT_NBR_TIMES_TO_RUN_PER_DIAG_CYCLE,
																   DFLT_NBR_TIMES_RAN_THIS_DIAG_CYCLE,
																   BlackfinDiagTest::DIAG_DATA_RAM_TEST_TYPE,
																   DFLT_INITIAL_TEST_EXECUTION_STATE
															    };
											
BlackfinDiagDataRam m_DataRamTest(  &DATA_RAM_TEST_TEST_SUITE, 
                                    DATA_RAM_TEST_TEST_PATTERNS, 
                                    DATA_RAM_TEST_NMBR_TEST_PATTERNS, 
                                    NMBR_DATA_RAM_BYTES_TESTED_PER_ITERATION,
                                    MEMORY_BANK_FAILURE_BIT_POS,
                                    TEST_PATTERNS_ERROR_BIT_POS,
                                    DATA_RAM_TEST_TEST_DATA );
    
static BlackfinDiagTest * m_pDataRamTest = &m_DataRamTest;

//***********************************************************************************************************
//                                                                                                          *
// Register testing parameters, structures and definitions.                                                 *
//                                                                                                          *
//***********************************************************************************************************

const UINT32 REGISTER_TEST_TEST_PATTERNS[]  = 
                                            { 
                                                0xffffffff, 
                                                0xaaaaaaaa, 
                                                0x55555555, 
                                                0 
                                            };
UINT32 REGISTER_TEST_NMBR_OF_TEST_PATTERNS  = sizeof(REGISTER_TEST_TEST_PATTERNS)/sizeof(UINT32);
	
  	
    
    //
    // Only one test but has flexibility to add more and we may break up current test.
    //
BlackfinDiagRegistersTest::pRegisterTest m_SanityCheck[] = 
    	                                        { 
    	                                            BlackfinDiagRegSanityChk 
    	                                        };
    	                                        
UINT32 NMBR_OF_SANITY_CHECKS = sizeof(m_SanityCheck)/sizeof( BlackfinDiagRegistersTest::pRegisterTest );
    		


BlackfinDiagRegistersTest::pRegisterTest m_DataRegisters[]   =  
                                                    {     
                                            		    BlackfinDiagRegDataReg7Chk, 
                                                        BlackfinDiagRegDataReg6Chk,
                                                        BlackfinDiagRegDataReg5Chk,
                                                        BlackfinDiagRegDataReg4Chk,
                                                        BlackfinDiagRegDataReg3Chk
                                                    };

UINT32 NMBR_OF_DATA_REG_TESTS = sizeof(m_DataRegisters)/sizeof( BlackfinDiagRegistersTest::pRegisterTest );
    
BlackfinDiagRegistersTest::pRegisterTest m_PointerRegisters[]    = 
                                                        {
    		                                                BlackfinDiagRegPointerReg5Chk, 
                                                            BlackfinDiagRegPointerReg4Chk,
                                                            BlackfinDiagRegPointerReg3Chk,
                                                            BlackfinDiagRegPointerReg2Chk,
                                                            BlackfinDiagRegPointerReg0Chk
                                                        };
                                                        
UINT32 NMBR_OF_POINTER_REG_TESTS = sizeof(m_PointerRegisters)/sizeof( BlackfinDiagRegistersTest::pRegisterTest );


BlackfinDiagRegistersTest::pRegisterTest m_Accumulators[]    = 
                                                    {
                                                        BlackfinDiagAccum0Chk, 
    		                                            BlackfinDiagAccum1Chk
    	                                            };
UINT32 NMBR_OF_ACCUM_REG_TESTS = sizeof(m_Accumulators)/sizeof( BlackfinDiagRegistersTest::pRegisterTest );
	
BlackfinDiagRegistersTest::pRegisterTest m_ModifyRegisters[] = 
                                                    {
			                                            BlackfinDiagRegModifyReg3Chk,
                                                        BlackfinDiagRegModifyReg2Chk,
                                                        BlackfinDiagRegModifyReg1Chk,
                                                        BlackfinDiagRegModifyReg0Chk
                                                    };
UINT32 NMBR_OF_MODIFY_REG_TESTS = sizeof(m_ModifyRegisters)/sizeof( BlackfinDiagRegistersTest::pRegisterTest );

BlackfinDiagRegistersTest::pRegisterTest m_LengthRegisters[] =
                                                    {
                                        			    BlackfinDiagRegLengthReg3Chk,
                                                        BlackfinDiagRegLengthReg2Chk,
                                                        BlackfinDiagRegLengthReg1Chk,
                                                        BlackfinDiagRegLengthReg0Chk
                                                    };
UINT32 NMBR_OF_LENGTH_REG_TESTS = sizeof(m_LengthRegisters)/sizeof( BlackfinDiagRegistersTest::pRegisterTest );

BlackfinDiagRegistersTest::pRegisterTest m_IndexRegisters[]  = 
                                                    {
                                                        BlackfinDiagRegIndexReg3Chk,
                                                        BlackfinDiagRegIndexReg2Chk,
                                                        BlackfinDiagRegIndexReg1Chk,
                                                        BlackfinDiagRegIndexReg0Chk
                                                    };
UINT32 NMBR_OF_INDEX_REG_TESTS = sizeof(m_IndexRegisters)/sizeof( BlackfinDiagRegistersTest::pRegisterTest );

BlackfinDiagRegistersTest::pRegisterTest m_BaseRegisters[]   = 
                                                    {
                                                        BlackfinDiagRegBaseReg3Chk,
                                                        BlackfinDiagRegBaseReg2Chk,
                                                        BlackfinDiagRegBaseReg1Chk,
                                                        BlackfinDiagRegBaseReg0Chk
                                                    };
UINT32 NMBR_OF_BASE_REG_TESTS = sizeof(m_BaseRegisters)/sizeof( BlackfinDiagRegistersTest::pRegisterTest );

const BlackfinDiagRegistersTest::RegisterTestDescriptor REGISTER_TEST_TEST_DESCRIPTORS[] = 
								{
									{ 
										m_SanityCheck,      
										NMBR_OF_SANITY_CHECKS,        
										FALSE 
									},
									{ 
										m_DataRegisters,
										NMBR_OF_DATA_REG_TESTS,        
										FALSE  
									},
									{ 
										m_PointerRegisters, 
										NMBR_OF_POINTER_REG_TESTS,     
										FALSE 
									},
									{ 
										m_Accumulators,     
										NMBR_OF_ACCUM_REG_TESTS, 
										FALSE  
									},
									{ 
										m_BaseRegisters,    
										NMBR_OF_BASE_REG_TESTS,        
										FALSE 
									},
									{ 
										m_IndexRegisters,   
										NMBR_OF_INDEX_REG_TESTS,       
										FALSE  
									},
									{ 
										m_LengthRegisters,  
										NMBR_OF_LENGTH_REG_TESTS,      
										FALSE 
									},
									{ 
										m_ModifyRegisters,  
										NMBR_OF_MODIFY_REG_TESTS,      
										FALSE  
									}
								};
	 
UINT32 REGISTER_TEST_NUMBER_TEST_DESCRIPTORS = sizeof( REGISTER_TEST_TEST_DESCRIPTORS ) / sizeof(BlackfinDiagRegistersTest::RegisterTestDescriptor); 

const UINT32 CORRUPTED_REG_TST_SUITE = 0xff;
	
DiagElapsedTime                REGISTER_TEST_ITERATION_PERIOD_MS = 2000;          // Every Two Seconds	
	
BlackfinDiagTest::BlackfinExecTestData REGISTER_TEST_TEST_DATA = {
																	REGISTER_TEST_ITERATION_PERIOD_MS,
    																DFLT_INITIAL_TIMESTAMP,
    																DFLT_INITIAL_TIMESTAMP,
    																DFLT_INITIAL_ELAPSED_TIME,
    																DFLT_INITIAL_ELAPSED_TIME,
   																    DFLT_INITIAL_TIMESTAMP,
																	DFLT_NBR_TIMES_TO_RUN_PER_DIAG_CYCLE,
																	DFLT_NBR_TIMES_RAN_THIS_DIAG_CYCLE,
																	BlackfinDiagTest::DIAG_REGISTER_TEST_TEST_TYPE,
																	DFLT_INITIAL_TEST_EXECUTION_STATE
																};

BlackfinDiagRegistersTest m_RegisterTest( REGISTER_TEST_TEST_DESCRIPTORS, 
                                          REGISTER_TEST_NUMBER_TEST_DESCRIPTORS, 
                                          REGISTER_TEST_TEST_PATTERNS, 
                                          REGISTER_TEST_NMBR_OF_TEST_PATTERNS,
                                          CORRUPTED_REG_TST_SUITE,
                                          REGISTER_TEST_TEST_DATA ); 
 
static BlackfinDiagTest * m_pRegisterTest = &m_RegisterTest;
                                         
 

//***********************************************************************************************************
//                                                                                                          *
// Instruction RAM testing parameters, structures and definitions.                                          *
//                                                                                                          *
//***********************************************************************************************************
DiagElapsedTime       INSTRCTN_RAM_TEST_ITERATION_PERIOD_MS = 2000;          // 2 second for now
UINT32                BAD_BOOTSTREAM_ERR                    = 0xffd00000;
BOOL                  EMULATION_ACTIVE                      = TRUE;
UINT32                UNABLE_TO_START_ERR                   = 0xfff00000;
UINT32                MISMATCH_ERR                          = 0xffe00000;
UINT8 *               BOOT_STREAM_START                     = reinterpret_cast<UINT8 *>(0x20040000L);
void *                INSTR_START_ADDR                      = reinterpret_cast<void *>(0xffa00000);
    
    

BlackfinDiagTest::BlackfinExecTestData TEST_DATA_INTSTR_RAM_TEST = {
					      												INSTRCTN_RAM_TEST_ITERATION_PERIOD_MS,
																		DFLT_INITIAL_TIMESTAMP,
																		DFLT_INITIAL_TIMESTAMP,
																		DFLT_INITIAL_ELAPSED_TIME,
																		DFLT_INITIAL_ELAPSED_TIME,
       																    DFLT_INITIAL_TIMESTAMP,
											    						DFLT_NBR_TIMES_TO_RUN_PER_DIAG_CYCLE,
							    										DFLT_NBR_TIMES_RAN_THIS_DIAG_CYCLE,
								    									BlackfinDiagTest::DIAG_INTRUCTION_RAM_TEST_TYPE,
														    			DFLT_INITIAL_TEST_EXECUTION_STATE
															         };



BlackfinDiagInstructionRam m_InstructionRamTest( TEST_DATA_INTSTR_RAM_TEST,
                                                 BAD_BOOTSTREAM_ERR,
                                                 UNABLE_TO_START_ERR,
                                                 MISMATCH_ERR,
                                                 BOOT_STREAM_START,
                                                 INSTR_START_ADDR,
                                                 EMULATION_ACTIVE );

static BlackfinDiagTest * m_pInstructionRamTest = &m_InstructionRamTest;
	
//***********************************************************************************************************
//                                                                                                          *
// Timer testing parameters, structures and definitions.                                          *
//                                                                                                          *
//***********************************************************************************************************
DiagnosticCommon::DiagElapsedTime         TIMER_TEST_ITERATION_PERIOD_MS  = 1000;  // Every second.
	
UINT32                                    m_TimerTestApexTimerErr         = 1;
UINT32                                    m_TimerTestHostTimerErr         = 2;
DiagElapsedTime                           m_MaxTimerTestElapsedTimeApex   = 950;
DiagElapsedTime                           m_MaxTimerTestElapsedTimeHost   = 1050;
DiagElapsedTime                           m_MinTimerTestElapsedTimeApex   = 950;
DiagElapsedTime                           m_MinTimerTestElapsedTimeHost   = 1050; 
BlackfinDiagTest::BlackfinExecTestData    TIMER_TEST_TEST_DATA  = 
                                                                {
																    TIMER_TEST_ITERATION_PERIOD_MS,
																    DFLT_INITIAL_TIMESTAMP,
																    DFLT_INITIAL_TIMESTAMP,
																    DFLT_INITIAL_ELAPSED_TIME,
																    DFLT_INITIAL_ELAPSED_TIME,
   																    DFLT_INITIAL_TIMESTAMP,
											    				    DFLT_NBR_TIMES_TO_RUN_PER_DIAG_CYCLE,
							    								    DFLT_NBR_TIMES_RAN_THIS_DIAG_CYCLE,
								    							    BlackfinDiagTest::DIAG_TIMER_TEST_TYPE,
														    	    DFLT_INITIAL_TEST_EXECUTION_STATE
															    };

BlackfinDiagTesting::BlackfinDiagTimerTest    m_TimerTest( m_TimerTestApexTimerErr,
     	                                                   m_TimerTestHostTimerErr,
	    												   m_MaxTimerTestElapsedTimeApex,
														   m_MaxTimerTestElapsedTimeHost,
					                                       m_MinTimerTestElapsedTimeApex,
					                                       m_MinTimerTestElapsedTimeHost ,
					                                       TIMER_TEST_TEST_DATA );
					                                                 
			
static BlackfinDiagTest * m_pTimerTestPtr = &m_TimerTest;

	
//***********************************************************************************************************
//                                                                                                          *
// Instructions testing parameters, structures and definitions.                                             *
//                                                                                                          *
//***********************************************************************************************************
DiagElapsedTime                              INSTRUCTIONS_TEST_ITERATION_PERIOD_MS = 5000;
	
BlackfinDiagTest::BlackfinExecTestData        INSTRUCTIONS_TEST_TEST_DATA   = 
                                                                            {
																                INSTRUCTIONS_TEST_ITERATION_PERIOD_MS,
            																    DFLT_INITIAL_TIMESTAMP,
            																    DFLT_INITIAL_TIMESTAMP,
            																    DFLT_INITIAL_ELAPSED_TIME,
            																    DFLT_INITIAL_ELAPSED_TIME,
            																    DFLT_INITIAL_TIMESTAMP,
            											    				    DFLT_NBR_TIMES_TO_RUN_PER_DIAG_CYCLE,
            							    								    DFLT_NBR_TIMES_RAN_THIS_DIAG_CYCLE,
            								    							    BlackfinDiagTest::DIAG_INSTRUCTIONS_TEST_TYPE,
            														    	    DFLT_INITIAL_TEST_EXECUTION_STATE
            															    };
    
BlackfinDiagTesting::BlackfinDiagInstructionsTest       m_InstructionsTest( INSTRUCTIONS_TEST_TEST_DATA );
//***********************************************************************************************************
//                                                                                                          *
// Scheduler runtime data                                                                    *
//                                                                                                          *
//***********************************************************************************************************
	
//UINT32    SCHEDULERTEST_TYPE              = ;
	
BlackfinDiagTest * BlackfinDiagRuntime::m_pDiagnosticTests[]    = 
                                                                {
                                                                    //		registerTestPtr,
                                                                    //		m_pDataRamTest, 
                                                            		m_pTimerTestPtr,
                                                                    //		m_pInstructionRamTest
                                                                }; 


std::vector <BlackfinDiagTest *> BlackfinDiagRuntime::m_Diagnostics(m_pDiagnosticTests, end( m_pDiagnosticTests));


void BlackfinDiagRuntime::ExecuteDiagnostics() 
{ 
    static DiagnosticScheduling::DiagnosticRunTimeParameters  drtp   = 
	                                    {
											DiagnosticCommon::GetTimestamp,
											DiagnosticCommon::CalcElapsedTimeMS,
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
    static DiagnosticScheduling::DiagnosticScheduler<BlackfinDiagTesting::BlackfinDiagTest> Schedule( &m_Diagnostics, &drtp );

	Schedule.RunScheduled(); 
}

BlackfinDiagRuntime::BlackfinDiagRuntime() 
{

}


};


