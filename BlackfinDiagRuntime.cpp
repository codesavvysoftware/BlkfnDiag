#include "BlackfinDiagScheduler.hpp"
#include "BlackfinDiagRuntime.hpp"
#include "Os_iotk.h"
#include "Hw.h"
#include "DiagnosticTiming.hpp"

using namespace BlackfinDiagTesting;


namespace BlackfinDiagRuntimeEnvironment 
{

    #define DFLT_INITIAL_TIMESTAMP                0 
    #define DFLT_INITIAL_ELAPSED_TIME             0           
    #define DFLT_NBR_TIMES_TO_RUN_PER_DIAG_CYCLE  1 
    #define DFLT_NBR_TIMES_RAN_THIS_DIAG_CYCLE    1
    #define DFLT_INITIAL_TEST_EXECUTION_STATE     TEST_IDLE


//***********************************************************************************************************
//                                                                                                          *
// Register testing parameters, structures and definitions.                                                 *
//                                                                                                          *
//***********************************************************************************************************

    static void BlackfinCrash( INT errorCode )
    {
        OS_Assert( errorCode );
    }
        


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
                                                   execTestData ); 
    

        #define  REGISTER_TEST_ITERATION_PERIOD_MS 2000          // Every Two Seconds	
	
        execTestData.m_IterationPeriod                   = REGISTER_TEST_ITERATION_PERIOD_MS;
   		execTestData.m_TestType                          = BlackfinDiagTest::DIAG_REGISTER_TEST_TEST_TYPE;


        static BlackfinDiagRegistersTest m_RegisterTest( execTestData ); 
 

        //***********************************************************************************************************
        //                                                                                                          *
        // Instruction RAM testing parameters, structures and definitions.                                          *
        //                                                                                                          *
        //***********************************************************************************************************
        #define INSTRCTN_RAM_TEST_ITERATION_PERIOD_MS 2000          // 2 second for now
    
        execTestData.m_IterationPeriod                   = INSTRCTN_RAM_TEST_ITERATION_PERIOD_MS;
        execTestData.m_TestType                          = BlackfinDiagTest::DIAG_INTRUCTION_RAM_TEST_TYPE;

        static BlackfinDiagInstructionRam m_InstructionRamTest( execTestData );

        //***********************************************************************************************************
        //                                                                                                          *
        // Timer testing parameters, structures and definitions.                                          *
        //                                                                                                          *
        //***********************************************************************************************************
        #define TIMER_TEST_ITERATION_PERIOD_MS   1000  // Every second.
        execTestData.m_IterationPeriod                   = TIMER_TEST_ITERATION_PERIOD_MS;
   		execTestData.m_TestType                          = BlackfinDiagTest::DIAG_TIMER_TEST_TYPE;

        static BlackfinDiagTesting::BlackfinDiagTimerTest    m_TimerTest( execTestData );
					                                                 
			
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
        #define PERIOD_FOR_ALL_DIAGNOSTICS_COMPLETED_MS     3 * 60 * 1000 // 3 minutes for now 4 * 60 * 60 * 1000 // 4 hours, number of milleseconds in 4 hours
    
        #define PERIOD_FOR_ONE_DIAGNOSTIC_TEST_ITERATION_MS 50 // Milleseconds

        static BlackfinDiagTest * pDiagnosticTests[]    = 
                                                 {
                                                     //&m_RegisterTest,
                                                     //&m_DataRamTest, 
//                                                     &m_TimerTest,
                                                     &m_InstructionRamTest,
                                                     //&m_InstructionsTest,
                                                 };
     
        DiagnosticScheduling::DiagnosticRunTimeParameters drtp   = 
	                                    {
											DiagnosticTiming::GetTimestamp,
											DiagnosticTiming::CalcElapsedTimeMS,
											&BlackfinCrash,
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


};


