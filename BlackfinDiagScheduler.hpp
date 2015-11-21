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
#include <time.h>
#include "Defs.h"

namespace DiagnosticScheduling
{
    typedef struct 
    {
        UINT64                                                 (*m_SysTimestamp)();
            	
        UINT32                                                  (*m_CalcElapsedTime)(UINT64 current, UINT64 previous);
            	
        void                                                    (*m_ExcetionError)(INT errorCode);
            	
        UINT32                                                  m_PeriodForAllDiagnosticsToCompleteInMS;
            	
        UINT32                                                  m_PeriodForOneDiagnosticIteration;
            	
        BOOL                                                    m_MonitorIndividualTotalTestingTime;
            	
        BOOL                                                    m_MonitorIndividualTestIterationTimes;
            	
        UINT32                                                  m_SchedulerTestType;
            	
        UINT32                                                  m_ErrorTypeMask;
            	
        UINT32                                                  m_ErrorTypeBitPos;
            	
        UINT32                                                  m_CorruptedVectorErr;
                                                 
        UINT32                                                  m_CorruptedTestMemoryErr;
            	
        UINT32                                                  m_TestTookTooLongErr;
                                                 
        UINT32                                                  m_AllDiagnosticsNotCompletedErr;

    } DiagnosticRunTimeParameters;
    
    template <typename T >
    class DiagnosticScheduler
    {
        public:

            DiagnosticScheduler( T **                          ppDiagnostics,
                                 UINT32                        numberOfDiagnosticTests,
                                 DiagnosticRunTimeParameters  runTimeData );

        	/// Initializes diagnostics.
        	void Initialize(void) 
        	{
        	}

        	/// Runs power-up diagnostic tests.
        	void PowerUp(void) 
        	{
        	}
	
        	/// Performs run-time tests according to the schedule.
        	void RunScheduled();


        private:

            static const UINT64  DEFAULT_INITIAL_TIMESTAMP = 0;	

            typedef enum 
        	{
        		INITIAL_INSTANTIATION,
        		MAX_PERIOD_EXPIRED_ALL_TESTS_COMPLETE,
        		MAX_PERIOD_EXPIRED_INCOMPLETE_TESTING,
        		NO_NEW_SCHEDULING_PERIOD,
        		NO_TESTS_TO_RUN_ALL_COMPLETED,
        		NO_TEST_ITERATIONS_SCHEDULED,
        		TEST_ITERATIONS_SCHEDULED,
        	} 
        	SchedulerStates;
	
        	SchedulerStates                 m_CurrentSchedulerState;
	
        	T **                            m_ppRunTimeDiagnostics;
        	
        	UINT32                          m_NumberOfDiagTests;
	
        	T **                            m_ppTestEnumeration; 
		
            DiagnosticRunTimeParameters     m_RuntimeData;
            
       	    // Number of timeslices between diagnostics completion time checks
        	// Start Fault Injection Point 3
        	// Declaration of DGN_COMPL_CHECK_INTERVAL_TIME_SLICE constant with a smaller value to make
        	// completion time diagnostic injected fault happen faster will be injected here.
        	//	static const DiagSlices_t DGN_COMPL_CHECK_INTERVAL_TIME_SLICE = 15 * DGN_INTERVALS_PER_MINUTE;

        	UINT64 m_TimestampCurrent;	
	
            UINT64 m_TimeTestCycleStarted;
    
            UINT64 m_TimeLastIterationPeriodExpired;

            DiagnosticScheduler();
	
        	DiagnosticScheduler(const  DiagnosticScheduler &);

        	DiagnosticScheduler &operator=(const DiagnosticScheduler &);

        	void DetermineCurrentSchedulerState();

        	void ConfigureErrorCode( UINT32 & returnedErrorCode, UINT32 testTypeCurrent );

        	void DoMoreDiagnosticTesting();

        	BOOL IsTestScheduledToRun( T * & rpPbdt );

            BOOL IsTestingCompleteForDiagCycle( T * & rpPbdt );
            
//            DiagnosticScheduler(const DiagnosticScheduler &);
	
//	        const DiagnosticScheduler & operator = (const DiagnosticScheduler & );
		
//            DiagnosticScheduler();
            
//            ~DiagnosticScheduler();

	
    };
};



