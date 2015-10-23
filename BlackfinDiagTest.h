#pragma once
#include "BlackfinDiag.h"

using namespace DiagnosticCommon;


class BlackfinDiagTest {

public:
	enum TestState {
		TEST_LOOP_COMPLETE,
		TEST_IN_PROGRESS,
		TEST_FAILURE
	};

	typedef enum _DiagnosticTestTypes {
		DiagDataRamTestType        = 1,
		DiagInstructionRamTestType = 2,
		DiagRegisterTestType       = 3,
		DiagSchedulerTestType      = 4 } DiagnosticTestTypes;
	
    typedef UINT32 (* const REGISTER_TEST)(const UINT32 *, UINT32);
	
    typedef struct {
    	const REGISTER_TEST * RegisterTests;
    	UINT32                NumberOfRegisterTests;
		BOOL                  testsCompleted;
    }  RegisterTestDescriptor;
    
		
    typedef struct {
    	UINT8 * pDataRamAddressStart;
    	UINT32  NumberOfContiguousBytesToTest;
    	UINT32  NumberOfBytesTested;
    	BOOL    testCompleted;
    } DataRamTestDescriptor;
    
    typedef struct {
    	DataRamTestDescriptor  BankA;
    	DataRamTestDescriptor  BankB;
    	DataRamTestDescriptor  BankC;
    } BlackfinDataRamTestSuite;
    
	BlackfinDiagTest( 	DiagnosticTestTypes TestType,
						UINT32              PeriodBetweenIterations_Milleseconds, 
						UINT32              FrequencyOfExecutionPerDiagCycle = nDefaultTimesToRunPerCycle,
	                  	UINT32              MaxTestDuration_Milleseconds 
	                                           = (dcbTestExecutionTimeoutPeriodSlices_Default * DiagnosticSlicePeriod_Milleseconds) ) 
	:	triggerValueTimeslice   ( dcbSliceTriggerVal_Default ),
		stepValueTimeslice      ( PeriodBetweenIterations_Milleseconds / DiagnosticSlicePeriod_Milleseconds ),
		lastCompleteTimeslice   ( dcbSliceLastComplete_Default ),
		durationUs              ( dcbTestDurationInSlices_Default ),
		maxTimeslices           ( dcbTestExecutionMaxSlices_Default ),
		oneComplete             ( dcbTestComplete_Default ),
		timeoutTimeslice        ( MaxTestDuration_Milleseconds / DiagnosticSlicePeriod_Milleseconds ),
		bCompleteForDiagCycle   ( TRUE ),
		nTimesToRunPerDiagCycle ( FrequencyOfExecutionPerDiagCycle ),
		nTimesRanThisDiagCycle	( 0 )
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
		return ((value > DiagnosticSlicePeriod_Milleseconds) 
					? value/DiagnosticSlicePeriod_Milleseconds : 1);
	}
	
	DiagnosticTestTypes GetTestType() { return TestType; }
	
	virtual void AdjustForAnyScheduleTimeConflict( BlackfinDiagTest * pdbtPrev ) {

		if ( triggerValueTimeslice == pdbtPrev->triggerValueTimeslice) { 

			triggerValueTimeslice++;
		}
	}
	
	BOOL HasTestTakenTooLongToRun( DiagSlices_t CurrentTimeSlice ) { 
		BOOL bTestExecutionTooLong = FALSE;
		
		if ( ( CurrentTimeSlice - lastCompleteTimeslice ) > timeoutTimeslice ) {
			bTestExecutionTooLong = TRUE;
		}
		
		return bTestExecutionTooLong;
	}
	
	virtual BOOL IsItTimeToScheduleNextTest( DiagSlices_t CurrentTimeSlice ) { return (CurrentTimeSlice > triggerValueTimeslice); }
	
	virtual void SetMaxTestExecutionPeriod_Timeslices( DiagSlices_t CurrentTimeSlice ) {  
		
		// If the test has completed all possible iterations, figure out
		// how long it's been since the last total completion.
		DiagSlices_t timeToCompleteTimeslice = CurrentTimeSlice - lastCompleteTimeslice;

		if (    ( timeToCompleteTimeslice > maxTimeslices )
			 && ( oneComplete == TRUE)
		   ) {
		   	maxTimeslices = timeToCompleteTimeslice;
		} 
	}

	virtual void SetTestComplete( DiagSlices_t CurrentTimeSlice ) {

		oneComplete = true;
		
		lastCompleteTimeslice = CurrentTimeSlice;
	}
			
    virtual void SetCompletedTestDuration_microseconds( DiagTime_t CurrentTime_microseconds ){ 
    	durationUs = CurrentTime_microseconds - TestStartTime_microseconds;
    }
    
    virtual void      SetTimesliceForNextTestIteration( DiagSlices_t Adjustment ) { triggerValueTimeslice = stepValueTimeslice + Adjustment; }
		
	virtual void      UpdateTimesliceForNextTestIteration() { triggerValueTimeslice += stepValueTimeslice; }
		
	virtual TestState   RunTest( UINT32 & ErrorCode, DiagTime_t SystemTime = GetSystemTime() ) = 0;

	static  DiagTime_t GetTimeslicePeriod_microseconds() { return DiagnosticSlicePeriod_Microseconds; }

