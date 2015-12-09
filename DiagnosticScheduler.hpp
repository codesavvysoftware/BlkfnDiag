// #pragma once
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @file DiagnosticScheduler.hpp
///
/// Namespace for scheduling and monitoring diagnostic tests.
///
/// @par Full Description
/// 
/// Contains the namespace for Diagnostic scheduling.  The namespace includes the definition of the parameters         
/// that comprise the runtime environment for diagnostics and the templace class definition for the diagnostic scheduler;
///
/// @if REVISION_HISTORY_INCLUDED
/// @par Edit History
/// - [0]  thaley1  01-Dec-2015 Initial revision of file.
/// @endif
///
/// @ingroup Diagnostics
///
/// @par Copyright (c) 2015 Rockwell Automation Technologies, Inc.  All rights reserved.
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#if !defined(DIAGNOSTIC_SCHEDULER_HPP)
#define DIAGNOSTIC_SCHEDULER_HPP

// SYSTEM INCLUDES
#include <time.h>

// C PROJECT INCLUDES
#include "Defs.h"
 
// C++ PROJECT INCLUDES
// (none)

// FORWARD REFERENCES
// (none)


//
// Start Fault Injection Point 1
// Declaration of global UINT32 variable InjectFaultFlag used to delay
// the injected fault until run-time will be injected here.
// End Fault Injection Point 1


namespace DiagnosticScheduling
{
    typedef struct 
    {
        // Gererate a timestamp that has the resolution of timer ticks.
        UINT64                                                 (*m_SysTimestamp)();
            	
        // Calculate the elapsed time in milleseconds between timer tick readings;
        UINT32                                                  (*m_CalcElapsedTime)(UINT64 current, UINT64 previous);
            	
        // When diagnostics detect errors, call this function to report the error.
        void                                                    (*m_ExceptionError)(INT errorCode);
        
        // For "kicking" the watchdog timer
        void                                                    (*m_KickWatchdogTimer)();
            	
        // All diagnostics must complete within this time period.
        UINT32                                                  m_PeriodForAllDiagnosticsToCompleteInMS;
            	
        // Time period for one iteration of diagnostic testing.
        UINT32                                                  m_PeriodForOneDiagnosticIteration;
            	
        // TRUE when monitorint the time it takes for individual tests to run..
        BOOL                                                    m_MonitorIndividualTotalTestingTime;
            	
        // TRUE when monitoring the time for an individual test iteration
        BOOL                                                    m_MonitorIndividualTestIterationTimes;
            	
        // Type of test that is associated with the scheduler.  Used to configure error codes for errors in diagnotics
        // the scheduler detects;
        UINT32                                                  m_SchedulerTestType;
            	
        // Error number associated with a corrupted pointer to a test to run.
        UINT32                                                  m_CorruptedVectorErr;
                                                 
        // Corrupted data for running a test error number.
        UINT32                                                  m_CorruptedTestMemoryErr;
            	
        // Error number for reporting when a test is taking too long to complete.
        UINT32                                                  m_TestTookTooLongErr;
                                                 
        // Error number for reporting when all tests are taking too long to complete.
        UINT32                                                  m_AllDiagnosticsNotCompletedErr;

    } 
    DiagnosticRunTimeParameters;
    
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// CLASS NAME: DiagnosticScheduler
    ///
    /// Template class for instantiating a DiagnosticScheduler class.
    ///
    /// @par Full Description
    ///
    /// Instantiate this template with a type the fulfills the interface defined for the generic typename.  The intent 
    /// is to provide a reusable component that implements a common set of diagnostic requirements across multiple 
    /// platforms.  The idea here is to define a base class for the test type (Blackfin or Apex currently), derive 
    /// individual tests like data ram, timing, instructions, etc. from the base class.  Create a runtime environoment 
    /// for the platform being tested and pass the runtime environment to the instantiated scheduler class in order 
    /// to schedule and monitor diagnostic testing.  
    ///
    /// Diagnostics must complete in a time period, for Blackfin it is currently 4 hours.  For Apex it is 8 hours.  
    /// In the code the term diagnostic test cycle and/or diagnostic test period are used to refer to this overall time 
    /// period.  Within this diagnostic cycle individual diagnostic tests are run until they all complete.  Determining 
    /// whether or not tests chould be run and is done on a periodic basis.  The time resolution of this period is in
    /// microseconds so there are literally millions of these periods within a diagnostic test cycle.  These periods are 
    /// referred to as iteration periods in the code since many tests run in iterations that are scheduled.
    ///
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    template <typename T >
    class DiagnosticScheduler
    {
        public:

            //***************************************************************************
            // PUBLIC METHODS
            //***************************************************************************
            
