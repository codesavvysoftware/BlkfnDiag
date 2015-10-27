#pragma once
#include "BlackfinDiag.h"

namespace BlackfinDiagTests {

class BlackfinDiagTest {

public:
	typedef enum _TestState {
		TEST_LOOP_COMPLETE,
		TEST_IN_PROGRESS,
		TEST_FAILURE,
		TEST_IDLE
	} TestState;

	typedef enum _DiagnosticTestTypes {
		DiagDataRamTestType        = 1,
		DiagInstructionRamTestType = 2,
		DiagRegisterTestType       = 3,
		DiagSchedulerTestType      = 4 
	} DiagnosticTestTypes;
	
    typedef UINT32 (* const REGISTER_TEST)(const UINT32 *, UINT32);
	
    typedef struct {
    	const REGISTER_TEST * registerTests;
    	UINT32                nmbrRegisterTests;
		BOOL                  testsCompleted;
    }  RegisterTestDescriptor;
    
		
    typedef struct {
    	UINT8 * dataRamAddressStart;
    	UINT32  nmbrContiguousBytesToTest;
    	UINT32  nmbrBytesTested;
    	BOOL    testCompleted;
    } DataRamTestDescriptor;
    
    typedef struct {
    	DataRamTestDescriptor  BankA;
    	DataRamTestDescriptor  BankB;
    	DataRamTestDescriptor  BankC;
    } BlackfinDataRamTestSuite;
    
    typedef struct {
    	DiagnosticCommon::DiagTime_t	      iterationPeriod;
    	DiagnosticCommon::DiagTime_t          currentTestExecutionTime;
    	DiagnosticCommon::DiagTime_t          maxTestDuration;
    	DiagnosticCommon::DiagTime_t		  maxTimeForTestToComplete;
    	DiagnosticCommon::DiagTime_t          scheduledToRunTime;
    	DiagnosticCommon::DiagTime_t          testStartTime_milleseconds;	
   		BOOL 		        				  completeForDiagCycle;     		// When true test configures for executing next cycle;
   		UINT32              				  nmbrTimesToRunPerDiagCycle;  		// Number of times to run the test per diagnostic cycle
   		UINT32		               			  nmbrTimesRanThisDiagCycle;   		// Times test has run this cycle
		DiagnosticTestTypes 				  testType;
		TestState  							  tsCurrentTestState;	
    } BlackfinExecTestData;    

    
	BlackfinDiagTest( BlackfinExecTestData & newTestExecutionData ) : testExecutionData ( newTestExecutionData )    
 	{}
		
			
	virtual ~BlackfinDiagTest()  {}

	TestState             GetCurrentTestState();

	DiagnosticTestTypes   GetTestType();
	
	BOOL                  HasTestTakenTooLongToRun(); 
	
	BOOL                  IsTestingCompleteForDiagCycle();
		
	BOOL                  IsTestInProgress();	
	

	BOOL                  IsTestScheduledToRun();
		
	void                  ResetTestsCompletedForCycle();
	
	void                  SetAnotherTestCompletedForCycle();

	void                  SetCurrentTestState( TestState tsCurrent );

	void                  SetInitialSchedule();
	
	void                  ScheduleTestForNextCycle();
		
	void                  UpdateMaxDurationTime();  

	static DiagnosticCommon::DiagTime_t   GetTimeslicePeriod_milleseconds(); 
		

	virtual TestState   RunTest( UINT32 & ErrorCode, DiagnosticCommon::DiagTime_t SystemTime ) = 0;

protected:

	virtual void 		ConfigureForNextTestCycle() = 0;	

	void                ConfigForAnyNewDiagCycle( BlackfinDiagTest * btd ); 
	
	BOOL                HaveTestsCompletedAtLeastOnce();


private:
	
	static const DiagnosticCommon::DiagTime_t DiagTimeSlicePeriod_Milleseconds = 50;  // Resolution of microseconds

	BlackfinExecTestData  testExecutionData;
	
	BlackfinDiagTest() {}
	
	BlackfinDiagTest(const BlackfinDiagTest & other) {}

	const BlackfinDiagTest & operator = (const BlackfinDiagTest &);
		
	BOOL AreThereMoreCompleteTestsToRunThisDiagCycle();

};


};

