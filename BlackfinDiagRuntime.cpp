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
/// @par Copyright (c) 2015 Rockwell Automation Technologies, Inc.  All rights reserved.
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// SYSTEM INCLUDES

// C PROJECT INCLUDES
#include "Defs.h"
#include "Os_iotk.h"
#include "Hw.h"
#include "Apex.h"

// C++ PROJECT INCLUDES

#include "DiagnosticScheduler.hpp"
#include "BlackfinDiagRuntime.hpp"
#include "DiagnosticTesting.hpp"
#include "BlackfinDiagInstructionRam.hpp"
#include "BlackfinDiagDataRam.hpp"
#include "BlackfinDiagRegistersTest.hpp"
#include "BlackfinDiagTimerTest.hpp"
#include "BlackfinDiagInstructionsTest.hpp"


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
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////
    ///	METHOD NAME: BlackfinDiagRuntime: ComputeElapsedTimeMS
    ///
    /// @par Full Description
    ///      A fast way to convert clock ticks to milleseconds.  See description below for the math benind it.
    ///      
    ///
    ///                               
    /// @return                             Elapsed time in milleseconds
    ///
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////
    static UINT32 ComputeElapsedTimeMS( UINT64 current, UINT64 previous ) 
	{
        return ( CCLK_TO_US( current - previous ) / 1000 );
    }
	   
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////
    ///	METHOD NAME: BlackfinDiagRuntime: ReadTimestamp
    ///
    /// @par Full Description
    ///      Method scheduler uses to get a timestamp based on clock ticks.
    ///      
    ///
    ///                               
    /// @return                             Current time in clock ticks.
    ///
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////
	static UINT64 ReadTimestamp () 
	{
	    UINT64  timestamp = 0;
	       
	    _GET_CYCLE_COUNT( timestamp );
	        
	    return timestamp;
	}
	
    //
    // Default initial values for diagnostic data.
    //
    static const UINT64  DFLT_INITIAL_TIMESTAMP               = 0; 
    static const UINT32  DFLT_INITIAL_ELAPSED_TIME            = 0;          
    static const UINT32  DFLT_NBR_TIMES_TO_RUN_PER_DIAG_CYCLE = 1; 
    static const UINT32  DFLT_NBR_TIMES_RAN_THIS_DIAG_CYCLE   = 0;


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
    
        static DiagnosticScheduling::DiagnosticScheduler<DiagnosticTesting::DiagnosticTest> * pSchedule;
    
        if ( !Initialized ) 
        {

            //***********************************************************************************************************
            //                                                                                                          *
            // Data RAM testing parameters, structures and definitions.                                                 *
            //                                                                                                          *
            //***********************************************************************************************************

            static const UINT32 NMBR_DATA_RAM_BYTES_TESTED_PER_ITERATION = 0x400; // Test 1k at a time for now
            static const UINT32 DATA_RAM_TEST_ITERATION_PERIOD_MS        = 1000; // 1 second for now

            static UINT8 DATA_RAM_TEST_TEST_PATTERNS[]  = 
                                                    { 
                                                        0xff,0, 0x55, 0xaa, 0xf, 0xf0, 0xa0, 0xa, 0x50, 0x5, 0x5a, 0xa5 
                                                    };


            // Initial values of data accessed by the scheduler
            DiagnosticTesting::DiagnosticTest::ExecuteTestData execTestData = 
                                                                {
                                                                    0,
                                                                    DFLT_INITIAL_TIMESTAMP,
                                                                    DFLT_INITIAL_TIMESTAMP,
                                                                    DFLT_INITIAL_ELAPSED_TIME,
                                                                    DFLT_INITIAL_ELAPSED_TIME,
                                                                    DFLT_INITIAL_TIMESTAMP,
                                                                    DFLT_NBR_TIMES_TO_RUN_PER_DIAG_CYCLE,
                                                                    DFLT_NBR_TIMES_RAN_THIS_DIAG_CYCLE,
                                                                    DiagnosticTesting::DiagnosticTest::DIAG_NO_TEST_TYPE,
                                                                    DiagnosticTesting::DiagnosticTest::TEST_IDLE
                                                                };	

        
            execTestData.m_IterationPeriod                   = DATA_RAM_TEST_ITERATION_PERIOD_MS;
       		execTestData.m_TestType                          = DiagnosticTesting::DiagnosticTest::DIAG_DATA_RAM_TEST_TYPE;
 
											
            // Data ram memory regions tested
            BlackfinDiagnosticTesting::BlackfinDiagDataRam::DataRamTestDescriptor BANK_A =
                    { 
    				   reinterpret_cast<UINT8 *>(0xff800000), 
    				   0x8000, 
    				   0, 
    				   FALSE 
    				}; // Bank A

    		BlackfinDiagnosticTesting::BlackfinDiagDataRam::DataRamTestDescriptor BANK_B =
                    { 
    				   reinterpret_cast<UINT8 *>(0xff900000), 
    				   0x8000, 
    				   0, 
    				   FALSE 
    				}; // Bank B
                                                                             
            BlackfinDiagnosticTesting::BlackfinDiagDataRam::DataRamTestDescriptor BANK_C =
                    { 
    				   reinterpret_cast<UINT8 *>(0xffb00000), 
    				   0x1000, 
    				   0, 
    				   FALSE 
    				}; // Bank C
                                                                        
            // Create DataRamTest object. Refer to BlackfinDataRam.hpp and BlackfinDataRam.cpp for a description
            static BlackfinDiagnosticTesting::BlackfinDiagDataRam m_DataRamTest(  BANK_A, 
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
            static const UINT32  REGISTER_TEST_ITERATION_PERIOD_MS = 120000;          // Every Two Minutes	
	
            execTestData.m_IterationPeriod                   = REGISTER_TEST_ITERATION_PERIOD_MS;
       		execTestData.m_TestType                          = DiagnosticTesting::DiagnosticTest::DIAG_REGISTER_TEST_TEST_TYPE;


            // Create Register Test object.  Refer to BlackfinDiagRegistersTest.hpp and BlackfinDiagRegistersTest.cpp 
            // for a description.
            static BlackfinDiagnosticTesting::BlackfinDiagRegistersTest m_RegisterTest( execTestData ); 
 

            //***********************************************************************************************************
            //                                                                                                          *
            // Instruction RAM testing parameters, structures and definitions.                                          *
            //                                                                                                          *
            //***********************************************************************************************************
            static const UINT32 INSTRCTN_RAM_TEST_ITERATION_PERIOD_MS = 15000;          // 15 seconds for now
    
            execTestData.m_IterationPeriod                   = INSTRCTN_RAM_TEST_ITERATION_PERIOD_MS;
            execTestData.m_TestType                          = DiagnosticTesting::DiagnosticTest::DIAG_INTRUCTION_RAM_TEST_TYPE;

            // Create Instruction Ram Test object.  Refer to BlackfinInstructionRam.hpp and BlackfinInstructionRam.cpp 
            // for a description.
            static BlackfinDiagnosticTesting::BlackfinDiagInstructionRam m_InstructionRamTest( execTestData );

            //***********************************************************************************************************
            //                                                                                                          *
            // Timer testing parameters, structures and definitions.                                                    *
            //                                                                                                          *
            //***********************************************************************************************************
            static const UINT32 TIMER_TEST_ITERATION_PERIOD_MS =  10000;  // Start after 10 seconds.
            execTestData.m_IterationPeriod                   = TIMER_TEST_ITERATION_PERIOD_MS;
       		execTestData.m_TestType                          = DiagnosticTesting::DiagnosticTest::DIAG_TIMER_TEST_TYPE;

            // Create Timer Test object.  Refer to BlackfinTimerTest.hpp and BlackfinTimerTest.cpp 
            // for a description.
            static BlackfinDiagnosticTesting::BlackfinDiagTimerTest    m_TimerTest( execTestData );
					                                                 
			
            //***********************************************************************************************************
            //                                                                                                          *
            // Instructions testing parameters, structures and definitions.                                             *
            //                                                                                                          *
            //***********************************************************************************************************
            static const UINT32 INSTRUCTIONS_TEST_ITERATION_PERIOD_MS = 5000;
	
            execTestData.m_IterationPeriod                   = INSTRUCTIONS_TEST_ITERATION_PERIOD_MS;
       		execTestData.m_TestType                          = DiagnosticTesting::DiagnosticTest::DIAG_INSTRUCTIONS_TEST_TYPE;

    
            // Create Instructions Test object.  Refer to BlackfinInstructionsTest.hpp and BlackfinInstructionsTest.cpp 
            // for a description.
            static BlackfinDiagnosticTesting::BlackfinDiagInstructionsTest       m_InstructionsTest( execTestData );


            //////////////////////////////////////////////////////////////////////////////////////////////////////////
            ///
            /// Create data structures defining runtime environment for the scheduler
            ///
            //////////////////////////////////////////////////////////////////////////////////////////////////////////
            static const UINT32 CORRUPTED_DIAG_TEST_VECTOR_ERR = 1;
	
            static const UINT32 CORRUPTED_DIAG_TEST_MEMORY_ERR = 2;
        	
            static const UINT32 TEST_TOOK_TOO_LONG_ERR         = 3;
	
            static const UINT32 ALL_DIAG_DID_NOT_COMPLETE_ERR  = 4;
        	
            //
            // Requirement:  All Diagnostic Tests Complete in 4 Hours.
            //
            static const UINT32 PERIOD_FOR_ALL_DIAGNOSTICS_COMPLETED_MS     = 2 * 60 * 60 * 1000; // 2 hours for now, number of milleseconds in 4 hours
    
            static const UINT32 PERIOD_FOR_ONE_DIAGNOSTIC_TEST_ITERATION_MS = 50; // Milleseconds

            static DiagnosticTesting::DiagnosticTest * pDiagnosticTests[]    = 
                                                     {
                                                         &m_RegisterTest,
                                                         &m_DataRamTest, 
//
// When debugging other system issues, if this conditional is FALSE then the timer test and the instruction ram test will fail probably
// 
// The timer test will fail because setting breakpoints, examining memory, examining registers, etc. that are part of normal debug activity
// will affect the apex timer values read as the Apex is running asynchronously to the Blackfin system.
//
// The instuction ram test will fail if a new flash image is not written when source code changes are made.  As an aside running with the emulatr
// affects what is in instruction memory as EMU trap instructions are inserted in the program memory.  The DEBUG_BUILD conditional is used in the
// instruction RAM test.
//

#if !defined(BLACKFIN_DIAG_SYSTEM_DEBUG_ACTIVE)       
                                                         &m_TimerTest,
                                                         &m_InstructionRamTest,
#endif
                                                         &m_InstructionsTest,
                                                     };
     
            DiagnosticScheduling::DiagnosticRunTimeParameters drtp   = 
    	                                    {
    											&ReadTimestamp,
    											&ComputeElapsedTimeMS,
    											&BlackfinCrash,
    											&Apex_WatchdogKick,
    											PERIOD_FOR_ALL_DIAGNOSTICS_COMPLETED_MS,
    											PERIOD_FOR_ONE_DIAGNOSTIC_TEST_ITERATION_MS,
    											FALSE,    // m_MonitorIndividualTotalTestingTime
    											FALSE,    // m_MonitorIndividualTestIterationTimes
    											DiagnosticTesting::DiagnosticTest::DIAG_SCHEDULER_TEST_TYPE,
                                                CORRUPTED_DIAG_TEST_VECTOR_ERR,
                                                CORRUPTED_DIAG_TEST_MEMORY_ERR,
                                                TEST_TOOK_TOO_LONG_ERR,
                                                ALL_DIAG_DID_NOT_COMPLETE_ERR
    	                                    }; 
	    
            static DiagnosticScheduling::DiagnosticScheduler<DiagnosticTesting::DiagnosticTest> Schedule
                                                                              ( pDiagnosticTests, 
                                                                                sizeof( pDiagnosticTests ) 
                                                                                        / sizeof(DiagnosticTesting::DiagnosticTest *),
                                                                                drtp );
    
            pSchedule = &Schedule;
        
            Initialized = TRUE;
        }
    
    	pSchedule->RunScheduled(); 
    }

};