            ///////////////////////////////////////////////////////////////////////////////////////////////////////////
            ///	METHOD NAME: DiagnositcScheduler: DiagnosticScheduler
            ///
            /// @par Full Description
            ///      For instantiating a scheduler class for a system.  This the one and only method for instantiating 
            ///      the scheduler.
            ///      
            ///
            /// @param ppDiagnostics                Diagnostic pointer array to tests to run.
            ///
            ///        numberOfDiagnosticTests:     Number of diagnostic tests in total.
            ///                                     
            ///        DiagnosticRunTimeParameters: Linkage to system runtime parameters scheduler needs to run tests in.
            ///                               
            /// @return                             Scheduler is configured.
            ///
            ///////////////////////////////////////////////////////////////////////////////////////////////////////////
            DiagnosticScheduler( T **                          ppDiagnostics,
                                 UINT32                        numberOfDiagnosticTests,
                                 DiagnosticRunTimeParameters  runTimeData );

            //////////////////////////////////////////////////////////////////////////////////////////////////////////
            //	METHOD NAME: DiagnositcScheduler: PowerUp
            //
            /// @par Full Description
            ///      For doing testing at system power up time.
            ///      
            ///
            /// @param                              None
            ///                               
            /// @return                             Power up testing executed.
            ///
            /////////////////////////////////////////////////////////////////////////////////////////////////////////
        	void PowerUp(void) 
        	{
        	}
	
            ////////////////////////////////////////////////////////////////////////////////////////////////////////
            ///	METHOD NAME: DiagnositcScheduler: RunScheduled
            ///
            /// @par Full Description
            ///      For determining when and what diagnostics are run.
            ///      
            ///
            /// @param                              None.              
            ///                               
            /// @return                             Performs run-time tests according to the schedule defined by the 
            ///                                     runtime input to the scheduler.  Scheduler runs diagnostics when 
            ///                                     they are supposed to be run.
            ///
            //////////////////////////////////////////////////////////////////////////////////////////////////////// 
        	void RunScheduled();


        private:

            static const UINT64  DEFAULT_INITIAL_TIMESTAMP = 0;	

            typedef enum 
        	{
        		MAX_PERIOD_EXPIRED_ALL_TESTS_COMPLETE,
        		MAX_PERIOD_EXPIRED_INCOMPLETE_TESTING,
        		NO_NEW_SCHEDULING_PERIOD,
        		NO_TESTS_TO_RUN_ALL_COMPLETED,
        		NO_TEST_ITERATIONS_SCHEDULED,
        		TEST_ITERATIONS_SCHEDULED,
        	} 
        	SchedulerStates;
	
        	// Current statue the scheduler is in.
        	SchedulerStates                 m_CurrentSchedulerState;
	
        	// Array of pointers to diagnostic tests
        	T **                            m_ppRunTimeDiagnostics;
        	
        	// Number of diagnostic test pointers in the array.
        	UINT32                          m_NumberOfDiagTests;
	
        	// Last test enumerated as diagnostic that is ready to run.
        	T **                            m_ppTestEnumeration; 
		
            // Linkage to system runtime parameters scheduler needs to run tests in
            DiagnosticRunTimeParameters     m_RuntimeData;
            
       	    // Number of timeslices between diagnostics completion time checks
        	// Start Fault Injection Point 3
        	// Declaration of DGN_COMPL_CHECK_INTERVAL_TIME_SLICE constant with a smaller value to make
        	// completion time diagnostic injected fault happen faster will be injected here.
        	//	static const DiagSlices_t DGN_COMPL_CHECK_INTERVAL_TIME_SLICE = 15 * DGN_INTERVALS_PER_MINUTE;

        	// Timestamp of current diagnostic iteration in clock ticks since diagnostic cycle started.
        	UINT64 m_TimestampCurrent;	
	
            // Timestamp of when the current diagnostic iteration started in terms of clock ticks
            UINT64 m_TimeTestCycleStarted;
    
            // For determining if a new period within the total time to run all the tests has started.
            UINT64 m_TimeLastIterationPeriodExpired;

            UINT32 m_NumberOfDiagCycles;
            //***************************************************************************
            // PRIVATE METHODS
            //***************************************************************************
            
            // Hide certain members of a class that are automatically created by the compiler if not defined;
            // Don't want the default constructer, the copy constructor, or the assignment operator to be 
            // accessable.
            DiagnosticScheduler();
	
        	DiagnosticScheduler(const  DiagnosticScheduler &);

        	DiagnosticScheduler &operator=(const DiagnosticScheduler &);

            /////////////////////////////////////////////////////////////////////////////////////////////////
            ///	METHOD NAME: DiagnositcScheduler: AreAllTestsComplete
            ///
            /// @par Full Description
            ///      Determine if all tests are complete for the diagnostic cycle.
            ///      
            ///
            /// @param                        none
            ///
            /// @return                       TRUE when all tests have completed for a diagnostic cycle
            ///
            ///////////////////////////////////////////////////////////////////////////////////////////////////////////
            BOOL AreAllTestsComplete();
            
