#include "BlackfinDiagTest.hpp"

using namespace DiagnosticCommon;

namespace BlackfinDiagTesting 
{
    BlackfinDiagTest::BlackfinDiagTest( BlackfinExecTestData & newTestExecutionData ) : m_TestExecutionData ( newTestExecutionData )    
 	{
 	}
	
    void BlackfinDiagTest::ConfigForAnyNewDiagCycle( BlackfinDiagTest * btd ) 
    {
		
	    if ( m_TestExecutionData.m_NmbrTimesRanThisDiagCycle > 0 ) 
	    {
			m_TestExecutionData.m_NmbrTimesRanThisDiagCycle = 0;

		    btd->ConfigureForNextTestCycle();
	    }
    }

    BlackfinDiagTest::TestState BlackfinDiagTest::GetCurrentTestState() 
    {
        return m_TestExecutionData.m_CurrentTestState;
    }	
		
    DiagTimestampTime BlackfinDiagTest::GetIterationCompletedTimestamp() 
    {
	    return m_TestExecutionData.m_IterationCompleteTimestamp;
    }

    DiagElapsedTime BlackfinDiagTest::GetIterationPeriod() 
    {
	    return m_TestExecutionData.m_IterationPeriod;
    }

    UINT32 BlackfinDiagTest::GetNumberOfTimesToRunPerDiagCycle() 
    {
	    return m_TestExecutionData.m_NmbrTimesToRunPerDiagCycle;  
    }

    UINT32 BlackfinDiagTest::GetNumberOfTimesRanThisDiagCycle() 
    {
	    return m_TestExecutionData.m_NmbrTimesRanThisDiagCycle;  
    }

    BlackfinDiagTesting::BlackfinDiagTest::DiagnosticTestTypes BlackfinDiagTest::GetTestType() 
    {	
	   return m_TestExecutionData.m_TestType; 
    }
	
    void BlackfinDiagTest::SetCurrentTestState( TestState tsCurrent ) 
    {
	    m_TestExecutionData.m_CurrentTestState = tsCurrent;
    }

    void BlackfinDiagTest::SetIterationCompletedTimestamp(DiagTimestampTime timestamp) 
    {
	    m_TestExecutionData.m_IterationCompleteTimestamp = timestamp;
    }

    void BlackfinDiagTest::SetIterationPeriod(DiagElapsedTime period) 
    {
	    m_TestExecutionData.m_IterationPeriod = period;
    }
			
    void BlackfinDiagTest::SetNumberOfTimesRanThisDiagCycle(UINT32 nmberOfTimesRan) 
    {
        m_TestExecutionData.m_NmbrTimesRanThisDiagCycle = nmberOfTimesRan;
    }

    void BlackfinDiagTest::SetNumberOfTimesToRunPerDiagCycle(UINT32 numberTimesToRun) 
    {
	    m_TestExecutionData.m_NmbrTimesToRunPerDiagCycle = numberTimesToRun;  
    }
	

    void BlackfinDiagTest::SetTestType(BlackfinDiagTesting::BlackfinDiagTest::DiagnosticTestTypes type) 
    {	
	    m_TestExecutionData.m_TestType = type; 
    }
	
};


