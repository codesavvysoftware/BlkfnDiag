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
						DiagTime_t          PeriodBetweenIterations_Milleseconds, 
						DiagTime_t          ScheduledRuntime_milleseconds,              
						UINT32              FrequencyOfExecutionPerDiagCycle = nDefaultTimesToRunPerCycle,
	                  	UINT32              MaxTestDuration_Milleseconds = PeriodForAllDiagnosticsCompleted_milleseconds )
	:	IterationPeriod          ( PeriodBetweenIterations_Milleseconds ),
		ScheduledToRunTime       ( ScheduledRuntime_milleseconds ),
		CurrentTestExecutionTime ( 0 ),
		MaxTestDuration          ( 0 ),
		MaxTimeForTestToComplete ( MaxTestDuration_Milleseconds ),
		bCompleteForDiagCycle    ( TRUE ),
		nTimesToRunPerDiagCycle  ( FrequencyOfExecutionPerDiagCycle ),
		nTimesRanThisDiagCycle	 ( 0 )
                                                                                     // Probably will be constant for all tests
	{}
		
			
	virtual ~BlackfinDiagTest()  {}

	DiagnosticTestTypes GetTestType() { return TestType; }
	
	TestState GetCurrentTestState() {
		return tsCurrentTestState;
	}

	BOOL HasTestTakenTooLongToRun() { 
		BOOL bTestExecutionTooLong = FALSE;

		DiagTime_t CurrentTime = GetSystemTime();

		DiagTime_t ElapsedTime = CurrentTime - TestStartTime_milleseconds;

		if ( ElapsedTime > MaxTimeForTestToComplete ) {
			bTestExecutionTooLong = TRUE;
		}
		
		return bTestExecutionTooLong;
	}
	
	BOOL IsTestingCompleteForDiagCycle() {
		
		BOOL bTestingForCycleCompleted = FALSE;
		
		BOOL bTestsHaveCompleteAtLeastOnce = HaveTestsCompletedAtLeastOnce();
		
		BOOL bNoMoreCompleteTestsToRun = !AreThereMoreCompleteTestsToRunThisDiagCycle();
		
		BOOL bTestIsNotRunning = !IsTestInProgress();
		
		if (    bTestsHaveCompleteAtLeastOnce
			 && bNoMoreCompleteTestsToRun ) {
			 bTestingForCycleCompleted = TRUE;
		}
			 	
		return bTestingForCycleCompleted;
	}	
			
		
	BOOL IsTestInProgress() { return (TEST_IN_PROGRESS == tsCurrentTestState ); } 		
	

	BOOL IsTestScheduledToRun() {
	
		BOOL bTimeToRun = FALSE;
	
		DiagTime_t CurrentTime = GetSystemTime();
	
		DiagTime_t ElapsedTime = CurrentTime - ScheduledToRunTime;
	
		if (ElapsedTime >= IterationPeriod ) {
			bTimeToRun = TRUE;
		}
	
		return bTimeToRun;
	}
		
	void ResetTestsCompletedForCycle() {
		bCompleteForDiagCycle  = FALSE;
		nTimesRanThisDiagCycle = 0;
	}
	
	void SetAnotherTestCompletedForCycle() { 
		
		bCompleteForDiagCycle = TRUE;
		
		CurrentTestExecutionTime = GetSystemTime();
		
		CurrentTestExecutionTime -=  TestStartTime_milleseconds;

		nTimesRanThisDiagCycle++;
	}

	void SetCurrentTestState( TestState tsCurrent ) {
		tsCurrentTestState = tsCurrent;
	}
	
	void SetInitialSchedule() {
		ScheduledToRunTime += GetSystemTime();
	}
	
	void ScheduleTestForNextCycle() {
		
		ScheduledToRunTime = GetSystemTime();
	}
		
	void UpdateMaxDurationTime() {  
		
		if ( CurrentTestExecutionTime >   MaxTestDuration ) {
			MaxTestDuration = CurrentTestExecutionTime;
		}
	}

	virtual TestState   RunTest( UINT32 & ErrorCode, DiagTime_t SystemTime = GetSystemTime() ) = 0;

	static DiagTime_t   GetTimeslicePeriod_milleseconds() { return DiagTimeSlicePeriod_Milleseconds; }
	
protected:

	virtual void 		ConfigureForNextTestCycle() = 0;	

	void ConfigForAnyNewDiagCycle( BlackfinDiagTest * btd ) {
		BOOL bTestsCompletedFromLastCycle = HaveTestsCompletedAtLeastOnce();
		
		if ( bTestsCompletedFromLastCycle ) {
			
			ResetTestsCompletedForCycle();
			
			btd->ConfigureForNextTestCycle();
		}
	}
	
	BOOL HaveTestsCompletedAtLeastOnce() { return (nTimesRanThisDiagCycle > 0); }


private:
	
	static const DiagTime_t DiagTimeSlicePeriod_Milleseconds = 50;  // Resolution of microseconds

	static const UINT32       nDefaultTimesToRunPerCycle                  = 1;
	
   	BOOL 		bCompleteForDiagCycle;     // When true test configures for executing next cycle;

	DiagTime_t	 IterationPeriod;
	DiagTime_t   CurrentTestExecutionTime;
	DiagTime_t   MaxTestDuration;
	DiagTime_t	 MaxTimeForTestToComplete;
	DiagTime_t   ScheduledToRunTime;
	DiagTime_t   TestStartTime_milleseconds;	

	
	
	UINT32       nTimesToRunPerDiagCycle;  // Number of times to run the test per diagnostic cycle
	UINT32		 nTimesRanThisDiagCycle;   // Times test has run this cycle
									
   	DiagnosticTestTypes TestType;
   	
   	TestState  			tsCurrentTestState;	    

	BlackfinDiagTest() {}
	
	BlackfinDiagTest(const BlackfinDiagTest & other) {}

	const BlackfinDiagTest & operator = (const BlackfinDiagTest &);
		
	BOOL AreThereMoreCompleteTestsToRunThisDiagCycle() {
		return ( nTimesRanThisDiagCycle < nTimesToRunPerDiagCycle );
	}

};