            //////////////////////////////////////////////////////////////////////////////////////////////////////////
            ///	METHOD NAME: DiagnositcScheduler: ConfigureErrorCode
            ///
            /// @par Full Description
            ///      Configure error code to report.
            ///      
            ///
            /// @param                        returnedErrorCode: Error code generated as a result of testing
            ///
            ///                               testType:          Test type that detected the error code.  Almost all 
            ///                                                  errors detected by the scheduler will have the sched-
            ///                                                  uler test type IE most errors detected by individual 
            ///                                                  tests are reported by the tests themselves and never
            ///                                                  get passed back to the scheduler.              
            ///                               
            /// @return                       Error reported and recorded.
            ///
            ///////////////////////////////////////////////////////////////////////////////////////////////////////////
        	void ConfigureErrorCode( UINT32 & returnedErrorCode, UINT32 testTypeCurrent );

            //////////////////////////////////////////////////////////////////////////////////////////////////////////
            ///	METHOD NAME: DiagnositcScheduler: DetermineCurrentSchedulerState
            ///
            /// @par Full Description
            ///      Determine the current state the scheduler is in.
            ///      
            ///
            /// @param                        None.              
            ///                                     
            ///                               
            /// @return                       m_CurrentScheduleState configured with current scheduler stated.
            ///
            ///////////////////////////////////////////////////////////////////////////////////////////////////////////
        	void DetermineCurrentSchedulerState();

            //////////////////////////////////////////////////////////////////////////////////////////////////////////
            ///	METHOD NAME: DiagnositcScheduler: DetermineIfIterationsAreScheduled
            ///
            /// @par Full Description
            ///      Determines if test iterations are scheduled to run.  The state of the scheduler is configured 
            ///      and a pointer to the next test to run is also configured..
            ///      
            ///
            /// @param                        None.              
            ///                                     
            ///                               
            /// @return                       m_CurrentScheduleState configured with current scheduler stated.
            ///
            ///////////////////////////////////////////////////////////////////////////////////////////////////////////
            void DetermineIfIterationsAreScheduled();
            
            //////////////////////////////////////////////////////////////////////////////////////////////////////////
            ///	METHOD NAME: DiagnositcScheduler: DoMoreDiagnosticTesting
            ///
            /// @par Full Description
            ///      Schedulre more diagnostic testing when there is more testing to do.
            ///      
            ///
            /// @param                        None.            
            ///                                                                    
            /// @return                       Test iteration performed
            ///.
            ///////////////////////////////////////////////////////////////////////////////////////////////////////////
        	void DoMoreDiagnosticTesting();

            ///////////////////////////////////////////////////////////////////////////////////////////////////////////
            ///	METHOD NAME: DiagnositcScheduler: IsDiagnosticCyleTimePeriodExpired
            ///
            /// @par Full Description
            ///      Returns TRUE when a diagnostic cylcle time period has expired
            ///      
            ///
            /// @param                        None.
            ///                               
            /// @return                       TRUE when the time period for a diagnostic cycle has expired
            ///                             
            ///////////////////////////////////////////////////////////////////////////////////////////////////////////
            BOOL IsDiagnosticCyleTimePeriodExpired();

            ///////////////////////////////////////////////////////////////////////////////////////////////////////////
            ///	METHOD NAME: DiagnositcScheduler: IsIterationWithinDiagnosticCycleExpired
            ///
            /// @par Full Description
            ///      Returns TRUE when an iternation period within a diagnostic cylcle time period has expired
            ///      
            ///
            /// @param                        None.
            ///                               
            /// @return                       TRUE when an iternation period within a diagnostic cylcle time period 
            ///                               has expired
            ///                             
            ///////////////////////////////////////////////////////////////////////////////////////////////////////////
            BOOL IsIterationWithinDiagnosticCycleExpired();
            ///////////////////////////////////////////////////////////////////////////////////////////////////////////
            ///	METHOD NAME: DiagnositcScheduler: IsTestingCompleteForDiagCycle
            ///
            /// @par Full Description
            ///      Returns true when a testing for test indicated is complete for current diagnostic cycle.
            ///      
            ///
            /// @param                        rpPbdt: Reference to a pointer to determine if testing is completed for the 
            ///                                       current diagnostic cycle. A reference is used because references 
            ///                                       can't be NULL and thus no concerns about de-referencing a NULL 
            ///                                       pointer.
            ///                               
            /// @return                       TRUE when a test is completed for 
            ///                               the current diagnostic cycle.
            ///                             
            ///////////////////////////////////////////////////////////////////////////////////////////////////////////
            BOOL IsTestingCompleteForDiagCycle( T * & rpPbdt );            

            ///////////////////////////////////////////////////////////////////////////////////////////////////////////
            ///	METHOD NAME: DiagnositcScheduler: IsTestScheduledToRun
            ///
            /// @par Full Description
            ///      Returns true when a test is scheduled to run and returns a pointer to the test to be run.
            ///      
            ///
            /// @param                         rpPbdt: Reference to a pointer when a test is scheduled to be run for 
            ///                                        the current iteration.
            ///                               
            /// @return                        TRUE when a test iteration is ready 
            ///                                to be run by the scheduler.
            ///                             
            ///////////////////////////////////////////////////////////////////////////////////////////////////////////
        	BOOL IsTestScheduledToRun( T * & rpPbdt );

    };
};
#endif  // if !defined(DIAGNOSTIC_SCHEDULER_HPP)