protected:

	virtual BOOL        IsTestComplete() = 0;
	virtual void 		ConfigureForNextTestCycle() = 0;	

	BOOL AreTestsCompletedFromLastCycle() { return bCompleteForDiagCycle; }
	
	void ConfigForAnyNewDiagCycle( BlackfinDiagTest * btd ) {
		BOOL bTestsCompletedFromLastCycle = AreTestsCompletedFromLastCycle();
		
		if ( bTestsCompletedFromLastCycle ) {
			
			ResetTestsCompletedForCycle();
			
			btd->ConfigureForNextTestCycle();
		}
	}
	
	void ResetTestsCompletedForCycle() { 
		bCompleteForDiagCycle  = FALSE;
		nTimesRanThisDiagCycle = 0;
	}
	
	void SetTestsCompletedForCycle() { bCompleteForDiagCycle = TRUE; }

private:
	
	static const DiagTime_t DiagnosticSlicePeriod_Microseconds = 50000;  // Resolution of microseconds

	static const DiagTime_t DiagnosticSlicePeriod_Milleseconds = DiagnosticSlicePeriod_Microseconds / 1000;

	static const DiagTime_t MaxCyclePeriodAllTestsHoursComponent        = 4;
	static const DiagTime_t MaxCyclePeriodAllTestsMinutesComponent      = 0;
	static const DiagTime_t MaxCyclePeriodAllTestsSecondsComponent      = 0;
	static const DiagTime_t MaxCyclePeriodAllTestsMillesecondsComponent = 0;
	static const DiagTime_t MillesecondsInAnHour = 3600000;  //60 minutes/per hour * 60 seconds per minute * 1000 milleseconds per second

	static const DiagSlices_t dcbSliceTriggerVal_Default                  = 0;
	static const DiagSlices_t dcbSliceStepVal_Default                     = 1;
	static const DiagSlices_t dcbSliceLastComplete_Default                = 0;
	static const DiagSlices_t dcbTestDurationInSlices_Default             = 0;	
	static const DiagSlices_t dcbTestExecutionMaxSlices_Default           = 0;
	static const BOOL         dcbTestComplete_Default                     = FALSE;
	static const DiagSlices_t dcbTestExecutionTimeoutPeriodSlices_Default = (  ( MaxCyclePeriodAllTestsHoursComponent * MillesecondsInAnHour )
    	                                                               + ( MaxCyclePeriodAllTestsMinutesComponent * 60 * 1000 )
        	                                                           + ( MaxCyclePeriodAllTestsSecondsComponent * 100 )
            	                                                       + ( MaxCyclePeriodAllTestsMillesecondsComponent ))
                	                                                   /DiagnosticSlicePeriod_Milleseconds;
	static const UINT32       nDefaultTimesToRunPerCycle                  = 1;
	
	UINT32       nTimesToRunPerDiagCycle;  // Number of times to run the test per diagnostic cycle
	UINT32		 nTimesRanThisDiagCycle;   // Times test has run this cycle
	DiagSlices_t triggerValueTimeslice;    // Timeslice number for next trigger
	DiagSlices_t stepValueTimeslice;       // Number of diag timeslices between activationsr
	DiagSlices_t lastCompleteTimeslice;    // Timeslice number of the previous test completioN
	DiagSlices_t durationUs;               // Time spent in diag function
	DiagSlices_t maxTimeslices;            // Max number of timeslices ever seen between triggers
	BOOL         oneComplete;              // True if diag has completed at least once. This is used to decide
										   //      whether or not maxTimeslices is valid, since  maxTimeslices
										   //      should indicate a completion-to-completion time
   	BOOL 		bCompleteForDiagCycle;     // When true test configures for executing next cycle;

   	DiagSlices_t timeoutTimeslice;         // Max number of timeslices that test
										   // can wait to be completed in
									
	DiagTime_t TestStartTime_microseconds;	

   	DiagnosticTestTypes TestType;	    

	BlackfinDiagTest() {}
	
	BlackfinDiagTest(const BlackfinDiagTest & other) {}

	const BlackfinDiagTest & operator = (const BlackfinDiagTest &);
		
};




