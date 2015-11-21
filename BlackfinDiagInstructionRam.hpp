#pragma once

#include "BlackfinDiagTest.hpp"

namespace BlackfinDiagTesting 
{
    #define BAD_BOOTSTREAM_ERR                    0xffd00000
    #define EMULATION_ACTIVE                      TRUE
    #define UNABLE_TO_START_ERR                   0xfff00000
    #define MISMATCH_ERR                          0xffe00000
    #define BOOT_STREAM_START                     reinterpret_cast<UINT8 *>(0x20040000L)
    #define INSTR_START_ADDR                      reinterpret_cast<const void *>(0xffa00000)
    
    class BlackfinDiagInstructionRam : public BlackfinDiagTest 
    {
	    public:
	        
	        BlackfinDiagInstructionRam( BlackfinExecTestData &     rTestData) 
		            		         :  BlackfinDiagTest           ( rTestData ),
                                        m_pBootStreamStartAddr      ( BOOT_STREAM_START ),
                                        m_pInstructionRamStartAddr ( INSTR_START_ADDR ),
							            m_EmulationActive     ( EMULATION_ACTIVE ) 
	        {
	        }
	
	
            virtual TestState RunTest( UINT32 & ErrorCode );

        protected:

	        virtual void 		ConfigureForNextTestCycle();	

        private:

            #define DMA_BFR_SZ               256	
    
            #define EMUEXCEPT_OPCODE         0x25
    
            #define DMA_COMPLETE_MASK        8
            
            #define INITIAL_HDR_OFFSET       0
	
	        #define INITIAL_BFR_OFFSET       0
	
            #define INITIAL_NUM_BYTES_IN_BFR 0
                       
            UINT8 *                      m_pDmaBfr[ DMA_BFR_SZ ];
    
            typedef struct InstructionComparisonParams 
            { 
	            UINT8          m_InstrMemRead[DMA_BFR_SZ];   // Instruction Memory Read via DMA
                UINT32         m_HeaderOffset;                // Offset from begining of Current Bootstream Header
                UINT32         m_CurrentBfrOffset;            // Current Offset from begining of bootstream data for Instruction Ram comparisons
                UINT32         m_NmbrOfBytesInBuffer;         // Number of Bytes to Compare
                UINT8 *        m_pReadFromAddr;                // Address instruction RAM start for Current DMA buffer
                BOOL           m_EmulationActive;             // Emulator inserts trap instructions at key places to catch breakpoints and issues.
                                                              // Instruction memory read from DMA will not compare.  Therefore to continue testing
                                                              // with the emulator this flag was introduced.
            } 
            InstructionCompareParams;
    
	        InstructionCompareParams     m_IcpCompare;
	
	        BOOL                         m_EmulationActive;
	
            const UINT8 *                m_pBootStreamStartAddr;
            
            const void *                 m_pInstructionRamStartAddr;
                   	//
	        // Inhibit copy construction and assignments of this class by putting the declarations in private portion.
	        // If using C++ 11 and later use the delete keyword to do this.
	        //
	        
            BOOL                         CompareInstructMemToBootStream( InstructionCompareParams &icpCompare );

            BOOL                         ConfigureDMAReadOfInstructionMemory( InstructionCompareParams & rIcp );

            void                         DmaXferMDMA0(void * pReadFromAddress, void * pWriteToAddress);

            BOOL                         EnumerateNextInstructionBootStreamHeader( UINT32 & rHdrOffst, BOOL & rError );   	

            void                         GetBootStreamStartAddr( const UINT8 * & rpBootStreamStartAddr );

	        TestState                    RunInstructionRamTestIteration( InstructionCompareParams & rIcpCompare,
									                                     UINT32 &                   rErrorCode	);
																
	        BOOL                         StartEnumeratingInstructionBootStreamHeaders(UINT32 & rHeaderOffset);

	        BlackfinDiagInstructionRam(const BlackfinDiagInstructionRam &);
	
	        const BlackfinDiagInstructionRam & operator = (const BlackfinDiagInstructionRam & );
		
            BlackfinDiagInstructionRam();
            
	};

};

