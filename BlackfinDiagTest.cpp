#include "BlackfinDiagTest.h"
using namespace DiagnosticCommon;

namespace BlackfinDiagTests {
	
void BlackfinDiagTest::ConfigForAnyNewDiagCycle( BlackfinDiagTest * btd ) {
		
	if ( testExecutionData.nmbrTimesRanThisDiagCycle > 0 ) {
			
		testExecutionData.nmbrTimesRanThisDiagCycle = 0;

		btd->ConfigureForNextTestCycle();
	}
}



BlackfinDiagTest::TestState BlackfinDiagTest::GetCurrentTestState() {

	return testExecutionData.tsCurrentTestState;
}	
		
DiagTimestampTime_t BlackfinDiagTest::GetIterationCompletedTimestamp() {
	return testExecutionData.iterationCompleteTimestamp;
}

DiagElapsedTime_t BlackfinDiagTest::GetIterationPeriod() {
	return testExecutionData.iterationPeriod;
}

UINT32 BlackfinDiagTest::GetNumberOfTimesToRunPerDiagCycle() {
	return testExecutionData.nmbrTimesToRunPerDiagCycle;  
}

UINT32 BlackfinDiagTest::GetNumberOfTimesRanThisDiagCycle() {
	return ;  
}

DiagElapsedTime_t BlackfinDiagTest::GetOffsetFromDiagCycleStart() {
	return testExecutionData.offsetFromDiagCycleStart;
}


BlackfinDiagTests::BlackfinDiagTest::DiagnosticTestTypes BlackfinDiagTest::GetTestType() {
	
	return testExecutionData.testType; 
}
	

void BlackfinDiagTest::SetCurrentTestState( TestState tsCurrent ) {

	testExecutionData.tsCurrentTestState = tsCurrent;
}

void BlackfinDiagTest::SetIterationCompletedTimestamp(DiagTimestampTime_t timestamp) {
	testExecutionData.iterationCompleteTimestamp = timestamp;
}

void BlackfinDiagTest::SetIterationPeriod(DiagElapsedTime_t period) {
	testExecutionData.iterationPeriod = period;
}
			
void BlackfinDiagTest::SetNumberOfTimesRanThisDiagCycle(UINT32 nmberOfTimesRan) {
	testExecutionData.nmbrTimesRanThisDiagCycle = nmberOfTimesRan;
}

void BlackfinDiagTest::SetNumberOfTimesToRunPerDiagCycle(UINT32 numberTimesToRun) {
	testExecutionData.nmbrTimesToRunPerDiagCycle = numberTimesToRun;  
}
	
void BlackfinDiagTest::SetOffsetFromDiagCycleStart(DiagElapsedTime_t offset) {
	testExecutionData.offsetFromDiagCycleStart;
}

void BlackfinDiagTest::SetTestType(BlackfinDiagTests::BlackfinDiagTest::DiagnosticTestTypes type) {
	
	testExecutionData.testType = type; 
}
	
};