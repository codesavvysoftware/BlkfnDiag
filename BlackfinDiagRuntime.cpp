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
    //*
    //* The compiler generates much more effecient code both memory and execution speed wise by using constants here.
    //* Originally I had a class that would compute the divisor and adjShift values based on the CLOCKS_PER_SEC 
    //* constant during construction.  Here is how the algorithm works:
    //* Find the most significant bit of the CLOCKS_PER_SECOND rate.  The divisor is most significant bit minus 1.
    //* then use a known value like 1000 milleseconds per second to determine the closest adjustment factor. The 
    //* algoriothm below works well.  I don't use it because a lot of code is generated for the Blackfin as a result.
    //* Just computing the constants by hand and using them results in much better code but is not as generic of
    //* a solution.  It is much better than doing divides though.
    //* 
    //*     void CalcShiftFactors( UINT32 & rShiftFactor, UINT32 & rShiftAdjustment ) 
    //* 		{
    //* 	    //
    //* 	    // Find MSB of ClocksPerMillesecond
	//*	        //
	//* 	    UINT64 dtt = (CLOCKS_PER_SEC / 1000 );
    //* 
    //* 	    UINT32 bitpos = 0;
    //*
    //* 	    while (dtt != 0 ) 
    //* 		{
    //* 		    ++bitpos;
    //*
    //* 		    dtt >>= 1;
    //* 	    }
    //*
    //* 	    // Scale to the seoond
    //* 	    dtt = CLOCKS_PER_SEC;
    //* 
    //* 	    dtt >>= bitpos-1;
    //* 
    //* 	    UINT32 i = 1;
    //* 
    //* 	    UINT64 error = 0;
    //* 
    //* 	    //
    //*         // Determine the best adjustment shift factor that yields the closest value to one second.
    //*         //
    //*         UINT64 prev_error = 0;
    //* 
    //* 	    for ( ; i < bitpos - 1; ++i ) 
	//* 		{    	
    //* 		    error = ( dtt - (dtt >> i ) );	
    //*
    //*             //
    //*             // dtt is scaled to the second but we know it will be greater than one second because we've divided
    //*             // by a value less than CLOCKS_PER_SECOND.  When we start off with i == 1 we're dividing by one half.
    //*             // and subtracting that from the CLOCKS_PER_SECOND scaled to the second approximation.  That number
    //*             // almost always will be less than 1000 milleseconds or a second.  We want to find the value of i 
    //*             // where error is greater than 1 second.  Then by keeping track of the pervious error we know
    //*             // the value of i where the threshold is crossed between less than a second and greater than a 
    //*             // second for the adjmustment shift factor.  Then we pick the value that yields the lowest error IE
    //*             // the value that comes closes to approximating one second.
    //*             if ( error > 1000 )	break;
    //*      
    //*             prev_error = error;
    //*  	    }
    //* 
    //* 	    //
    //* 	    // Pick the adjustment that yiels the closest value to 1000 milleseconds/ 1 second
    //* 	    //
    //* 
    //* 	    UINT64 lower = 1000 - prev_error;
    //* 
    //* 	    UINT64 upper = error - 1000;
    //* 
    //*		    if ( upper < lower ) 
	//* 		{
    //* 		    rShiftAdjustment = i;
   	//* 	    }
   	//* 	    else 
	//* 		{
   	//* 		    rShiftAdjustment = i-1;
   	//* 	    }
    //* 
   	//* 	    rShiftFactor = bitpos -1;   
	//*     }    
        
        
    #define divisor 19
    #define adjShift 3
        
    static UINT32 ComputeElapsedTimeMS( UINT64 current, UINT64 previous ) 
	{
        UINT64 diff       = current - previous; // difference in clock cycles;
	
        // An approximation that is actually very close when CLOCKS_PER_SEC == 600000000
        // avoiding a constant divide in the background
		//
		// Math for the approximation
		//  CLOCKS_PER_SEC == 600000000
		//  CLOCKS_PER_MILLESECOND = CLOCKS_PER_SEC * SECONDS_PER_MILLESECOND = 600000000 / 1000 = 600000
		//  Elapsed time in milleseconds = difference in clock readings / CLOCKS_PER_MILLESECOND = diff / 600000;
		//  600000 == 0x927c0
		//  We're looking for a shift that is less which would be 0x80000 ==  524288.
		//  600000 ~= 524288 * 1.14441
		//  Elapsed time in millesconds ~= diff / (524288 * 1.1441) ~=  ( diff / 0x8000 ) * (1/1.1441) 
		//                                                          ~=  ( diff >> 19 ) * (.8738 )
		//                                                          ~=  ( diff >> 19 ) * ( 7/8 )
		//                                                          ~=  ( diff >> 19 ) ( 1 - 1/8 )
		//                                                          ~=  ( diff >> 19 ) - ( ( diff >> 19 ) * 1/8)
		//                                                          ~=  ( diff >> 19 ) - ( ( diff >> 19 ) >> 3 );
		//                                                        substitute  fast for ( diff >> 19 );
	    UINT64   fast = diff >> divisor;
			    
        fast -= (fast >> adjShift );

		return fast;  // difference in clock cycles times milleseconds per clock cycle
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
    #define DFLT_INITIAL_TIMESTAMP                0 
    #define DFLT_INITIAL_ELAPSED_TIME             0           
    #define DFLT_NBR_TIMES_TO_RUN_PER_DIAG_CYCLE  1 
    #define DFLT_NBR_TIMES_RAN_THIS_DIAG_CYCLE    1
    #define DFLT_INITIAL_TEST_EXECUTION_STATE     DiagnosticTesting::DiagnosticTest::TEST_IDLE


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

            #define NMBR_DATA_RAM_BYTES_TESTED_PER_ITERATION  0x400 // Test 1k at a time for now
            #define DATA_RAM_TEST_ITERATION_PERIOD_MS         1000 // 1 second for now

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
                                                                    DFLT_INITIAL_TEST_EXECUTION_STATE
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
            #define  REGISTER_TEST_ITERATION_PERIOD_MS 2000          // Every Two Seconds	
	
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
            #define INSTRCTN_RAM_TEST_ITERATION_PERIOD_MS 2000          // 2 second for now
    
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
            #define TIMER_TEST_ITERATION_PERIOD_MS   1000  // Every second.
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
            #define INSTRUCTIONS_TEST_ITERATION_PERIOD_MS 5000
	
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
            #define CORRUPTED_DIAG_TEST_VECTOR_ERR  1
	
            #define CORRUPTED_DIAG_TEST_MEMORY_ERR 2
        	
            #define TEST_TOOK_TOO_LONG_ERR         3
	
            #define ALL_DIAG_DID_NOT_COMPLETE_ERR  4
        	
            //
            // Requirement:  All Diagnostic Tests Complete in 4 Hours.
            //
            #define PERIOD_FOR_ALL_DIAGNOSTICS_COMPLETED_MS     4 * 60 * 60 * 1000 // 4 hours, number of milleseconds in 4 hours
    
            #define PERIOD_FOR_ONE_DIAGNOSTIC_TEST_ITERATION_MS 50 // Milleseconds

            static DiagnosticTesting::DiagnosticTest * pDiagnosticTests[]    = 
                                                     {
                                                         //&m_RegisterTest,
                                                         //&m_DataRamTest, 
                                                         //&m_TimerTest,
                                                         //&m_InstructionRamTest,
                                                         &m_InstructionsTest,
                                                     };
     
            DiagnosticScheduling::DiagnosticRunTimeParameters drtp   = 
    	                                    {
    											&ReadTimestamp,
    											&ComputeElapsedTimeMS,
    											&BlackfinCrash,
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


