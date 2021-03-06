///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @file BlackfinDiagInstructionsTest.cpp
///
/// Namespace that contains the class definitions, attributes and methods for the BlackfinDiagInstructionsTest class. 
///
/// @see BlackfinDiagInstructionsTest.hpp for a detailed description of this class.
///
/// @if REVISION_HISTORY_INCLUDED
/// @par Edit History
/// - [0]  thaley1  01-Dec-2015 Initial revision of file.
/// @endif
///
/// @ingroup Diagnostics
///
/// @par Copyright (c) 2015 Rockwell Automation Technologies, Inc.  All rights reserved.
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// SYSTEM INCLUDES
// (none)

// C PROJECT INCLUDES
#include "Defs.h"
#include "Os_iotk.h"             // This file depends on Defs.h.  It should include that file
#include "Hw.h"                  // Ditto 


// C++ PROJECT INCLUDES
#include "BlackfinDiagInstructionsTest.hpp"


// FORWARD REFERENCES
// (none)
//

namespace BlackfinDiagnosticTesting 
{
    // Linkage for assembly language C callable subroutine.
    extern "C" UINT32  BlackfinDiagInstrTest();

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////
    ///	METHOD NAME: BlackfinDiagInstructiosTest: RunTest
    ///
    ///      Provides interface specified by the pure virtual method in the base class.  The scheduler calls 
    ///      this method to run iterations of the diagnostic test..
    ///
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////
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
    
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////
    ///	METHOD NAME: BlackfinDiagInstructionsTest: ConfigureForNextTestCycle
    ///
    ///      Provides interface specified by the pure virtual method in the base class. This method is called 
    ///      at that start of testing for the test during a new diagnostics cycle. The data that needs to be 
    ///      initialized for an individual test is initialized.      
    ///
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////
    void BlackfinDiagInstructionsTest::ConfigureForNextTestCycle() 
    {
    }
};
