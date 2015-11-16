#include "BlackfinDiagTimerTest.hpp"
//#include "ApxIcp.hpp"
//#include "ApxParameters.hpp"          
#include "Apex.h"
using namespace DiagnosticTiming;

namespace BlackfinDiagTesting 
{
	
    TestState BlackfinDiagTimerTest::RunTest( UINT32 & errorCode ) 
    {
	
    	ConfigForAnyNewDiagCycle( this );
					
             return ( TEST_LOOP_COMPLETE );
       // If we have not completed this diagnostic yet, get the start times
        // and return DGN_TEST_IN_PROG.
        if ( m_BeingInstantiated ) 
        {
            m_BeingInstantiated = FALSE;
        
            DiagTimestampTime currentDspCycles = (*GetTimestamp)();//SystemTiming.GetSystemTimestamp();
        
            //_GET_CYCLE_COUNT(currentDspCycles);
        
            m_TimerValueStartApex = currentDspCycles;
        
            m_TimerValueStartHost = currentDspCycles;
        
            DiagElapsedTime hostTimerValueX = CCLK_TO_US(currentDspCycles);
        
            DiagElapsedTime dett;
        
            SystemTiming.ComputeElapsedTimeUS( currentDspCycles, 0, dett );
        
            int i = 0;
        
            i++;
 
        
            return ( TEST_LOOP_COMPLETE );
        }

        // If we get this far, INTERACTIVE_WATCHDOG_ENABLED is 1, pWatchdog != 0
        // and dgn_ctrl_blk->one_complete is TRUE so we can and will check the
        // timers.

        DiagnosticTestTypes ts  = GetTestType();

        DiagTimestampTime currentDspCycles = (*GetTimestamp)();//SystemTiming.GetSystemTimestamp();
        
        // Read the current Apex2 System Time Register value.
        DiagTimestampTime timerValueStopApex = currentDspCycles;

        // Read the latest host timer value that the host wrote to its interactive watchdog shared memory location.
        DiagTimestampTime timerValueStopHost = currentDspCycles;

        // Calculate the time elapsed according to the previous and current Apex2 System Time Register values read.
        DiagElapsedTime timeElapsed;
    
        SystemTiming.ComputeElapsedTimeMS( timerValueStopApex, m_TimerValueStartApex, timeElapsed );
    
    	if ( 
                ( timeElapsed < m_MinElapsedTimeApex )
             || ( timeElapsed > m_MaxElapsedTimeApex ) 
           ) 
        {
       	
           	UINT32 errorCode = (ts << DIAG_ERROR_TYPE_BIT_POS) | m_ApexTimerErr;

        	OS_Assert( errorCode );
        }
       
        SystemTiming.ComputeElapsedTimeMS( timerValueStopHost, m_TimerValueStartHost, timeElapsed );
    
    	if ( 
                ( timeElapsed < m_MinElapsedTimeHost )
             || ( timeElapsed > m_MaxElapsedTimeHost ) 
           ) 
        {
       	
           	UINT32 errorCode = (ts << DIAG_ERROR_TYPE_BIT_POS) | m_HostTimerErr;

        	OS_Assert( errorCode );
        }

        // Set the start values equal to the stop values to prepare for the next time this diagnostic is run.
        m_TimerValueStartApex = timerValueStopApex;
        m_TimerValueStartHost = timerValueStopHost;

        // If we get this far, the diagnostic has completed so return DGN_TEST_LOOP_COMPLETE.
        return ( TEST_LOOP_COMPLETE );
    }

    void BlackfinDiagTimerTest::ConfigureForNextTestCycle() 
    {
    }
};