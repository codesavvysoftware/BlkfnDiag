#pragma once

#include "BlackfinParameters.h"
#include "Defs.h"
#include <time.h>
#include <vector>

namespace BlackfinDiagnosticGlobals {
	const UINT32 DGN_INTERVAL_US = 50000;      // Resolution of microseconds

	const UINT32 DiagnosticSlicePeriod_Microseconds = 50000;  // Resolution of microseconds

	const UINT32 DiagnosticSlicePeriod_Milleseconds = DiagnosticSlicePeriod_Microseconds / 1000;
	
	const UINT8 * const pRAMDataStart = reinterpret_cast<UINT8 *>(0xff900000);

	const UINT8 TestPatternsForRamTesting[] = { 0xff,0, 0x55, 0xaa, 0xf, 0xf0, 0xa0, 0xa, 0x50, 0x5, 0x5a, 0xa5 };

	const UINT32 NumberOfTestingPatterns = sizeof(TestPatternsForRamTesting) / sizeof(UINT8);

    //
    // Requirement:  All Diagnostic Tests Complete in 4 Hours.
    //
    static const UINT32 MaxCyclePeriodAllTestsHoursComponent        = 4;
    static const UINT32 MaxCyclePeriodAllTestsMinutesComponent      = 0;
    static const UINT32 MaxCyclePeriodAllTestsSecondsComponent      = 0;
    static const UINT32 MaxCyclePeriodAllTestsMillesecondsComponent = 0;
    static const UINT32 MillesecondsInAnHour = 3600000;  //60 minutes/per hour * 60 seconds per minute * 1000 milleseconds per second

    static const UINT32 dcbSliceTriggerVal_Default                  = 0;
    static const UINT32 dcbSliceStepVal_Default                     = 1;
    static const UINT32 dcbSliceLastComplete_Default                = 0;
    static const UINT32 dcbTestDurationInSlices_Default             = 0;	
    static const UINT32 dcbTestExecutionMaxSlices_Default           = 0;
    static const BOOL   dcbTestComplete_Default                     = FALSE;
    static const UINT32 dcbTestExecutionTimeoutPeriodSlices_Default = (  ( MaxCyclePeriodAllTestsHoursComponent * MillesecondsInAnHour )
                                                                         + ( MaxCyclePeriodAllTestsMinutesComponent * 60 * 1000 )
                                                                         + ( MaxCyclePeriodAllTestsSecondsComponent * 100 )
                                                                         + ( MaxCyclePeriodAllTestsMillesecondsComponent ))
                                                                         /DiagnosticSlicePeriod_Milleseconds;
 
	enum TestState
	{
		TEST_LOOP_COMPLETE = 0,
		TEST_IN_PROGRESS = 1,
		TEST_FAILURE = 2
	};

 /*   enum RuntimeTestList
    {
        DGN_SAFE_RAM_NUM,            // Safe RAM data test
        DGN_INSTRUCTION_RAM,
        DGN_TIMER,                   // Timer diagnostic
        DGN_EXE_CRC_NUM,             // Executable RAM checksum
        DGN_RAM_DATA_NUM,            // RAM data test
        DGN_NUM_RUNTIME_TESTS        // Total entries in runtime test list
    }; */
	struct DiagControlBlock
	{
		UINT32 triggerValueTimeslice;       // Timeslice number for next trigger
		UINT32 stepValueTimeslice;          // Number of diag timeslices between activations
		UINT32 lastCompleteTimeslice;       // Timeslice number of the previous test completion
		UINT32 durationUs;                  // Time spent in diag function
		UINT32 maxTimeslices;               // Max number of timeslices ever seen between triggers
		bool   oneComplete;                 // True if diag has completed at least once. This is used to decide
											//      whether or not maxTimeslices is valid, since  maxTimeslices
											//      should indicate a completion-to-completion time
		UINT32 timeoutTimeslice;            // Max number of timeslices that test
											// can wait to be completed in
		
