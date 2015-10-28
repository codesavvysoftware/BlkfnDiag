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
    	DiagnosticCommon::DiagElapsedTime_t	      iterationPeriod;
    	DiagnosticCommon::DiagElapsedTime_t       maxTestDuration;
    	DiagnosticCommon::DiagElapsedTime_t		  maxTimeForTestToComplete;
    	DiagnosticCommon::DiagElapsedTime_t       scheduledToRunTime;
    	DiagnosticCommon::DiagTimestampTime_t     testStartTimestamp;
		DiagnosticCommon::DiagTimestampTime_t     testIterationStartTimestamp;
   		BOOL 		        				  completeForDiagCycle;     		// When true test configures for executing next cycle;
   		UINT32              				  nmbrTimesToRunPerDiagCycle;  		// Number of times to run the test per diagnostic cycle
   		UINT32		               			  nmbrTimesRanThisDiagCycle;   		// Times test has run this cycle
		DiagnosticTestTypes 				  testType;
		TestState  							  tsCurrentTestState;	
    } BlackfinExecTestData;    

    
	BlackfinDiagTest( BlackfinExecTestData & newTestExecutionData ) : testExecutionData ( newTestExecutionData )    
 	{}
		
			
	virtual ~BlackfinDiagTest()  {}

	BOOL				  GetCompleteForDiagCycleStatus();
	
	TestState             GetCurrentTestState();

	DiagTimestampTime_t   GetStartOfTestIterationTimestamp();

	DiagElapsedTime_t     GetIterationPeriod();

	DiagTimestampTime_t   GetStartOfTestTimestamp();

	BOOL				  GetTestRanSpecifiedNumberOfTimesStatus();
	
	DiagnosticTestTypes   GetTestType();

	void                  IncrementTimesRanThisDiagCycle();

	BOOL                  IsTestInProgress();	
	
	void                  ResetCompleteForDiagCycle();

	void                  SetCompleteForDiagCycle();

	void                  SetTimesRanThisDiagCycle(UINT32 nmberOfTimesRan);

	void                  SetInitialSchedule();
	
	void                  SetStartOfTestIterationTimestamp(DiagTimestampTime_t timestamp);

	void				  SetStartOfTestTimestamp(DiagTimestampTime_t timestamp);

	void                  UpdateMaxDurationTime();  

	static DiagnosticCommon::DiagElapsedTime_t   GetTimeslicePeriod_milleseconds(); 
		

	virtual TestState   RunTest( UINT32 & ErrorCode ) = 0;

protected:

	virtual void 		ConfigureForNextTestCycle() = 0;	

	void                ConfigForAnyNewDiagCycle( BlackfinDiagTest * btd ); 
	
	BOOL                HaveTestsCompletedAtLeastOnce();


private:
	
	static const DiagnosticCommon::DiagElapsedTime_t DiagTimeSlicePeriod_Milleseconds = 50;  // Resolution of microseconds

	BlackfinExecTestData  testExecutionData;
	
	BlackfinDiagTest() {}
	
	BlackfinDiagTest(const BlackfinDiagTest & other) {}

	const BlackfinDiagTest & operator = (const BlackfinDiagTest &);
		
	BOOL AreThereMoreCompleteTestsToRunThisDiagCycle();

};


};

