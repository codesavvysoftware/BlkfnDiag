/// #pragma once
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @file BlackfinDiagInstructionRam.hpp
///
/// Namespace for the base class for all diagnostic tests
///
/// @par Full Description
/// 
/// Diagnostic testing for the Blackfin instruction RAM.  Instruction RAM is not directly addressable and thus must be
/// read via DMA.  The intruction memory read is compared to the instruction data in the bootstream flash and an error
/// occurs when there is a mismatch.
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
#if !defined(BLACKFIN_DIAG_INSTRUCTION_RAM_HPP)
#define BLACKFIN_DIAG_INSTRUCTION_RAM_HPP

// SYSTEM INCLUDES

// C PROJECT INCLUDES
#include "Nvs_Obj.h"
 
// C++ PROJECT INCLUDES
#include "DiagnosticTesting.hpp"

// FORWARD REFERENCES
// (none)

namespace BlackfinDiagnosticTesting 
{
    static const UINT32  BAD_BOOTSTREAM_ERR  = 0xffd00000;
    static const UINT32  UNABLE_TO_START_ERR = 0xfff00000;
    static const UINT32  MISMATCH_ERR        = 0xffe00000;
    static const UINT8 * BOOT_STREAM_START   = reinterpret_cast<UINT8 *>NVS_MAIN_START_ADDR;
    static const void *  INSTR_START_ADDR    = reinterpret_cast<const void *>(0xffa00000);
    
    class BlackfinDiagInstructionRam : public DiagnosticTesting::DiagnosticTest 
    {
	    public:
	        
	        //***************************************************************************
            // PUBLIC METHODS
            //***************************************************************************
            
            ///////////////////////////////////////////////////////////////////////////////////////////////////////////
            ///	METHOD NAME: BlackfinDiagInstructionRam: BlackfinDiagInstructionRam
            ///
            /// @par Full Description
            ///      Construction that is used to construct the BlackfinDiagInstructionRam object that is used to test Blackfin
            ///      instruction RAM.  It is derived from the DiagnosticTesting::DiagnosticTest base class.
            ///      
            ///
            /// @param ExecuteTestData              Initial runtime data passed to the base for running this test.
            ///                               
            /// @return                             Blackfin Instruction Ram diagnostic instance created.
            ///
            ///////////////////////////////////////////////////////////////////////////////////////////////////////////
	        BlackfinDiagInstructionRam( DiagnosticTesting::DiagnosticTest::ExecuteTestData &     rTestData) 
		            		         :  DiagnosticTest              ( rTestData ),
                                        m_pBootStreamStartAddr      ( BOOT_STREAM_START ),
                                        m_pInstructionRamStartAddr  ( INSTR_START_ADDR )
	        {
	        }
	
	
            ///////////////////////////////////////////////////////////////////////////////////////////////////////////
            ///	METHOD NAME: BlackfinDiagInstructioRam: RunTest
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
            DiagnosticTesting::DiagnosticTest::TestState RunTest( UINT32 & ErrorCode );

        protected:

	        //***************************************************************************
            // PROTECTED METHODS
            //***************************************************************************
            
            ///////////////////////////////////////////////////////////////////////////////////////////////////////////
            ///	METHOD NAME: BlackfinDiagInstructionRam: ConfigureForNextTestCycle
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
	        void 		ConfigureForNextTestCycle();	

        private:

            // Size of DMA buffer
            #define DMA_BFR_SZ               256	
    
            // Exception Instruction Opcode
            #define EMUEXCEPT_OPCODE         0x25
    
            // For determining when DMA is complete
            #define DMA_COMPLETE_MASK        8
            
            // For reading boot stream data
            #define INITIAL_HDR_OFFSET       0
	
	        // For processing instruction ram via DMA
	        #define INITIAL_BFR_OFFSET       0
	
            #define INITIAL_NUM_BYTES_IN_BFR 0
                       
            // DMA data containing instruction RAM is read into this buffer.
            UINT8 *                      m_pDmaBfr[ DMA_BFR_SZ ];
    
            // For keeping track of the progress of instruction RAM testing.
            typedef struct InstructionComparisonParams 
            { 
	            UINT8          m_InstrMemRead[DMA_BFR_SZ];   // Instruction Memory Read via DMA
                UINT32         m_HeaderOffset;                // Offset from begining of Current Bootstream Header
                UINT32         m_CurrentBfrOffset;            // Current Offset from begining of bootstream data for Instruction Ram comparisons
                UINT32         m_NmbrOfBytesInBuffer;         // Number of Bytes to Compare
                UINT8 *        m_pReadFromAddr;                // Address instruction RAM start for Current DMA buffer
            } 
            InstructionCompareParams;
    
	        // Current status data for testing instruction RAM
	        InstructionCompareParams     m_IcpCompare;
	
            // Where the bootstream is located in FLASH
            const UINT8 *                m_pBootStreamStartAddr;
            
            // Contains address of where instruction RAM testing begins at.
            const void *                 m_pInstructionRamStartAddr;
                   	//
	        // Inhibit copy construction and assignments of this class by putting the declarations in private portion.
	        // If using C++ 11 and later use the delete keyword to do this.
	        //
	        BlackfinDiagInstructionRam(const BlackfinDiagInstructionRam &);
	
	        const BlackfinDiagInstructionRam & operator = (const BlackfinDiagInstructionRam & );
		
