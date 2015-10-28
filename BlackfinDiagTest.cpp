#include "BlackfinDiagTest.h"
using namespace DiagnosticCommon;

namespace BlackfinDiagTests {
	
void BlackfinDiagTest::ConfigForAnyNewDiagCycle( BlackfinDiagTest * btd ) {
		
	BOOL testsCompletedFromLastCycle = HaveTestsCompletedAtLeastOnce();
		
	if ( testsCompletedFromLastCycle ) {
			
		testExecutionData.completeForDiagCycle = FALSE;

		testExecutionData.nmbrTimesRanThisDiagCycle = 0;

		btd->ConfigureForNextTestCycle();
	}
}
	
DiagTime_t   BlackfinDiagTest::GetTimeslicePeriod_milleseconds() { 
	return DiagTimeSlicePeriod_Milleseconds; 
}
		
BlackfinDiagTest::DiagnosticTestTypes BlackfinDiagTest::GetTestType() {
	
	return testExecutionData.testType; 
}
	
BlackfinDiagTest::TestState BlackfinDiagTest::GetCurrentTestState() {

	return testExecutionData.tsCurrentTestState;
}	
		
BOOL BlackfinDiagTest::IsTestInProgress() { 
	return (TEST_IN_PROGRESS == testExecutionData.tsCurrentTestState ); 
} 		
	

void BlackfinDiagTest::SetCurrentTestState( TestState tsCurrent ) {

	testExecutionData.tsCurrentTestState = tsCurrent;
}
	
void BlackfinDiagTest::SetInitialSchedule() {

	testExecutionData.scheduledToRunTime += GetSystemTime();
}
	
		
void BlackfinDiagTest::UpdateMaxDurationTime() {  
		
	if ( testExecutionData.currentTestExecutionTime >   testExecutionData.maxTestDuration ) {
		
		testExecutionData.maxTestDuration = testExecutionData.currentTestExecutionTime;
	}
}

BOOL BlackfinDiagTest::AreThereMoreCompleteTestsToRunThisDiagCycle() {
		return ( testExecutionData.nmbrTimesRanThisDiagCycle < testExecutionData.nmbrTimesToRunPerDiagCycle );
	}

};
BOOL BlackfinDiagTest::GetTestRanSpecifiedNumberOfTimesStatus() {
	return (testExecutionData.nmbrTimesToRunPerDiagCycle == testExecutionData.nmbrTimesRanThisDiagCycle);
}

BOOL BlackfinDiagTest::GetCompleteForDiagCycleStatus() {
	return testExecutionData.completeForDiagCycle;
}
void BlackfinDiagTest::SetStartOfTestTimestamp(DiagTimestampTime_t timestamp) {
	testExecutionData.testStartTimestamp = timestamp;
}

DiagTimestampTime_t BlackfinDiagTest::GetStartOfTestTimestamp() {
	return testExecutionData.testStartTimestamp;
}

DiagElapsedTime_t BlackfinDiagTest::GetMaxTimeForTestToComplete() {
	return testExecutionData.maxTimeForTestToComplete;
}

void BlackfinDiagTest::SetStartOfTestIterationTimestamp(DiagTimestampTime_t timestamp) {
	testExecutionData.testIterationStartTimestamp = timestamp;
}

void BlackfinDiagTest::ResetCompleteForDiagCycle() {
	testExecutionData.completeForDiagCycle = FALSE;
}
void BlackfinDiagTest::SetTimesRanThisDiagCycle(UINT32 nmberOfTimesRan) {
	testExecutionData.nmbrTimesRanThisDiagCycle = nmberOfTimesRan;
}

void BlackfinDiagTest::IncrementTimesRanThisDiagCycle() {
	++testExecutionData.nmbrTimesRanThisDiagCycle;
}

void BlackfinDiagTest::SetCompleteForDiagCycle() {
	testExecutionData.completeForDiagCycle = TRUE;
}

DiagTimestampTime_t BlackfinDiagTest::GetStartOfTestIterationTimestamp() {
	return testExecutionData.testIterationStartTimestamp;
}

DiagElapsedTime_t BlackfinDiagTest::GetIterationPeriod() {
	return testExecutionData.iterationPeriod;
}

