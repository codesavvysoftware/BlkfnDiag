#include "BlackfinDiagTest.h"
using namespace DiagnosticCommon;

namespace BlackfinDiagTests {
BlackfinDiagTest::BlackfinDiagTest( BlackfinExecTestData & newTestExecutionData ) : testExecutionData_ ( newTestExecutionData )    
 	{
 		int i = 0;
 		
 		i++;
 	}
	
void BlackfinDiagTest::ConfigForAnyNewDiagCycle( BlackfinDiagTest * btd ) {
		
	if ( testExecutionData_.nmbrTimesRanThisDiagCycle > 0 ) {
			
		testExecutionData_.nmbrTimesRanThisDiagCycle = 0;

		btd->ConfigureForNextTestCycle();
	}
}



BlackfinDiagTest::TestState BlackfinDiagTest::GetCurrentTestState() {

	return testExecutionData_.tsCurrentTestState;
}	
		
DiagTimestampTime_t BlackfinDiagTest::GetIterationCompletedTimestamp() {
	return testExecutionData_.iterationCompleteTimestamp;
}

DiagElapsedTime_t BlackfinDiagTest::GetIterationPeriod() {
	return testExecutionData_.iterationPeriod;
}

UINT32 BlackfinDiagTest::GetNumberOfTimesToRunPerDiagCycle() {
	return testExecutionData_.nmbrTimesToRunPerDiagCycle;  
}

UINT32 BlackfinDiagTest::GetNumberOfTimesRanThisDiagCycle() {
	
	return testExecutionData_.nmbrTimesRanThisDiagCycle;  
}

DiagElapsedTime_t BlackfinDiagTest::GetOffsetFromDiagCycleStart() {
	return testExecutionData_.offsetFromDiagCycleStart;
}


BlackfinDiagTests::BlackfinDiagTest::DiagnosticTestTypes BlackfinDiagTest::GetTestType() {
	
	return testExecutionData_.testType; 
}
	

void BlackfinDiagTest::SetCurrentTestState( TestState tsCurrent ) {

	testExecutionData_.tsCurrentTestState = tsCurrent;
}

void BlackfinDiagTest::SetIterationCompletedTimestamp(DiagTimestampTime_t timestamp) {
	testExecutionData_.iterationCompleteTimestamp = timestamp;
}

void BlackfinDiagTest::SetIterationPeriod(DiagElapsedTime_t period) {
	testExecutionData_.iterationPeriod = period;
}
			
void BlackfinDiagTest::SetNumberOfTimesRanThisDiagCycle(UINT32 nmberOfTimesRan) {
	testExecutionData_.nmbrTimesRanThisDiagCycle = nmberOfTimesRan;
}

void BlackfinDiagTest::SetNumberOfTimesToRunPerDiagCycle(UINT32 numberTimesToRun) {
	testExecutionData_.nmbrTimesToRunPerDiagCycle = numberTimesToRun;  
}
	
void BlackfinDiagTest::SetOffsetFromDiagCycleStart(DiagElapsedTime_t offset) {
	testExecutionData_.offsetFromDiagCycleStart;
}

void BlackfinDiagTest::SetTestType(BlackfinDiagTests::BlackfinDiagTest::DiagnosticTestTypes type) {
	
	testExecutionData_.testType = type; 
}
	
};