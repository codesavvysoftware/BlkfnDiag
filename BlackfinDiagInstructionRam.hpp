#pragma once

#include "BlackfinDiag.hpp"
#include "BlackfinDiagTest.hpp"
#include <vector>

namespace BlackfinDiagTesting 
{
	static const UINT32   BAD_BOOTSTREAM_ERR  = 0xffd00000;
	static const UINT32   UNABLE_TO_START_ERR = 0xfff00000;
	static const UINT32   MISMATCH_ERR        = 0xffe00000;
	
    class BlackfinDiagInstructionRam : public BlackfinDiagTest 
    {
	    public:
	        
	        BlackfinDiagInstructionRam( BlackfinExecTestData &     rTestData ) 
		            		         :  BlackfinDiagTest     ( rTestData ),
                                        m_BadBootstreamErr   ( BAD_BOOTSTREAM_ERR ),
                                        m_UnableToStartErr   ( UNABLE_TO_START_ERR ),
                                        m_MismatchErr        ( MISMATCH_ERR ),
							            m_EmulationActive    ( TRUE ) 
	        {
	        }
	
	
	        virtual ~BlackfinDiagInstructionRam()
	        {
	        }

            virtual TestState RunTest( UINT32 & ErrorCode );

        protected:

	        virtual void 		ConfigureForNextTestCycle();	

        private:

            static const UINT32 	     DMA_BFR_SZ               = 256;	
    
            static const UINT32          EMUEXCEPT_OPCODE         = 0x25;
    
            static const UINT32          DMA_COMPLETE_MASK        = 8;
            
            static const UINT32          INITIAL_HDR_OFFSET       = 0;
	
	        static const UINT32          INITIAL_BFR_OFFSET       = 0;
	
            static const UINT32          INITIAL_NUM_BYTES_IN_BFR = 0;
                       
            static const UINT8 *         BOOT_STREAM_START;
    
            static const void *          INSTR_START_ADDR;
            
            UINT8 *                      m_pDmaBfr[ DMA_BFR_SZ ];
    
            typedef struct InstructionComparisonParams 
            { 
	            UINT8          m_InstrMemRead[DMA_BFR_SZ];   // Instruction Memory Read via DMA
                UINT32         m_HeaderOffset;                // Offset from begining of Current Bootstream Header
                UINT32         m_CurrentBfrOffset;            // Current Offset from begining of bootstream data for Instruction Ram comparisons
                UINT32         m_NmbrOfBytesInBuffer;         // Number of Bytes to Compare
                UINT8 *        m_pReadFromAddr;                // Address instruction RAM start for Current DMA buffer
                BOOL           m_ScaffoldingActive;           // Active Status of Scaffolding for debugging.
                BOOL           m_EmulationActive;             // Emulator inserts trap instructions at key places to catch breakpoints and issues.
                                                              // Instruction memory read from DMA will not compare.  Therefore to continue testing
                                                              // with the emulator this flag was introduced.
            } 
            InstructionCompareParams;
    
	        const UINT32                 m_BadBootstreamErr;
    		
	        const UINT32                 m_UnableToStartErr;
	
	        const UINT32                 m_MismatchErr;
	        
	        InstructionCompareParams     m_IcpCompare;
	
	        BOOL                         m_EmulationActive;
	
	        BOOL                         m_ScaffoldingActive;
	
        	//
	        // Inhibit copy construction and assignments of this class by putting the declarations in private portion.
	        // If using C++ 11 and later use the delete keyword to do this.
	        //
	        BlackfinDiagInstructionRam(const BlackfinDiagInstructionRam &);
	
	        const BlackfinDiagInstructionRam & operator = (const BlackfinDiagInstructionRam & );
	
            BOOL                         CompareInstructMemToBootStream( InstructionCompareParams &icpCompare );

            BOOL                         ConfigureDMAReadOfInstructionMemory( InstructionCompareParams & rIcp );

            void                         DmaXferMDMA0(void * pReadFromAddress, void * pWriteToAddress);

            BOOL                         EnumerateNextInstructionBootStreamHeader( UINT32 & rHdrOffst, BOOL & rError );   	

            void                         GetBootStreamStartAddr( const UINT8 * & rpBootStreamStartAddr );

	        BlackfinDiagTest::TestState  RunInstructionRamTestIteration( InstructionCompareParams & rIcpCompare,
									                                     UINT32 &                   rErrorCode	);
																
	        BOOL                         StartEnumeratingInstructionBootStreamHeaders(UINT32 & rHeaderOffset);
	};

};

