/// #pragma once
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @file BlackfinDiagDataRam.hpp
///
/// Namespace for the base class for all diagnostic tests
///
/// @par Full Description
/// 
/// Diagnostic testing for the Blackfin data ram regions bank A, bank B and bank C.  Refer to the hardware
//  reference manual for a detailed description of these regions.  The test writes test patterns to RAM and
/// reads them back after writing.  If there are miscompares then an error is reported.  An C callable
//  assembly language routine is called which does the reading and writing for a byte as all the values
//  are saved in registers.  One byte is tested with data patterns at a time, interrupts are disabled during
//  the testing of a byte and re-enabled when a byte is tested.  The test is run in iterations that are 
//  test a number of bytes are passed in when the test object is instantiated.
/// Set and Get methods.
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
#if !defined(BLACKFIN_DIAG_DATA_RAM_HPP)
#define BLACKFIN_DIAG_DATA_RAM_HPP

// SYSTEM INCLUDES

// C PROJECT INCLUDES
#include <ccblkfn.h>                              /* cli/sti( ) */
 
// C++ PROJECT INCLUDES
#include "DiagnosticTesting.hpp"

// FORWARD REFERENCES
// (none)
	
namespace BlackfinDiagnosticTesting 
{
    #define MEMORY_BANK_FAILURE_BIT_POS               30
    #define TEST_PATTERNS_ERROR_BIT_POS               16

    class BlackfinDiagDataRam : public DiagnosticTesting::DiagnosticTest 
    {
        public:      

            // For banks A, B, and C memory regions
            typedef struct 
            {
    	        UINT8 * m_pDataRamAddressStart;
    	        UINT32  m_NmbrContiguousBytesToTest;
    	        UINT32  m_NmbrBytesTested;
    	        BOOL    m_TestCompleted;
            } 
            DataRamTestDescriptor;
            
    
 	        // For testing each byte of RAM
 	        typedef struct 
 	        {
		              UINT8  * m_pByteToTest;
		              UINT8  * m_pPatternThatFailed;
		        const UINT8  * m_pTestPatterns;
		              UINT32   m_NmbrTestPatterns;
	        } 
	        ByteTestParameters;

	        //***************************************************************************
            // PUBLIC METHODS
            //***************************************************************************
            
            ///////////////////////////////////////////////////////////////////////////////////////////////////////////
            ///	METHOD NAME: BlackfinDiagDataRam: BlackfinDiagDataRam
            ///
            /// @par Full Description
            ///      Construction that is used to construct the BlackfinDiagDataRam object that is used to test Blackfin
            ///      data RAM.  It is derived from the DiagnosticTesting::DiagnosticTest base class.
            ///      
            ///
            /// @param bankA                        BankA Data RAM memory region
            ///        bankB                        BankB Data RAM memory region
            ///        bankC                        BankC Data RAM memory region
            ///        pTestPatternsForRamTesting   The array of test patterns to be written and read to each RAM location. 
            ///        nmbrRamTestingPatterns       Size of the the array of test patterns.
            ///        nmbrBytesToTestPerIternation Number of bytes of Ram per iteration of the test.
            ///        ExecuteTestData              Initial runtime data passed to the base for running this test.
            ///                               
            /// @return                             Blackfin Data Ram diagnostic instance created.
            ///
            ///////////////////////////////////////////////////////////////////////////////////////////////////////////
	        BlackfinDiagDataRam( DataRamTestDescriptor      bankA,
	                             DataRamTestDescriptor      bankB,
	                             DataRamTestDescriptor      bankC,
	                             const UINT8 *              pTestPatternsForRamTesting,
	                             UINT32                     nmbrRamTestingPatterns,
	                             UINT32                     nmbrBytesToTestPerIteration,
		    		             DiagnosticTesting::DiagnosticTest::ExecuteTestData &     rTestData )    
						         :  DiagnosticTesting::DiagnosticTest ( rTestData ),
	                       	        m_BankA                           ( bankA ),
	                       	        m_BankB                           ( bankB ),
	                       	        m_BankC                           ( bankC ),
	                       	        m_NmbrBytesToTestPerIteration     ( nmbrBytesToTestPerIteration ),
							        m_NmbrTestPatterns                ( nmbrRamTestingPatterns ),
							        m_pTestPatternsRAM                ( pTestPatternsForRamTesting )//,
	        {
	        }