            BlackfinDiagInstructionRam();
            
	        //***************************************************************************
            // PRIVATE METHODS
            //***************************************************************************
            
            ///////////////////////////////////////////////////////////////////////////////////////////////////////////
            ///	METHOD NAME: BlackfinDiagInstructionRam: CompareInstructMemToBootStream
            ///
            /// @par Full Description
            ///      Compare the instruction RAM contents read via DMA to the contents read from the bootstream for the
            ///      instruction RAM memory under test.
            ///
            /// @param        rIcpCompare         Data used to cycle through all of the Blackfin instruction RAM 
            ///                                   iteratively.
            ///                               
            /// @return       TRUE when there are no miscomparisons between the bootstream and the instruction RAM
            ///
            ///////////////////////////////////////////////////////////////////////////////////////////////////////////
            BOOL                                         CompareInstructMemToBootStream( InstructionCompareParams &rIcpCompare );

            ///////////////////////////////////////////////////////////////////////////////////////////////////////////
            ///	METHOD NAME: BlackfinDiagInstructionRam: ConfigureDMAReadOfInstructionMemory
            ///
            /// @par Full Description
            ///      Configure parameters for reading the next portion of instruction RAM memory to test.
            ///
            /// @param        rIcp                Data used to cycle through all of the Blackfin instruction RAM 
            ///                                   iteratively.
            ///                               
            /// @return       TRUE when successfully configuring the test for the next portion of instruction RAM
            ///
            ///////////////////////////////////////////////////////////////////////////////////////////////////////////
            BOOL                                         ConfigureDMAReadOfInstructionMemory( InstructionCompareParams & rIcp );

            ///////////////////////////////////////////////////////////////////////////////////////////////////////////
            ///	METHOD NAME: BlackfinDiagInstructionRam: DmaXferMDMA0
            ///
            /// @par Full Description
            ///      Perform DMA transfer of instruction RAM.  The MDMA0 is used to match up with the convention that
            ///      the AnalogDevices library uses for DMA operations in their sample code..
            ///
            /// @param        pReadFromAddress    Instruction RAM address to read from. 
            ///               pWriteToAddress     RAM buffer where data is transferred to.
            ///                               
            /// @return       None
            ///
            ///////////////////////////////////////////////////////////////////////////////////////////////////////////
            void                                         DmaXferMDMA0(void * pReadFromAddress, void * pWriteToAddress);

            ///////////////////////////////////////////////////////////////////////////////////////////////////////////
            ///	METHOD NAME: BlackfinDiagInstructionRam: EnumerateNextInstructionBootStreamHeader
            ///
            /// @par Full Description
            ///      When the bootstream indicates that there is less than a DMA_BFR_SZ instruction memory left to read,
            ///      this function is called to determine if there is another header with more data or that there is no
            ///      more data to process.
            ///
            /// @param        rHdrOffst           New offset into bootstream if there is more data. 
            ///               rError              TRUE when the bootstream is in error.
            ///                               
            /// @return       TRUE when there is more instruction RAM to process.
            ///
            ///////////////////////////////////////////////////////////////////////////////////////////////////////////
            BOOL                                         EnumerateNextInstructionBootStreamHeader( UINT32 & rHdrOffst, BOOL & rError );   	

            ///////////////////////////////////////////////////////////////////////////////////////////////////////////
            ///	METHOD NAME: BlackfinDiagInstructionRam: GetBootStreamStartAddr
            ///
            /// @par Full Description
            ///      Get the start address of the bootstream.
            ///
            /// @param        rpBootStreamStartAddr    Start address of the bootstream
            ///
            /// @return       None.
            ///
            ///////////////////////////////////////////////////////////////////////////////////////////////////////////
            void                                         GetBootStreamStartAddr( const UINT8 * & rpBootStreamStartAddr );

            ///////////////////////////////////////////////////////////////////////////////////////////////////////////
            ///	METHOD NAME: BlackfinDiagInstructionRam: RunInstructionRamTestIteration
            ///
            /// @par Full Description
            ///      Run the next iteration of the instruction RAM test.
            ///
            /// @param        rIcpCompare    Contains parameters for running the next interation
            ///               rErrorCode     Encoded error information if test detects a failure.
            ///
            /// @return       TestState      status of the test.
            ///
            ///////////////////////////////////////////////////////////////////////////////////////////////////////////
			DiagnosticTesting::DiagnosticTest::TestState RunInstructionRamTestIteration( InstructionCompareParams & rIcpCompare,
							                                                                UINT32 &                   rErrorCode	);
																
            ///////////////////////////////////////////////////////////////////////////////////////////////////////////
            ///	METHOD NAME: BlackfinDiagInstructionRam: StartEnumeratingInstructionBootStreamHeaders
            ///
            /// @par Full Description
            ///      Called at the start of a new diagnostic cycle.
            ///
            /// @param        rHeaderOffset  Offset into bootstream header
            ///
            /// @return       TRUE when bootstream is valid.
            ///
            ///////////////////////////////////////////////////////////////////////////////////////////////////////////
	        BOOL                                         StartEnumeratingInstructionBootStreamHeaders(UINT32 & rHeaderOffset);

	};

};

#endif //!defined(BLACKFIN_DIAG_INSTRUCTION_RAM_HPP)
