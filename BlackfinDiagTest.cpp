#include "BlackfinDiagTest.hpp"
#include "Os_iotk.h"
#include "Hw.h"

namespace BlackfinDiagTesting 
{
    BlackfinDiagTest::BlackfinDiagTest( BlackfinExecTestData  newTestExecutionData ) : m_TestExecutionData ( newTestExecutionData )    
 	{
 	}
	
    void BlackfinDiagTest::ConfigForAnyNewDiagCycle( BlackfinDiagTest * btd ) 
    {
		if ( 
		        (m_TestExecutionData.m_NmbrTimesRanThisDiagCycle > 0)
		     || (TEST_IDLE == m_TestExecutionData.m_CurrentTestState)
		   ) 
	    {
			m_TestExecutionData.m_NmbrTimesRanThisDiagCycle = 0;

		    btd->ConfigureForNextTestCycle();
	    }
    }

    TestState BlackfinDiagTest::GetCurrentTestState() 
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

    DiagElapsedTime BlackfinDiagTest::GetMaxTimeBetweenTestCompletions()
    {
        return m_TestExecutionData.m_MaximumTimeBetweenTestCompletions;
    }
    
    UINT32 BlackfinDiagTest::GetNumberOfTimesToRunPerDiagCycle() 
    {
	    return m_TestExecutionData.m_NmbrTimesToRunPerDiagCycle;  
    }

    UINT32 BlackfinDiagTest::GetNumberOfTimesRanThisDiagCycle() 
    {
	    return m_TestExecutionData.m_NmbrTimesRanThisDiagCycle;  
    }

    DiagTimestampTime BlackfinDiagTest::GetTestCompletedTimestamp()
    {
        return m_TestExecutionData.m_TestCompleteTimestamp;
    }
    
    DiagTimestampTime BlackfinDiagTest::GetTestStartTime()
    {
        return m_TestExecutionData.m_TestStartTimestamp;
    }
     
    BlackfinDiagTesting::BlackfinDiagTest::DiagnosticTestTypes BlackfinDiagTest::GetTestType() 
    {	
	   return m_TestExecutionData.m_TestType; 
    }
	
    void BlackfinDiagTest::SetCurrentIterationDuration( DiagElapsedTime duration )
    {
        m_TestExecutionData.m_CurrentIterationDuration = duration;
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
			
    void BlackfinDiagTest::SetMaxTimeBetweenTestCompletions( DiagElapsedTime period )
    {
        m_TestExecutionData.m_MaximumTimeBetweenTestCompletions = period;
    }
    
    void BlackfinDiagTest::SetNumberOfTimesRanThisDiagCycle(UINT32 nmberOfTimesRan) 
    {
        m_TestExecutionData.m_NmbrTimesRanThisDiagCycle = nmberOfTimesRan;
    }

    void BlackfinDiagTest::SetTestCompletedTimestamp( DiagTimestampTime period )
    {
        m_TestExecutionData.m_TestCompleteTimestamp = period;
    }
    
    void BlackfinDiagTest::SetTestStartTime( DiagTimestampTime time )
    {
        m_TestExecutionData.m_TestStartTimestamp = time;
    }
     
	
};


