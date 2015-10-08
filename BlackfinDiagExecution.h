#pragma once
#include <algorithm>


// FORWARD REFERENCES
// (none)

// Start Fault Injection Point 1
// Declaration of global UINT32 variable InjectFaultFlag used to delay
// the injected fault until run-time will be injected here.
// End Fault Injection Point 1

////////////////////////////////////////////////////////////////////////////////
/// @class DiagnosticExecution
///
/// @ingroup Diagnostics
///
/// @brief This class implements the main work flow for SIL-2 diagnostics.
////////////////////////////////////////////////////////////////////////////////    
#include "BlackfinDiag.h"

using namespace BlackfinDiagnosticGlobals;

class BlackfinDiagExecution
{
public:

	BlackfinDiagExecution(std::vector <BlackfinDiagTest *> * Diagnostics) : RunTimeDiagnostics(Diagnostics) {

		UINT32 ui = 0;
		
		for (std::vector<BlackfinDiagTest *>::iterator it = RunTimeDiagnostics->begin(); it != RunTimeDiagnostics->end(); ++it) {
			BlackfinDiagTest * pdt = (*it);

			pdt->dgnParams.triggerValueTimeslice = pdt->dgnParams.stepValueTimeslice + ui++;
		}
	}

	/// Default destructor.
	~BlackfinDiagExecution() {}

	/// Initializes Apex diagnostics.
	void Initialize(void) {}

	/// Runs power-up diagnostic tests.
	void PowerUp(void) {}

	/// Performs run-time tests according to the schedule.
	void RunScheduled()
	{
		// Start Fault Injection Point 2
		// Code which sets InjectFaultFlag after DGN_FI_DELAY_SEC has passed
		// to trigger an injected fault at run-time will be injected here.
		// End Fault Injection Point 2

		// Check if it's time for Io shutdown test
		if (false)//ApexDiagIoShutdown::IsTimeToRun(HI_ApexReg.SystemTime))
		{
			//ApexDiagIoShutdown::RunTest(HI_ApexReg.SystemTime);
		}
		// Then check if it's time for generic diagnostic test (unsigned math handles roll-over)
		else if ((GetSystemTime() - m_LastDiagTime) > DGN_INTERVAL_US)
		{
			// Execute a runtime diagnostic if it's time for one. Scheduling of the various tests is
			// handled within ExecuteSlice(), and is guided by the m_LastDiagTime variable, which
			// increments each time through.
			m_LastDiagTime = GetSystemTime();
			ExecuteSlice();
		}
		// Finally check if it's time to run diagnostic completion time check.
		else if ((m_SliceNumber - m_LastCompletionCheck) > DGN_COMPL_CHECK_INTERVAL_TIME_SLICE)
		{
			m_LastCompletionCheck = m_SliceNumber;
			CompletionTimeCheck();
		}
		else
		{
			// nothing to do
		}
	}


private:

//	friend class ApexWatchdog;      // for access to LogDgnTrace
//	friend class ApexDiagTimer;     // for access to DGN_TIMER_STEP_MS

									/// Default constructor.
	struct TraceLog
	{
		UINT32 timestamp;
		UINT32 value;
	};
	
	std::vector <BlackfinDiagTest *> * RunTimeDiagnostics;

	static const UINT32 TRACE_LOG_SIZE = 4;
	TraceLog m_TraceLog[4]; //C++ 11 constexpr would handle this situation very nicely TRACE_LOG_SIZE];

							// Timeslice that increments each time that runtime diagnostics run
	UINT32 m_SliceNumber;
	/// Last time a diagnostic test was run.
	UINT32  m_LastDiagTime;
	
	/// Last time diagnostic completion time
	UINT64  m_LastCompletionCheck;

	
	UINT32 DgnTrace[32];

	// Start Fault Injection Point 2
	// Declaration of DGN_FI_DELAY_SEC constant which determines
	// when InjectFaultFlag is set will be injected here.
	// End Fault Injection Point 2
	/// Copy constructor and assignment operator not implemented.
	BlackfinDiagExecution(const BlackfinDiagExecution &);
	BlackfinDiagExecution &operator=(const BlackfinDiagExecution &);

	void CompletionTimeCheck()
	{
		for (std::vector<BlackfinDiagTest *>::iterator it = RunTimeDiagnostics->begin(); it != RunTimeDiagnostics->end(); ++it) {
		
			if ((m_SliceNumber - (*it)->dgnParams.lastCompleteTimeslice) > (*it)->dgnParams.timeoutTimeslice)
			{
				firmExcept();
			}
		}

		if (false)//!ApexDiagIoShutdown::CheckCompletionTime())
		{
			firmExcept();
		}
		return;
	}

