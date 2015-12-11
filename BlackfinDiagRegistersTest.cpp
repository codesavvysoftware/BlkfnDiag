///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @file BlackfinDiagRegistersTest.cpp
///
/// Namespace that contains the class definitions, attributes and methods for the BlackfinDiagRegistersTest class. 
///
/// @see BlackfinDiagRegistersTest.hpp for a detailed description of this class.
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
#include "BlackfinDiagRegistersTest.hpp"


// FORWARD REFERENCES
// (none)
//


namespace BlackfinDiagnosticTesting 
{
      //
    // For linkage to c callable assembly language register tests
    //
    extern "C" UINT32  BlackfinDiagRegSanityChk();
    extern "C" UINT32  BlackfinDiagRegChk();

	
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////
    ///	METHOD NAME: BlackfinDiagRegistersTest: RunTest
    ///
    ///      Provides interface specified by the pure virtual method in the base class.  The scheduler calls 
    ///      this method to run iterations of the diagnostic test..
    ///
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////
    DiagnosticTesting::DiagnosticTest::TestState BlackfinDiagRegistersTest::RunTest( UINT32 & rErrorCode ) 
    {
        ConfigForAnyNewDiagCycle( this );
					
       	DiagnosticTesting::DiagnosticTest::TestState result = DiagnosticTest::TEST_IN_PROGRESS;
    	
    	if ( !m_SanityTestRan ) 
    	{
        	rErrorCode = BlackfinDiagRegSanityChk();
        	
        	m_SanityTestRan = TRUE;
    	
        	if ( rErrorCode ) 
        	{
  		
    			rErrorCode &= DiagnosticTesting::DiagnosticTest::DIAG_ERROR_MASK;
			
    			rErrorCode |= ( GetTestType() << DiagnosticTesting::DiagnosticTest::DIAG_ERROR_TYPE_BIT_POS );
			
    			OS_Assert( rErrorCode );
        	}
        	
       	}
       	else
       	{
       	    rErrorCode = BlackfinDiagRegChk();
       	    
       	    m_RegisterTestRan = TRUE;
       	    
        	if ( rErrorCode ) 
        	{
  		
    			rErrorCode &= DiagnosticTesting::DiagnosticTest::DIAG_ERROR_MASK;
			
    			rErrorCode |= ( GetTestType() << DiagnosticTesting::DiagnosticTest::DIAG_ERROR_TYPE_BIT_POS );
			
    			OS_Assert( rErrorCode );
        	}
       
        	result = DiagnosticTesting::DiagnosticTest::TEST_LOOP_COMPLETE;
       	}
       	
       	return result;
    }
           	    
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////
    ///	METHOD NAME: BlackfinDiagRegistersTest: ConfigureForNextTestCycle
    ///
    ///      Provides interface specified by the pure virtual method in the base class. This method is called 
    ///      at that start of testing for the test during a new diagnostics cycle. The data that needs to be 
    ///      initialized for an individual test is initialized.      
    ///
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////
    void BlackfinDiagRegistersTest::ConfigureForNextTestCycle() 
    {
        m_SanityTestRan   = FALSE;
        
        m_RegisterTestRan = FALSE;
    } 
	
};



	
