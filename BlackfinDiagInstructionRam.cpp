///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @file BlackfinDiagInstructionRam.cpp
///
/// Namespace that contains the class definitions, attributes and methods for the BlackfinDiagInstructionRam class. 
///
/// @see BlackfinDiagInstructionRam.hpp for a detailed description of this class.
///
/// @if REVISION_HISTORY_INCLUDED
/// @par Edit History
/// - [0]  thaley1  01-Dec-2015 Initial revision of file.
/// @endif
///
/// @ingroup Diagnostics
///
/// @par Copyright (c) 2013 Rockwell Automation Technologies, Inc.  All rights reserved.
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// SYSTEM INCLUDES
#include <bfrom.h>

// C PROJECT INCLUDES
#include "Defs.h"
#include "Os_iotk.h"             // This file depends on Defs.h.  It should include that file
#include "Hw.h"                  // Ditto 
#include "Nvs_Obj.h"


// C++ PROJECT INCLUDES
#include "BlackfinDiagInstructionRam.hpp"


// FORWARD REFERENCES
// (none)
//
namespace BlackfinDiagnosticTesting 
{

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////
    ///	METHOD NAME: BlackfinDiagInstructioRam: RunTest
    ///
    ///      Provides interface specified by the pure virtual method in the base class.  The scheduler calls 
    ///      this method to run iterations of the diagnostic test.
    ///
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////
    DiagnosticTesting::DiagnosticTest::TestState BlackfinDiagInstructionRam::RunTest( UINT32 & rErrorCode ) 
    {
	    ConfigForAnyNewDiagCycle( this );
	    
        return RunInstructionRamTestIteration( m_IcpCompare, rErrorCode );
    }
    
    
    //////////////////////////////////////////////////////////////////////////////////////////////////////////
    ///	METHOD NAME: BlackfinDiagInstructionRam: RunInstructionRamTestIteration
    ///
    ///      Run the next iteration of the instruction RAM test.
    ///
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////
    DiagnosticTesting::DiagnosticTest::TestState 
    BlackfinDiagInstructionRam::RunInstructionRamTestIteration(	InstructionCompareParams & rIcpCompare,
                                                                UINT32 &                   rErrorCode ) 
    {
		DiagnosticTesting::DiagnosticTest::TestState ts = DiagnosticTesting::DiagnosticTest::TEST_IN_PROGRESS;

	    BOOL hasError = TRUE;	  
	
	    BOOL isPartialDMABuffer = ConfigureDMAReadOfInstructionMemory( rIcpCompare );
	
	    DmaXferMDMA0( rIcpCompare.m_pReadFromAddr, rIcpCompare.m_InstrMemRead );
	
	    hasError = !CompareInstructMemToBootStream( rIcpCompare ); 

        // Miscomparison results in an error return.
	    if (hasError) 
	    {
		
   		    rErrorCode  = GetTestType() << DiagnosticTesting::DiagnosticTest::DIAG_ERROR_TYPE_BIT_POS;
    
   		    rErrorCode |= MISMATCH_ERR;
   		
   		    void * pCurrentAddr= rIcpCompare.m_pReadFromAddr + rIcpCompare.m_CurrentBfrOffset;
   		
   		    rErrorCode |= reinterpret_cast<UINT32>(pCurrentAddr) & 0xffffff;
   		
   		    ts = DiagnosticTesting::DiagnosticTest::TEST_FAILURE;
	    }
	
	    rIcpCompare.m_CurrentBfrOffset += DMA_BFR_SZ;
		
	    if (isPartialDMABuffer) 
	    {
		
		    hasError = FALSE;
	    	
	        BOOL noMoreHeaders = !EnumerateNextInstructionBootStreamHeader( rIcpCompare.m_HeaderOffset, hasError );
	    	
	        if ( noMoreHeaders ) 
	        {
	        	ts = DiagnosticTesting::DiagnosticTest::TEST_LOOP_COMPLETE;
	        }
	        else if ( hasError ) 
	        {
	    	    rErrorCode  = GetTestType() << DiagnosticTesting::DiagnosticTest::DIAG_ERROR_TYPE_BIT_POS;
	    	
	    	    rErrorCode |= BAD_BOOTSTREAM_ERR;   		
	
	    	    ts = DiagnosticTesting::DiagnosticTest::TEST_FAILURE;
	        }
	    	
	        rIcpCompare.m_CurrentBfrOffset = 0;
	    
	    }
	
        return ts;
    }


