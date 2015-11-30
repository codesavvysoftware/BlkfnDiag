/// #pragma once
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @file BlackfinDiagRegistersTest.hpp
///
/// Namespace for the base class for all diagnostic tests
///
/// @par Full Description
/// 
/// Diagnostic testing for the Blackfin registers.  The idea is to test a basic set of registers that are scratchpad
/// for C callable subroutines.  Then after the basic set of registers passes, use those registers to test the 
/// rest of the register file.
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
#if !defined(BLACKFIN_DIAG_REGISTERS_TEST_HPP)
#define BLACKFIN_DIAG_REGISTERS_TEST_HPP

// SYSTEM INCLUDES

// C PROJECT INCLUDES
#include <ccblkfn.h>                              /* Added for ssync( ), cli/sti( ) */
 
// C++ PROJECT INCLUDES
#include "DiagnosticTesting.hpp"

// FORWARD REFERENCES
// (none)
#include "DiagnosticTesting.hpp"


namespace BlackfinDiagnosticTesting 
{
    #define CORRUPTED_REG_TST 0xff
	
    class BlackfinDiagRegistersTest : public DiagnosticTesting::DiagnosticTest 
    {

        public:

            typedef UINT32 (* const pRegisterTest)();
	
	        //***************************************************************************
            // PUBLIC METHODS
            //***************************************************************************
            
            ///////////////////////////////////////////////////////////////////////////////////////////////////////////
            ///	METHOD NAME: BlackfinDiagRegistersTest: BlackfinDiagInstructionsTest
            ///
            /// @par Full Description
            ///      Construction that is used to construct the BlackfinDiagRegistersTest object that is used to test Blackfin
            ///      registers.  It is derived from the DiagnosticTesting::DiagnosticTest base class.
            ///      
            ///
            /// @param ExecuteTestData              Initial runtime data passed to the base for running this test.
            ///                               
            /// @return                             Blackfin registers test diagnostic instance created.
            ///
            ///////////////////////////////////////////////////////////////////////////////////////////////////////////
			BlackfinDiagRegistersTest( DiagnosticTesting::DiagnosticTest::ExecuteTestData &  rTestData ) 
		    		     		:	   DiagnosticTesting::DiagnosticTest ( rTestData ),
									   m_SanityTestRan                   ( FALSE ),
									   m_RegisterTestRan                 ( FALSE ) 
        	{
        	}

            ///////////////////////////////////////////////////////////////////////////////////////////////////////////
            ///	METHOD NAME: BlackfinDiagRegisterssTest: RunTest
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
            ///	METHOD NAME: BlackfinDiagRegistersTest: ConfigureForNextTestCycle
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
        	//
        	// Inhibit copy construction and assignments of this class by putting the declarations in private portion.
        	// If using C++ 11 and later use the delete keyword to do this.
        	//
        	BlackfinDiagRegistersTest(const BlackfinDiagRegistersTest &);
	
        	const BlackfinDiagRegistersTest & operator = (const BlackfinDiagRegistersTest & );
	
            BlackfinDiagRegistersTest();
           
        	// True when Sanity Test has run
        	BOOL                              m_SanityTestRan;
        	
        	// True when all registers have been tested.
        	BOOL                              m_RegisterTestRan;
    };
};
#endif //#if !defined(BLACKFIN_DIAG_REGISTERS_TEST_HPP)





