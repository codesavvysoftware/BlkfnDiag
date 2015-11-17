#include "BlackfinDiagTimerTest.hpp"
//#include "ApxIcp.hpp"
//#include "ApxParameters.hpp"          
#include "Apex.h"
#include "DiagnosticDefs.h"
#include "Hw.h"

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
        
            UINT64 ullCurrentDspCycles;
            
            _GET_CYCLE_COUNT( ullCurrentDspCycles );
             
            m_HostTimerValueStart = CCLK_TO_US( ullCurrentDspCycles );
        
            m_ApexTimerValueStart = pHI_ApexReg->SystemTime;
        
            return ( TEST_LOOP_COMPLETE );
        }

        // Read the current Apex2 System Time Register value.
        UINT32 apexTimerValueStop = pHI_ApexReg->SystemTime;

        UINT64 ullCurrentDspCycles;
            
        _GET_CYCLE_COUNT( ullCurrentDspCycles );
             
        UINT32 hostTimerValueStop = CCLK_TO_US( ullCurrentDspCycles );
        
        // Calculate the time elapsed according to the previous and current Apex2 System Time Register values read.
        UINT32 apexTimeElapsed = apexTimerValueStop - m_ApexTimerValueStart;

        // Calculate the time elapsed according to the previous and current host timer values read.
        UINT32 hostTimeElapsed = hostTimerValueStop - m_HostTimerValueStart;

        DiagnosticTestTypes ts  = GetTestType();

        // Check if either of the elapsed times are bad.
        if (
                ( hostTimeElapsed < m_MinElapsedTimeHost ) 
             || ( hostTimeElapsed > m_MaxElapsedTimeHost )
           ) 
        {
       	
           	UINT32 errorCode = (ts << DIAG_ERROR_TYPE_BIT_POS) | m_ApexTimerErr;

        	OS_Assert( errorCode );
        }
       
        if (
                ( apexTimeElapsed < m_MinElapsedTimeApex ) 
             || ( apexTimeElapsed > m_MaxElapsedTimeApex ) 
          )
        {
           	UINT32 errorCode = (ts << DIAG_ERROR_TYPE_BIT_POS) | m_HostTimerErr;

        	OS_Assert( errorCode );
        }

        // Set the start values equal to the stop values to prepare for the next time this diagnostic is run.
        m_ApexTimerValueStart = apexTimerValueStop;
        m_HostTimerValueStart = hostTimerValueStop;

        // If we get this far, the diagnostic has completed so return DGN_TEST_LOOP_COMPLETE.
        return ( TEST_LOOP_COMPLETE );
    }

    void BlackfinDiagTimerTest::ConfigureForNextTestCycle() 
    {
    }
};
