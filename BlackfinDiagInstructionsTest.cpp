#include "BlackfinDiagInstructionsTest.hpp"
#include "Os_iotk.h"
#include "Hw.h"
namespace BlackfinDiagnosticTesting 
{
	
    DiagnosticTesting::DiagnosticTest::TestState BlackfinDiagInstructionsTest::RunTest( UINT32 & rErrorCode ) 
    {
        UINT32 error = BlackfinDiagInstrTest();
        
        if ( error )
        {
    	    error &= DiagnosticTesting::DiagnosticTest::DIAG_ERROR_MASK;
			
    		error |= ( GetTestType() << DiagnosticTesting::DiagnosticTest::DIAG_ERROR_TYPE_BIT_POS );
			
    	    OS_Assert( error );
        }
        
    	return DiagnosticTesting::DiagnosticTest::TEST_LOOP_COMPLETE;
    }
    
    void BlackfinDiagInstructionsTest::ConfigureForNextTestCycle() 
    {
    }
};