		DiagControlBlock( UINT32 TriggerValueTimeslice, 
		                  UINT32 StepValue, 
		                  UINT32 LastComplete, 
		                  UINT32 Duration, 
		                  UINT32 MaxSlices,
		                  BOOL   OneComplete,
		                  UINT32 TimeoutSlice ) : triggerValueTimeslice(TriggerValueTimeslice),
		                                          stepValueTimeslice(StepValue),
		                                          lastCompleteTimeslice(LastComplete),
		                                          durationUs(Duration),
		                                          maxTimeslices(MaxSlices),
		                                          oneComplete(OneComplete),
		                                          timeoutTimeslice(TimeoutSlice) {}
		DiagControlBlock(){}
	          
	};
	typedef struct DiagControlBlock DGN_CTL_BLK;

	template<typename T, size_t N>
	T * end(T (&ra)[N]) {
		return ra + N;
	}
	
	class BlackfinDiagTest {
	public:
		//BlackfinDiagTest( DGN_CTL_BLK dgn ) : dgnParams(dgn){}
		
		BlackfinDiagTest( UINT32  PeriodBetweenIterations_Milleseconds, 
		                  UINT32  MaxTestDuration_Milleseconds 
		                            = (dcbTestExecutionTimeoutPeriodSlices_Default * DiagnosticSlicePeriod_Milleseconds) ) 
		: dgnParams( dcbSliceTriggerVal_Default, 
		             (PeriodBetweenIterations_Milleseconds/DiagnosticSlicePeriod_Milleseconds),
		             dcbSliceLastComplete_Default,
		             dcbTestDurationInSlices_Default,
		             dcbTestExecutionMaxSlices_Default,
                     dcbTestComplete_Default,
                     MaxTestDuration_Milleseconds / DiagnosticSlicePeriod_Milleseconds ) // Seems crazy to do this,
                                                                                        // Probably will be constant for all tests
		{}
		
			
		virtual ~BlackfinDiagTest()  {}

		/// Check if test are completing on time.
		// DiagnosticExecution::MsToDgnSlices
		//
		/// Function that converts the desired number of milliseconds between
		/// scheduled diagnostics to the proper integer number of diagnostic
		/// timeslices.  If zero, then set to one as a minimum.
		/////////////////////////////////////////////////////////////////////////////
		UINT32 MsToDgnSlices(UINT32 value)
		{
			return ((value > BlackfinDiagnosticGlobals::DiagnosticSlicePeriod_Milleseconds) 
						? value/BlackfinDiagnosticGlobals::DiagnosticSlicePeriod_Milleseconds : 1);
			
		}
		
		virtual TestState RunTest() = 0;

		DGN_CTL_BLK  dgnParams;

	private:
		BlackfinDiagTest() {}
		BlackfinDiagTest(const BlackfinDiagTest & other) {}

		const BlackfinDiagTest & operator = (const BlackfinDiagTest &);
	};


    #define firmExcept();//(__FILE__, __LINE__)

	const UINT32 DGN_CODE_SPACE_START            = 0x20000;    // Code RAM begin addr

	const UINT32 DGN_CONVERSION_FACTOR_CYCLES_uS = CLOCKS_PER_SEC / 1000000;
    
    const UINT32 DGN_EXT_SHR_RAM_START           = 0xc000000;  // External shared RAM begin addr
    
    const UINT32 DGN_EXT_SHR_RAM_END             = 0xc080000;  // External shared RAM end+1

    const UINT32 DGN_INT_SHR_RAM_START           = 0x2f000;    // Internal shared RAM begin addr

    const UINT32 DGN_INT_SHR_RAM_END             = 0x30000;    // Internal shared RAM end+1

	const UINT32 DGN_INTERVALS_PER_MINUTE       = 12000;      // (60 * 1000000) / 50000;

    const UINT32 SLICE_TIME_LIMIT_US             = 10;         // Max allowed time (us)

	// Number of timeslices between diagnostics completion time checks
	// Start Fault Injection Point 3
	// Declaration of DGN_COMPL_CHECK_INTERVAL_TIME_SLICE constant with a smaller value to make
	// completion time diagnostic injected fault happen faster will be injected here.
	const UINT32 DGN_COMPL_CHECK_INTERVAL_TIME_SLICE = 15 * DGN_INTERVALS_PER_MINUTE;
	// End Fault Injection Point 3

    

    static UINT32 GetSystemTime() {
		if (DGN_CONVERSION_FACTOR_CYCLES_uS) {
			return (clock() / DGN_CONVERSION_FACTOR_CYCLES_uS);
		}
		else {
			return clock();
		}
    }      
};



