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
		DiagnosticCommon::DiagElapsedTime_t       offsetFromDiagCycleStart;
		DiagnosticCommon::DiagTimestampTime_t     iterationCompleteTimestamp;
   		UINT32              				      nmbrTimesToRunPerDiagCycle;  		// Number of times to run the test per diagnostic cycle
   		UINT32		               			      nmbrTimesRanThisDiagCycle;   		// Times test has run this cycle
		DiagnosticTestTypes 				      testType;
		TestState  							      tsCurrentTestState;	
    } BlackfinExecTestData;    

    
	BlackfinDiagTest( BlackfinExecTestData & newTestExecutionData );		
			
	virtual ~BlackfinDiagTest()  {}

	TestState                               GetCurrentTestState();
	
	DiagnosticCommon::DiagTimestampTime_t   GetIterationCompletedTimestamp();

	DiagnosticCommon::DiagElapsedTime_t     GetIterationPeriod();
	
	UINT32                                  GetNumberOfTimesToRunPerDiagCycle();
	
	UINT32                                  GetNumberOfTimesRanThisDiagCycle();
	
	DiagnosticCommon::DiagElapsedTime_t     GetOffsetFromDiagCycleStart();

	DiagnosticTestTypes                     GetTestType();
	
	
	void                  SetCurrentTestState(TestState);
	
	void				  SetIterationCompletedTimestamp(DiagnosticCommon::DiagTimestampTime_t timestamp);

	void			      SetIterationPeriod(DiagnosticCommon::DiagElapsedTime_t period);
	
	void                  SetNumberOfTimesToRunPerDiagCycle(UINT32);
	
	void                  SetNumberOfTimesRanThisDiagCycle(UINT32);
	
	void 				  SetOffsetFromDiagCycleStart(DiagnosticCommon::DiagElapsedTime_t offset);

	void				  SetTestType(DiagnosticTestTypes test_type);

	virtual TestState     RunTest( UINT32 & ErrorCode ) = 0;

protected:

	virtual void 		ConfigureForNextTestCycle() = 0;	

	void                ConfigForAnyNewDiagCycle( BlackfinDiagTest * btd ); 
	
private:
	
	static const DiagnosticCommon::DiagElapsedTime_t DiagTimeSlicePeriod_Milleseconds = 50;  // Resolution of microseconds

	BlackfinExecTestData  testExecutionData_;
	
	BlackfinDiagTest() {}
	
	BlackfinDiagTest(const BlackfinDiagTest & other) {}

	const BlackfinDiagTest & operator = (const BlackfinDiagTest &);

};


};