	BOOL FindTestIndexForThisSlice(BlackfinDiagTest * & pbdtNextDiag)
	{
		BOOL bSuccess = FALSE;

		for (std::vector<BlackfinDiagTest *>::iterator it = RunTimeDiagnostics->begin(); it != RunTimeDiagnostics->end(); ++it) {
			BlackfinDiagTest * pdt = (*it);
			
			if (m_SliceNumber == pdt->dgnParams.triggerValueTimeslice)
			{
				pbdtNextDiag = pdt;

				bSuccess = TRUE;

				break;
			}
		}
		return bSuccess;
	}

	void ExecuteSlice()
	{
		//LogDgnTrace(0xbbbbbbbb);

		// Do hardware watchdog handing
		//ApexWatchdog::Service();

		BlackfinDiagTest * pCurrentDiagTest = NULL;
			
		BOOL bNotTimeToRunADiagnostic = !FindTestIndexForThisSlice(pCurrentDiagTest);

		if (bNotTimeToRunADiagnostic) {
			// Update diag slice number.
			m_SliceNumber++;

			return;
		}
		
		//LogDgnTrace(testIndex);

																   // Schedule next trigger slice for this diag. UINT math handles
																   // rollover. In some cases, this trigger value will be overwritten
																   // from within the test function that is called.  That's why we do the
																   // reschedule before we enter the function.
		pCurrentDiagTest->dgnParams.triggerValueTimeslice += pCurrentDiagTest->dgnParams.stepValueTimeslice;

		// Record the entry time before calling diag
		UINT32 entryTimeUs = GetSystemTime();

		// Call the test function pointed to in Diag CB
		TestState testResult = pCurrentDiagTest->RunTest();

		switch (testResult)
		{
		case TEST_LOOP_COMPLETE:
		{
			// If the test has completed all possible iterations, figure out
			// how long it's been since the last total completion.
			UINT32 timeToCompleteTimeslice = m_SliceNumber - pCurrentDiagTest->dgnParams.lastCompleteTimeslice;

			if ((timeToCompleteTimeslice > pCurrentDiagTest->dgnParams.maxTimeslices)
				&& (pCurrentDiagTest->dgnParams.oneComplete == true))
			{
				// Store the time if it exceeds the time already stored, and
				// the diagnostic has run at least one previous time
				pCurrentDiagTest->dgnParams.maxTimeslices = timeToCompleteTimeslice;
			}

			pCurrentDiagTest->dgnParams.oneComplete = true;
			pCurrentDiagTest->dgnParams.lastCompleteTimeslice = m_SliceNumber;
		}
		// (Fall through)

		case TEST_IN_PROGRESS:
		{
			// If the test has successfully run to the timelimit within the
			// timeslice, but has not completed all iterations, don't bother
			// with the maximum time calculations.  However, for debugging purposes,
			// save the amount of time that the diagnostic function took to run
			// during this timeslice into its diag CB.
			pCurrentDiagTest->dgnParams.durationUs = GetSystemTime() - entryTimeUs;
			break;
		}

		default:
		{
			// Call HW_HandleException for any other responses
			firmExcept();
			break;
		}
		};

		RescheduleLowerPriorityDiagnostics();

		//LogDgnTrace(0xeeeeeeee);

		// Update diag slice number.
		m_SliceNumber++;
	}

	void SchedulerInit(void) {
		m_SliceNumber = 0;
		m_LastCompletionCheck = 0;
	}

    static  const BOOL DEBUG_LOGGING_ENABLED = TRUE;

	void LogDgnTrace(const UINT32 val) {
		static UINT32 index = 0;

		if (DEBUG_LOGGING_ENABLED)
		{
			if (index > (UINT32)((sizeof(DgnTrace) / 4) - 4))
			{
				index = 0;
			}
			DgnTrace[index++] = GetSystemTime();
			DgnTrace[index++] = val;
		}
		else
		{
			if (index >= TRACE_LOG_SIZE)
			{
				index = 0;
			}

			m_TraceLog[index].timestamp = GetSystemTime();
			m_TraceLog[index].value = val;
			index++;
		}
	}


	void RescheduleLowerPriorityDiagnostics()
	{
		BlackfinDiagTest * pdbtPrev = NULL;

		BlackfinDiagTest * pdbtCurrent = NULL;

		for (std::vector<BlackfinDiagTest *>::iterator it = (RunTimeDiagnostics->begin() + 1); it != RunTimeDiagnostics->end(); ++it) {
			pdbtPrev = *(&(*(it-1)));

			pdbtCurrent = *(&(*it));

			if (pdbtCurrent->dgnParams.triggerValueTimeslice == pdbtPrev->dgnParams.triggerValueTimeslice) { 
				pdbtCurrent->dgnParams.triggerValueTimeslice++;
			}
		}
	}
};



