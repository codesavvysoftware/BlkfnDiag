#include "BlackfinDiagTimerTest.h"
#include "ApxIcp.hpp"
#include "ApxParameters.hpp"          
#include "Apex.h"
using namespace DiagnosticCommon;

namespace BlackfinDiagTesting {
	
cTiming  TimerTestTiming;
    
BlackfinDiagTest::TestState BlackfinDiagTimerTest::RunTest( UINT32 & errorCode ) {
	
	ConfigForAnyNewDiagCycle( this );
					
	if ( FALSE ) { //HI_ApexParam.pWatchdog == 0 ) {
        // This should never actually happen during runtime because ICE2 will
        // detect a software interactive watchdog timeout before this timer
        // diagnostic detects that the watchdog pointer (pWatchdog) has not
        // been changed to a non-zero value.
        //
        // Issue a major non-recoverable fault.
        OS_Assert( 0 );
    }

    // If we have not completed this diagnostic yet, get the start times
    // and return DGN_TEST_IN_PROG.
    if ( beingInstantiated_ ) {
        // Read the current Apex2 System Time Register value.
        //timerValueStartApex_ = HI_ApexReg.SystemTime;
        
        // Read the latest timer value that the host wrote to its interactive
        // watchdog shared memory location.
//        timerValueStartHost_ = HI_ApexParam.pWatchdog;
        
        beingInstantiated_ = FALSE;
        
        DiagTimestampTime_t CurrentDspCycles;
        
        _GET_CYCLE_COUNT(CurrentDspCycles);
        
        DiagElapsedTime_t m_HostTimerValueX = CCLK_TO_US(CurrentDspCycles);
        
        DiagElapsedTime_t dett;
        
        SystemTiming.ComputeElapsedTimeUS( CurrentDspCycles, 0, dett );
        
        int i = 0;
        
        i++;
 
        
        return ( TEST_LOOP_COMPLETE );
    }

    // If we get this far, INTERACTIVE_WATCHDOG_ENABLED is 1, pWatchdog != 0
    // and dgn_ctrl_blk->one_complete is TRUE so we can and will check the
    // timers.

    // Read the current Apex2 System Time Register value.
    DiagTimestampTime_t timerValueStopApex = 0;//HI_ApexReg.SystemTime;

    // Read the latest host timer value that the host wrote to its interactive watchdog shared memory location.
    DiagTimestampTime_t timerValueStopHost = 0;//HI_ApexParam.pWatchdog->host;

    // Calculate the time elapsed according to the previous and current Apex2 System Time Register values read.
    DiagElapsedTime_t timeElapsed;
    
	DiagnosticTestTypes ts  = GetTestType();

    SystemTiming.ComputeElapsedTimeMS( timerValueStopApex, timerValueStartApex_, timeElapsed );
    
	if ( 
            ( timeElapsed < minElapsedTimeApex_ )
         || ( timeElapsed > maxElapsedTimeApex_ ) 
       ) {
       	
       	UINT32 errorCode = (ts << DiagnosticErrorTestTypeBitPos) | errorApexTimer_;

    	OS_Assert( errorCode );
    }
       
    SystemTiming.ComputeElapsedTimeMS( timerValueStopHost, timerValueStartHost_, timeElapsed );
    
	if ( 
            ( timeElapsed < minElapsedTimeHost_ )
         || ( timeElapsed > maxElapsedTimeHost_ ) 
       ) {
       	
       	UINT32 errorCode = (ts << DiagnosticErrorTestTypeBitPos) | errorHostTimer_;

    	OS_Assert( errorCode );
    }

    // Set the start values equal to the stop values to prepare for the next time this diagnostic is run.
    timerValueStartApex_ = timerValueStopApex;
    timerValueStartHost_ = timerValueStopHost;

    // If we get this far, the diagnostic has completed so return DGN_TEST_LOOP_COMPLETE.
    return ( TEST_LOOP_COMPLETE );
}

void BlackfinDiagTimerTest::ConfigureForNextTestCycle() {
}
};


