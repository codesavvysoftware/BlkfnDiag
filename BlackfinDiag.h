#ifndef BLACKFIN_DIAG_H
#define BLACKFIN_DIAG_H
#include "BlackfinParameters.h"
#include "Defs.h"
#include <time.h>
#include <vector>

namespace BlackfinDiagnosticGlobals {
	const UINT32 DGN_INTERVAL_US = 50000;      // Resolution of microseconds


	enum TestState
	{
		TEST_LOOP_COMPLETE = 0,
		TEST_IN_PROGRESS = 1
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
		// Pointer to the relevant diagnostic function
		TestState(*diagFunc) (DiagControlBlock *diagControlBlock);
		UINT64 triggerValueTimeslice;       // Timeslice number for next trigger
		UINT32 stepValueTimeslice;          // Number of diag timeslices between activations
		UINT64 lastCompleteTimeslice;       // Timeslice number of the previous test completion
		UINT32 durationUs;                  // Time spent in diag function
		UINT32 maxTimeslices;               // Max number of timeslices ever seen between triggers
		bool   oneComplete;                 // True if diag has completed at least once. This is used to decide
											//      whether or not maxTimeslices is valid, since  maxTimeslices
											//      should indicate a completion-to-completion time
		UINT32 timeoutTimeslice;            // Max number of timeslices that test
											// can wait to be completed in

		UINT32 priorityRelative;            // Relative prioirty to other diagnostic tests. lower number == higher priority so a priority of 0 is highest priority.

		BOOL operator<(const DiagControlBlock & val) const {
			return (priorityRelative < val.priorityRelative);
		}
	};
	typedef struct DiagControlBlock DGN_CTL_BLK;


	class BlackfinDiagTest {
	public:
		BlackfinDiagTest( UINT64 TrgrValTmeSlce,
			              UINT32 StpValTmeSlce,
			              UINT64 LstCmplteTmeslce,
			              UINT32 DrtnUS,
			              UINT32 MxTmeSlce,
			              BOOL   OneCmplte,
			              UINT32 TmeoutTmeslce,
			              UINT32 PrtyRltve,
			              UINT32 mxdrtn,
			              UINT32 mndrtn ) : triggerValueTimeslice(TrgrValTmeSlce),
			                                stepValueTimeslice(MsToDgnSlices(StpValTmeSlce)),
				                            lastCompleteTimeslice(LstCmplteTmeslce),
				                            durationUs(DrtnUS),
				                            maxTimeslices(MxTmeSlce),
				                            oneComplete(FALSE),
				                            timeoutTimeslice(OneCmplte),
				                            maxduration(mxdrtn),
			                                minduration(mndrtn)
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
			static const UINT32 DGN_INTERVAL_US = 50000;      // Resolution of microseconds

			if ((value / (DGN_INTERVAL_US / 1000)) >= 1)
			{
				return value / (DGN_INTERVAL_US / 1000);
			}
			else
			{
				return 1;
			}
		}
		virtual TestState RunTest() = 0;

		UINT32 triggerValueTimeslice;       // Timeslice number for next trigger
		UINT32 stepValueTimeslice;          // Number of diag timeslices between activations
		UINT32 lastCompleteTimeslice;       // Timeslice number of the previous test completion
		UINT32 durationUs;                  // Time spent in diag function
		UINT32 maxTimeslices;               // Max number of timeslices ever seen between triggers
		BOOL   oneComplete;                 // True if diag has completed at least once. This is used to decide
											//      whether or not maxTimeslices is valid, since  maxTimeslices
											//      should indicate a completion-to-completion time
		UINT32 timeoutTimeslice;            // Max number of timeslices that test
											// can wait to be completed in
//#ifdef HI_APEX_DIAGNOSTICS
		UINT32 maxduration;                 // Max time spent in diag function
		UINT32 minduration;                 // Min time spent in diag function
//#endif		

	private:
		BlackfinDiagTest() {}
		BlackfinDiagTest(const BlackfinDiagTest & other) {}

		const BlackfinDiagTest & operator = (const BlackfinDiagTest &);
	};



    
	std::vector <DGN_CTL_BLK> DiagRuntimeTests;

	std::vector <BlackfinDiagTest *> DiagRuntimeObs;

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
			return (clock() * 1000);
		}
    }      
}


#endif
