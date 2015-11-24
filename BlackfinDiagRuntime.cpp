///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @file BlackfinDiagRuntime.cpp
///
/// Namespace for creating runtime environment for diagnostic tests. 
///
/// @see BlackfinDiagRuntime.hpp for a detailed description of this class.
///
/// @if REVISION_HISTORY_INCLUDED
/// @par Edit History
/// - [0]  thaley1  01-Dec-2015 Initial revision of file.
/// @endif
///
/// @ingroup Diagnostics
///
/// @par Copyright (c) 2013 Rockwell Automation Technologies, Inc.  All rights reserved.
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// SYSTEM INCLUDES

// C PROJECT INCLUDES
#include "Defs.h"
#include "Os_iotk.h"
#include "Hw.h"

// C++ PROJECT INCLUDES

#include "BlackfinDiagScheduler.hpp"
#include "BlackfinDiagRuntime.hpp"
#include "DiagnosticTiming.hpp"
#include "BlackfinDiagTest.hpp"
#include "BlackfinDiagInstructionRam.hpp"
#include "BlackfinDiagDataRam.hpp"
#include "BlackfinDiagRegistersTest.hpp"
#include "BlackfinDiagTimerTest.hpp"
#include "BlackfinDiagInstructionsTest.hpp"

using namespace BlackfinDiagTesting;

namespace BlackfinDiagRuntimeEnvironment 
{

    extern "C" void RunDiagnostics() 
    {
	    BlackfinDiagRuntimeEnvironment::BlackfinDiagRuntime::ExecuteDiagnostics();
    }

    static void BlackfinCrash( INT errorCode )
    {
        OS_Assert( errorCode );
    }
    //
    // Default initial values for diagnostic data.
    //
    #define DFLT_INITIAL_TIMESTAMP                0 
    #define DFLT_INITIAL_ELAPSED_TIME             0           
    #define DFLT_NBR_TIMES_TO_RUN_PER_DIAG_CYCLE  1 
    #define DFLT_NBR_TIMES_RAN_THIS_DIAG_CYCLE    1
    #define DFLT_INITIAL_TEST_EXECUTION_STATE     TEST_IDLE


    //***************************************************************************
    // PUBLIC METHODS
    //***************************************************************************

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////
    ///	METHOD NAME: BlackfinDiagRuntime: ExecuteDiagnostics
    ///
    /// @par Full Description
    ///      Configures the runtime environment by constructing the diagnostic tests and instantiating the 
    ///      This is the function that ultimately is called.  It is called by RunDiagnostics but RunDiagnostics
    ///      only provides a linkage between C and C++.
    ///      
    ///
    ///                               
    /// @return                             Scheduler is created on first call and then called from the
    ///                                     background..
    ///
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////
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


            // Initial values of data accessed by the scheduler
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
 
											
            // Data ram memory regions tested
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
                                                                        
            // Create DataRamTest object. Refer to BlackfinDataRam.hpp and BlackfinDataRam.cpp for a description
            static BlackfinDiagDataRam m_DataRamTest(  BANK_A, 
                                                       BANK_B,
                                                       BANK_C, 
                                                       DATA_RAM_TEST_TEST_PATTERNS, 
                                                       ( sizeof( DATA_RAM_TEST_TEST_PATTERNS ) / sizeof( UINT8 ) ), 
                                                       NMBR_DATA_RAM_BYTES_TESTED_PER_ITERATION,
                                                       execTestData ); 
    

            //***********************************************************************************************************
            //                                                                                                          *
            // Register testing parameters, structures and definitions.                                                 *
            //                                                                                                          *
            //***********************************************************************************************************
            #define  REGISTER_TEST_ITERATION_PERIOD_MS 2000          // Every Two Seconds	
	
            execTestData.m_IterationPeriod                   = REGISTER_TEST_ITERATION_PERIOD_MS;
       		execTestData.m_TestType                          = BlackfinDiagTest::DIAG_REGISTER_TEST_TEST_TYPE;


            // Create Register Test object.  Refer to BlackfinDiagRegistersTest.hpp and BlackfinDiagRegistersTest.cpp 
            // for a description.
            static BlackfinDiagRegistersTest m_RegisterTest( execTestData ); 
 

            //***********************************************************************************************************
            //                                                                                                          *
            // Instruction RAM testing parameters, structures and definitions.                                          *
            //                                                                                                          *
            //***********************************************************************************************************
            #define INSTRCTN_RAM_TEST_ITERATION_PERIOD_MS 2000          // 2 second for now
    
            execTestData.m_IterationPeriod                   = INSTRCTN_RAM_TEST_ITERATION_PERIOD_MS;
            execTestData.m_TestType                          = BlackfinDiagTest::DIAG_INTRUCTION_RAM_TEST_TYPE;

            // Create Instruction Ram Test object.  Refer to BlackfinInstructionRam.hpp and BlackfinInstructionRam.cpp 
            // for a description.
            static BlackfinDiagInstructionRam m_InstructionRamTest( execTestData );

            //***********************************************************************************************************
            //                                                                                                          *
            // Timer testing parameters, structures and definitions.                                                    *
            //                                                                                                          *
            //***********************************************************************************************************
            #define TIMER_TEST_ITERATION_PERIOD_MS   1000  // Every second.
            execTestData.m_IterationPeriod                   = TIMER_TEST_ITERATION_PERIOD_MS;
       		execTestData.m_TestType                          = BlackfinDiagTest::DIAG_TIMER_TEST_TYPE;

            // Create Timer Test object.  Refer to BlackfinTimerTest.hpp and BlackfinTimerTest.cpp 
            // for a description.
            static BlackfinDiagTesting::BlackfinDiagTimerTest    m_TimerTest( execTestData );
					                                                 
			
            //***********************************************************************************************************
            //                                                                                                          *
            // Instructions testing parameters, structures and definitions.                                             *
            //                                                                                                          *
            //***********************************************************************************************************
            #define INSTRUCTIONS_TEST_ITERATION_PERIOD_MS 5000
	
            execTestData.m_IterationPeriod                   = INSTRUCTIONS_TEST_ITERATION_PERIOD_MS;
       		execTestData.m_TestType                          = BlackfinDiagTest::DIAG_INSTRUCTIONS_TEST_TYPE;

    
            // Create Instructions Test object.  Refer to BlackfinInstructionsTest.hpp and BlackfinInstructionsTest.cpp 
            // for a description.
            static BlackfinDiagTesting::BlackfinDiagInstructionsTest       m_InstructionsTest( execTestData );


            //////////////////////////////////////////////////////////////////////////////////////////////////////////
            ///
            /// Create data structures defining runtime environment for the scheduler
            ///
            //////////////////////////////////////////////////////////////////////////////////////////////////////////
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