    ///////////////////////////////////////////////////////////////////////////////////////////////////////////
    ///	METHOD NAME: BlackfinDiagInstructionRam: DmaXferMDMA0
    ///
    ///      Perform DMA transfer of instruction RAM.  The MDMA0 is used to match up with the convention that
    ///      the AnalogDevices library uses for DMA operations in their sample code..
    ///
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////
    void BlackfinDiagInstructionRam::DmaXferMDMA0(void * pReadFromAddress, void * pWriteToAddress)
    {
        UINT nmbrOf16BitUnitsToDMA = (DMA_BFR_SZ >> 1);
    
        *pMDMA_S0_X_COUNT   	= nmbrOf16BitUnitsToDMA;
        *pMDMA_S0_X_MODIFY  	= 2;  // Number of Bytes per DMA Cycle
        *pMDMA_S0_Y_COUNT   	= 0;
        *pMDMA_S0_Y_MODIFY  	= 0;
        *pMDMA_S0_START_ADDR	= pReadFromAddress;
        *pMDMA_S0_CURR_DESC_PTR	= NULL;
        *pMDMA_S0_NEXT_DESC_PTR	= NULL;
    
    
        *pMDMA_D0_X_COUNT   	= nmbrOf16BitUnitsToDMA;
        *pMDMA_D0_X_MODIFY  	= 2;
        *pMDMA_D0_Y_COUNT   	= 0;
        *pMDMA_D0_Y_MODIFY  	= 0;
        *pMDMA_D0_START_ADDR	= pWriteToAddress;
        *pMDMA_D0_CURR_DESC_PTR	= NULL;
        *pMDMA_D0_NEXT_DESC_PTR	= NULL;

        // Just so DMA can't hang the system but probably a while forever loop should be coded.
        UINT32 loopCntr = 10000000;

        // 
        // Best source for understanding this is the ADSP-BF52X Blackfin Processor Hardware Reference, chapter 6 on DMA.
        // Also Visual DSP has some examples.
        //
        UINT32 statusD0     = *pMDMA_D0_IRQ_STATUS;

        *pMDMA_S0_CONFIG    = NDSIZE_0 | WDSIZE_16       | DMAEN;

        statusD0            = *pMDMA_D0_IRQ_STATUS;

        *pMDMA_D0_CONFIG    = NDSIZE_0 | WDSIZE_16 | WNR | DMAEN;
    
	    while( loopCntr ) 
	    {
		    statusD0 = *pMDMA_D0_IRQ_STATUS;
		
		    if (!(statusD0 & DMA_COMPLETE_MASK )) break;

		    loopCntr--;
	    }
	
	    *pMDMA_S0_CONFIG = 0;	
	    *pMDMA_D0_CONFIG = 0;

	    statusD0 = *pMDMA_D0_IRQ_STATUS; 
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////
    ///	METHOD NAME: BlackfinDiagInstructionRam: StartEnumeratingInstructionBootStreamHeaders
    ///
    ///      Called at the start of a new diagnostic cycle.
    ///
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////
    BOOL BlackfinDiagInstructionRam::StartEnumeratingInstructionBootStreamHeaders( UINT32 & rHeaderOffset ) 
    {
	    const UINT8        * pBootBase = NULL;
    
	    GetBootStreamStartAddr( pBootBase );    
    
	    UINT32  offsetLookahead  = 0;	
	    UINT32  offset           = 0;  
        BOOL    success         = FALSE; 
    
    
        while (TRUE) 
        {    	
    	    if ( offset & (sizeof(UDINT) - 1) ) break; 
    	
    	    ADI_BOOT_HEADER * pHeader = (ADI_BOOT_HEADER *)(pBootBase + offset);
    	
    	    if ( (pHeader->dBlockCode & HDRSGN) != 0xAD000000 ) break;
    	
    	    // Is this the start of instruction memory 
    	    if ( pHeader->pTargetAddress == m_pInstructionRamStartAddr && pHeader->dByteCount ) 
    	    {	
    		    // Yes save the header offset and exit with a true condition 
    		    rHeaderOffset     = offset;
    		    success           = TRUE;    		
    		    offsetLookahead   = offset;
    	        offsetLookahead  += sizeof(ADI_BOOT_HEADER);
    	    
    	        if ( !(pHeader->dBlockCode & BFLAG_FILL) ) 
    	        {
                    offsetLookahead += pHeader->dByteCount;
                }
            
                pHeader = (ADI_BOOT_HEADER *)(pBootBase + offsetLookahead);
           
    		    break;
    	    }
    	    // Have not found the start of the program memory
    	    // Calculate next block address 
    	    offset += sizeof(ADI_BOOT_HEADER);
    	
            if ( !(pHeader->dBlockCode & BFLAG_FILL) ) 
            {
                offset += pHeader->dByteCount;
            }
    	
    	    // Are there more headers to process 
      	    if ( (pHeader->dBlockCode & BFLAG_FINAL) ) break; 
        }

        return success; 
    }


    ///////////////////////////////////////////////////////////////////////////////////////////////////////////
    ///	METHOD NAME: BlackfinDiagInstructionRam: ConfigureDMAReadOfInstructionMemory
    ///
    ///      Configure parameters for reading the next portion of instruction RAM memory to test.
    ///
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////
    BOOL BlackfinDiagInstructionRam::ConfigureDMAReadOfInstructionMemory( InstructionCompareParams & rIcp ) 
    {
        const UINT8     *pBootBase = NULL;
    
        GetBootStreamStartAddr( pBootBase );    
    
   	    ADI_BOOT_HEADER * pHeader = (ADI_BOOT_HEADER *)(pBootBase + rIcp.m_HeaderOffset);
   	        	
	    rIcp.m_pReadFromAddr      = reinterpret_cast<UINT8 *>(pHeader->pTargetAddress) + rIcp.m_CurrentBfrOffset; 	
	
	    BOOL isPartialRead        = TRUE;	
	
	    INT32 bytesLeft           = pHeader->dByteCount;	
	
	    bytesLeft                -= (rIcp.m_CurrentBfrOffset + DMA_BFR_SZ);
	
	
	    if ( bytesLeft >= 0 ) isPartialRead = FALSE;
	
	    if ( isPartialRead ) 
	    {
		    rIcp.m_NmbrOfBytesInBuffer = pHeader->dByteCount - rIcp.m_CurrentBfrOffset;
	    }
	    else 
	    {
		    rIcp.m_NmbrOfBytesInBuffer = DMA_BFR_SZ;
	    }
	
	    return isPartialRead;
    }	
		 
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////
    ///	METHOD NAME: BlackfinDiagInstructionRam: EnumerateNextInstructionBootStreamHeader
    ///
    ///      When the bootstream indicates that there is less than a DMA_BFR_SZ instruction memory left to read,
    ///      this function is called to determine if there is another header with more data or that there is no
    ///      more data to process.
    ///
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////
    BOOL BlackfinDiagInstructionRam::EnumerateNextInstructionBootStreamHeader( UINT32 & rHdrOffst, BOOL & rError ) 
    {  	
        const UINT8              * pBootBase = NULL;
    
        GetBootStreamStartAddr( pBootBase );    
    
	    UINT32             offset   = 0; 
        BOOL               success = FALSE;    
        rError                      = FALSE;
    
        ADI_BOOT_HEADER * pHeader = (ADI_BOOT_HEADER *)(pBootBase + rHdrOffst);

        while( TRUE ) 
    	{
   		    rHdrOffst += sizeof(ADI_BOOT_HEADER);

            if ( !(pHeader->dBlockCode & BFLAG_FILL) ) 
            {
                rHdrOffst += pHeader->dByteCount;
            }

            pHeader = (ADI_BOOT_HEADER *)(pBootBase + rHdrOffst);

            if ( (pHeader->dBlockCode & HDRSGN) != 0xAD000000 ) 
            {
    	        rError = TRUE;
    	    
    	        break;
            }
            else if ( (pHeader->dBlockCode & BFLAG_FINAL) ) 
            {
                success = FALSE;
            
                break;
            }
            else if (    
                         ( pHeader->pTargetAddress >= m_pInstructionRamStartAddr ) 
                      && ( pHeader->dByteCount > 0 ) 
                    )
            {
        	    break;
            }
        } 	 
    
        return success;
    } 


    ///////////////////////////////////////////////////////////////////////////////////////////////////////////
    ///	METHOD NAME: BlackfinDiagInstructionRam: CompareInstructMemToBootStream
    ///
    ///      Compare the instruction RAM contents read via DMA to the contents read from the bootstream for the
    ///      instruction RAM memory under test.
    ///
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////
    BOOL BlackfinDiagInstructionRam::CompareInstructMemToBootStream(InstructionCompareParams & rIcp )
    {
        const UINT8 * pBootStreamStartAddr = NULL;
    
        GetBootStreamStartAddr( pBootStreamStartAddr );    
    
        UINT8 * pInstrMemBootStream          = const_cast<UINT8 *>(pBootStreamStartAddr + rIcp.m_HeaderOffset + sizeof(ADI_BOOT_HEADER) + rIcp.m_CurrentBfrOffset);
        
        UINT8   instrBootStream              = 0;
        UINT8   instrMemRead                 = 0;
        BOOL    checkNextByteForEmulation    = FALSE;
        UINT8 * pPrevInstrMemBootStreamAddr  = NULL;
	    BOOL    success                      = TRUE;
	    UINT8 * pCurrentInstrctnRead         = rIcp.m_InstrMemRead;
    
        for (UINT32 ui32 = 0; ui32 < rIcp.m_NmbrOfBytesInBuffer; ++ui32 ) 
    	{
    	    instrBootStream = *pInstrMemBootStream;
    	    instrMemRead    = *pCurrentInstrctnRead;
    	
    	    if (instrBootStream == instrMemRead) {

    	        ++pCurrentInstrctnRead;    	
    	        ++pInstrMemBootStream;
    		
			    checkNextByteForEmulation = FALSE;

			    continue;
    	    }
    	    else 
    	    {
    		    success = FALSE;
    		
    		    if (rIcp.m_EmulationActive) 
    		    {
    			    if ( EMUEXCEPT_OPCODE == instrMemRead ) 
    			    {
    				    success                     = TRUE;
    				    checkNextByteForEmulation   = TRUE;
    				    pPrevInstrMemBootStreamAddr = pInstrMemBootStream;
    				    success                     = TRUE;
    				
                        ++pCurrentInstrctnRead;    	
                        ++pInstrMemBootStream;
    		
    		
    				    continue;
    			    }
    			    else if (checkNextByteForEmulation) 
    			    {
    				    if ( ( pPrevInstrMemBootStreamAddr + 1 ) == (pInstrMemBootStream) ) 
    					{
    					    checkNextByteForEmulation = FALSE;
    					    success                   = TRUE;
    					
    	                    ++pCurrentInstrctnRead;
             	            ++pInstrMemBootStream;
    		
    					    continue;
    				    }
    			    }   			
    		    }   		    
   		        else 
   		        {
   		        	break;
   		        }
            }
        }
    

        return success; 
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////
    ///	METHOD NAME: BlackfinDiagInstructionRam: GetBootStreamStartAddr
    ///
    ///      Get the start address of the bootstream.
    ///
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////
    void BlackfinDiagInstructionRam::GetBootStreamStartAddr( const UINT8 * & rBootStreamStartAddr ) 
    {
		rBootStreamStartAddr = m_pBootStreamStartAddr;
    }
   


    ///////////////////////////////////////////////////////////////////////////////////////////////////////////
    ///	METHOD NAME: BlackfinDiagInstructionRam: ConfigureForNextTestCycle
    ///
    ///      Provides interface specified by the pure virtual method in the base class. This method is called 
    ///      at that start of testing for the test during a new diagnostics cycle. The data that needs to be 
    ///      initialized for an individual test is initialized.      
    ///
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////
    void BlackfinDiagInstructionRam::ConfigureForNextTestCycle() 
    {
        m_IcpCompare.m_HeaderOffset           = INITIAL_HDR_OFFSET;
	
	    m_IcpCompare.m_CurrentBfrOffset       = INITIAL_BFR_OFFSET;
	
        m_IcpCompare.m_NmbrOfBytesInBuffer    = INITIAL_NUM_BYTES_IN_BFR;
	
	    m_IcpCompare.m_pReadFromAddr          = NULL;
	
    	m_IcpCompare.m_EmulationActive        = m_EmulationActive;
	
	    BOOL enumerationNotStarted = !StartEnumeratingInstructionBootStreamHeaders( m_IcpCompare.m_HeaderOffset );
	
	    if ( enumerationNotStarted) 
	    {
		    //
    	    // An error report it to scheduler
    	    //
    	    UINT32 errorCode  = GetTestType() << DiagnosticTesting::DiagnosticTest::DIAG_ERROR_TYPE_BIT_POS;
    
    	    errorCode |= UNABLE_TO_START_ERR;
    		
		    OS_Assert( errorCode );
   	    }    	
    }

};