            ///////////////////////////////////////////////////////////////////////////////////////////////////////////
            ///	METHOD NAME: BlackfinDiagDataRam: RunTest
            ///
            /// @par Full Description
            ///      Provides interface specified by the pure virtual method in the base class.  The scheduler calls 
            ///      this method to run iterations of the diagnostic test..
            ///      
            ///
            /// @param                            Reference to a possible error code returned from the scheduler.
            ///                               
            /// @return                           Status of executing a test
            ///
            ///////////////////////////////////////////////////////////////////////////////////////////////////////////
	        DiagnosticTest::TestState RunTest( UINT32 & ErrorCode  );

	
        protected:

	        //***************************************************************************
            // PROTECTED METHODS
            //***************************************************************************
            
            ///////////////////////////////////////////////////////////////////////////////////////////////////////////
            ///	METHOD NAME: BlackfinDiagDataRam: ConfigureForNextTestCycle
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
	        BlackfinDiagDataRam(const BlackfinDiagDataRam &);
	
	        const BlackfinDiagDataRam & operator = (const BlackfinDiagDataRam & );
	
            BlackfinDiagDataRam();
            
            // An enumeration for cycling through data ram regions for testing
             typedef enum 
	        { 
	        	BANK_A = 1, 
	        	BANK_B = 2, 
	        	BANK_C = 3 
	        } 
	        DataRamMemoryBanks;
		
	        	        	        
            // For saving status of processor when interrupts are disabled and restoring the state when re-enabling.
            INT                        m_Critical; 
            
            // Descriptor for testing individual memory banks.  
            DataRamTestDescriptor      m_BankA;
            
            DataRamTestDescriptor      m_BankB;
            
            DataRamTestDescriptor      m_BankC;                   

	        // Number of test patterns in the test pattern array.
	        UINT32                     m_NmbrTestPatterns;
	
	        // Number of bytes to test per iteration of the test.
	        UINT32                     m_NmbrBytesToTestPerIteration;

	        // The test pattern array
	        const UINT8 *              m_pTestPatternsRAM;

            inline void DisableInterrupts(); 
	
	        inline void EnableInterrupts();
	
	        //***************************************************************************
            // PRIVATE METHODS
            //***************************************************************************
            
            ///////////////////////////////////////////////////////////////////////////////////////////////////////////
            ///	METHOD NAME: BlackfinDiagDataRam: EncodeErrorInfo
            ///
            /// @par Full Description
            ///      Encoding information about a test failure
            ///
            /// @param        rErrorInfo          A reference for passing back error info to the caller.
            ///               memoryBank          The bank where the failure occurred.
            ///               offsetFromBankStart Offset from the start of the memory bank where the failure was detected
            ///               failurePattern      The pattern that caused the failure
            ///                               
            /// @return       None
            ///
            ///////////////////////////////////////////////////////////////////////////////////////////////////////////
            void EncodeErrorInfo( UINT32 &             rErrorInfo, 
                                  DataRamMemoryBanks   memoryBank, 
                                  UINT32               offsetFromBankStart, 
                                  UINT32               failurePattern );
                          
            ///////////////////////////////////////////////////////////////////////////////////////////////////////////
            ///	METHOD NAME: BlackfinDiagDataRam: RunRamTest
            ///
            /// @par Full Description
            ///      Tests an "iternation" amount of RAM.
            ///
            /// @param        pRamDescriptor       Test information about the region being tested.
            ///               rOffsetFromBankStart Upon failure the offset from the start of the bank where test failed
            ///               rFailurePattern      The pattern that caused the failure 
            ///                               
            /// @return       TRUE when test passes
            ///
            ///////////////////////////////////////////////////////////////////////////////////////////////////////////
	        BOOL RunRamTest( DataRamTestDescriptor * pRamDescriptor, 
                             UINT32 &                rOffsetFromBankStart, 
                             UINT32 &                rFailurePattern );
                      
            ///////////////////////////////////////////////////////////////////////////////////////////////////////////
            ///	METHOD NAME: BlackfinDiagDataRam: TestAByte
            ///
            /// @par Full Description
            ///      Tests one byte of RAM
            ///
            /// @param        pbtp          Test information required for testing a byte of RAM.
            ///                               
            /// @return       TRUE when test passes
            ///
            ///////////////////////////////////////////////////////////////////////////////////////////////////////////
	        BOOL TestAByte(ByteTestParameters * pbtp);
    };
};

#endif //!defined(BLACKFIN_DIAG_DATA_RAM_HPP)
