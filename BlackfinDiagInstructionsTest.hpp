/// #pragma once
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @file BlackfinDiagInstructionsTest.hpp
///
/// Namespace for the base class for all diagnostic tests
///
/// @par Full Description
/// 
/// Diagnostic testing for the Blackfin instructions.  The idea is to test those instructions that are not executed as
/// part of the diagnostic testing but are executed as part of the application.
///
///
/// @if REVISION_HISTORY_INCLUDED
/// @par Edit History
/// - [0]  thaley1  01-Dec-2015 Initial revision of file.
/// @endif
///
/// @ingroup Diagnostics
///
/// @par Copyright (c) 2015 Rockwell Automation Technologies, Inc.  All rights reserved.
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#if !defined(BLACKFIN_DIAG_INSTRUCTIONS_HPP)
#define BLACKFIN_DIAG_INSTRUCTIONS_HPP

// SYSTEM INCLUDES

// C PROJECT INCLUDES
// (none)
 
// C++ PROJECT INCLUDES
#include "DiagnosticTesting.hpp"

// FORWARD REFERENCES
// (none)
namespace BlackfinDiagnosticTesting 
{
    class BlackfinDiagInstructionsTest : public DiagnosticTesting::DiagnosticTest 
    {

        public:
	        //***************************************************************************
            // PUBLIC METHODS
            //***************************************************************************
            
            ///////////////////////////////////////////////////////////////////////////////////////////////////////////
            ///	METHOD NAME: BlackfinDiagInstructionsTest: BlackfinDiagInstructionsTest
            ///
            /// @par Full Description
            ///      Construction that is used to construct the BlackfinDiagInstructionsTest object that is used to test 
            ///      Blackfin instructions.  It is derived from the DiagnosticTesting::DiagnosticTest base class.
            ///      
            ///
            /// @param ExecuteTestData              Initial runtime data passed to the base for running this test.
            ///                               
            /// @return                             Blackfin instructions test diagnostic instance created.
            ///
            ///////////////////////////////////////////////////////////////////////////////////////////////////////////
            BlackfinDiagInstructionsTest( DiagnosticTesting::DiagnosticTest::ExecuteTestData &  rTestData ) 
		    		     		:	DiagnosticTesting::DiagnosticTest             	  ( rTestData )
									
        	{
        	}

            ///////////////////////////////////////////////////////////////////////////////////////////////////////////
            ///	METHOD NAME: BlackfinDiagInstructionssTest: RunTest
            ///
            /// @par Full Description
            ///      Provides interface specified by the pure virtual method in the base class.  The scheduler calls 
            ///      this method to run iterations of the diagnostic test.
            ///      
            ///
            /// @param                            Reference to a possible error code returned from the scheduler.
            ///                               
            /// @return                           Status of executing a test
            ///
            ///////////////////////////////////////////////////////////////////////////////////////////////////////////
        	DiagnosticTesting::DiagnosticTest::TestState RunTest( UINT32 & rErrorCode );

        protected:

	        //***************************************************************************
            // PROTECTED METHODS
            //***************************************************************************
            
            ///////////////////////////////////////////////////////////////////////////////////////////////////////////
            ///	METHOD NAME: BlackfinDiagInstructionsTest: ConfigureForNextTestCycle
            ///
            /// @par Full Description
            ///      Provides interface specified by the pure virtual method in the base class. This method is called 
            ///      at that start of testing for the test during a new diagnostics cycle. The data that needs to be 
            ///      initialized for an individual test is initialized.      
            ///
            /// @param                            None.
            ///                               
            /// @return                           None
            ///
            ///////////////////////////////////////////////////////////////////////////////////////////////////////////
        	void ConfigureForNextTestCycle();
	
        private:
	        // Inhibit copy construction and assignments of this class by putting the declarations in private portion.
	        // If using C++ 11 and later use the delete keyword to do this.
	        //
	        BlackfinDiagInstructionsTest(const BlackfinDiagInstructionsTest &);
	
	        const BlackfinDiagInstructionsTest & operator = (const BlackfinDiagInstructionsTest & );
		
            BlackfinDiagInstructionsTest();
            
    };
};
#endif //#if !defined(BLACKFIN_DIAG_INSTRUCTIONS_HPP)
