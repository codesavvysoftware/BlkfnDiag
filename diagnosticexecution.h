////////////////////////////////////////////////////////////////////////////////
/// @file DiagnosticExecution.hpp
///
/// @details DiagnosticExecution class definition.
///
/// @if REVISION_HISTORY_INCLUDED
/// @par Edit History
/// -   mgrad     27-NOV-2013   Created.
/// -   pszramo   16-DEC-2013   Updated fault injection points according to
///                             Coverity defects triage.
/// -   dtstalte  04-FEB-2014   Increase CRC diagnostic step value to give
///                             wider margain of error to complete in time.
/// @endif
///
/// @par Copyright (c) 2013 Rockwell Automation Technologies, Inc. All rights reserved.
////////////////////////////////////////////////////////////////////////////////
#ifndef EXECUTEDIAGNOSTIC_HPP
#define EXECUTEDIAGNOSTIC_HPP
#include <algorithm>
#pragma once

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

class DiagnosticExecution
{
public:

	DiagnosticExecution(std::vector <BlackfinDiagTest *> * Diagnostics) : RunTimeDiagnostics(Diagnostics) {

		UINT32 ui = 0;
		
		for (std::vector<BlackfinDiagTest *>::iterator it = RunTimeDiagnostics->begin(); it != RunTimeDiagnostics->end(); ++it) {
			BlackfinDiagTest * pdt = (*it);

			pdt->triggerValueTimeslice = pdt->stepValueTimeslice + ui++;
		}
	}

	/// Default destructor.
	~DiagnosticExecution() {}

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

	// Timeslice that increments each time that runtime diagnostics run
	UINT64 m_SliceNumber;

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

	const UINT32 TRACE_LOG_SIZE = 4;
	TraceLog m_TraceLog[4]; //C++ 11 constexpr would handle this situation very nicely TRACE_LOG_SIZE];

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
	DiagnosticExecution(const DiagnosticExecution &);
	DiagnosticExecution &operator=(const DiagnosticExecution &);

	void CompletionTimeCheck()
	{
		for (std::vector<BlackfinDiagTest *>::iterator it = RunTimeDiagnostics->begin(); it != RunTimeDiagnostics->end(); ++it) {
		
			if ((m_SliceNumber - (*it)->lastCompleteTimeslice) > (*it)->timeoutTimeslice)
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
			
			if (m_SliceNumber == (*it)->triggerValueTimeslice)
			{
				pbdtNextDiag = *(&(*it));

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

		BlackfinDiagTest * pCurrentDiagTest;
			
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
		pCurrentDiagTest->triggerValueTimeslice += pCurrentDiagTest->stepValueTimeslice;

		// Record the entry time before calling diag
		UINT32 entryTimeUs = GetSystemTime();

		// Call the test function pointed to in Diag CB
		TestState testResult = (TestState)pCurrentDiagTest->RunTest();

		switch (testResult)
		{
		case TEST_LOOP_COMPLETE:
		{
			// If the test has completed all possible iterations, figure out
			// how long it's been since the last total completion.
			UINT32 timeToCompleteTimeslice = m_SliceNumber - pCurrentDiagTest->lastCompleteTimeslice;

			if ((timeToCompleteTimeslice > pCurrentDiagTest->maxTimeslices)
				&& (pCurrentDiagTest->oneComplete == true))
			{
				// Store the time if it exceeds the time already stored, and
				// the diagnostic has run at least one previous time
				pCurrentDiagTest->maxTimeslices = timeToCompleteTimeslice;
			}

			pCurrentDiagTest->oneComplete = true;
			pCurrentDiagTest->lastCompleteTimeslice = m_SliceNumber;
		}
		// (Fall through)

		case TEST_IN_PROGRESS:
		{
			// If the test has successfully run to the timelimit within the
			// timeslice, but has not completed all iterations, don't bother
			// with the maximum time calculations.  However, for debugging purposes,
			// save the amount of time that the diagnostic function took to run
			// during this timeslice into its diag CB.
			pCurrentDiagTest->durationUs = GetSystemTime() - entryTimeUs;
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

    const BOOL DEBUG_LOGGING_ENABLED = TRUE;

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

			if (pdbtCurrent->triggerValueTimeslice == pdbtPrev->triggerValueTimeslice) { 
				pdbtCurrent->triggerValueTimeslice++;
			}
		}
	}
};


#endif // #if !defined(DiagnosticExecution_HPP)
