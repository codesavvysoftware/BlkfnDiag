#include "BlackfinDiagInstructionRam.h"

#include <vector>


#include "Nvs_Obj.h"
#include <bfrom.h>

TestState BlackfinDiagInstructionRam::RunTest(DiagControlBlock * dcb) {
    static InstructionCompareParams icpCompare;

    icpCompare.HeaderOffset          = 0;
    icpCompare.CurrentBfrOffset      = 0;
    icpCompare.NumberOfBytesInBuffer = 0;
    icpCompare.pReadFromAddr         = NULL;
    icpCompare.bScaffoldingActive    = bScaffoldingActive;
    icpCompare.bEmulationActive      = bEmulationActive;
 	
    BOOL bEnumerationNotStarted = !StartEnumeratingInstructionBootStreamHeaders( icpCompare.HeaderOffset );
	  
    if (bEnumerationNotStarted) return TEST_LOOP_COMPLETE;
 	
	BOOL bError = TRUE;	  
	  
	while(TRUE) {
		
 	  	BOOL bPartialDMABuffer = ConfigureDMAReadOfInstructionMemory( icpCompare );
	  	

	    DMA_Xfer_MDMA0( icpCompare.pReadFromAddr, icpCompare.pInstrMemRead );
	    
	    bError = !CompareInstructMemToBootStream( icpCompare ); 

		icpCompare.CurrentBfrOffset += DMA_BFR_SZ;
		
        if (icpCompare.bScaffoldingActive) {
        	//
        	// If scaffolding is active record all mismatches. Resetting error will result in reading
        	// all of the instruction RAM.
        	//
        	if (bError) bError = FALSE;
        }

        // Miscomparison results in an error return.
	    if (bError) break;
	    
	    if (bPartialDMABuffer) {
	    	bError = FALSE;
	    	
	    	BOOL bNoMoreHeaders = 	!EnumerateNextInstructionBootStreamHeader( icpCompare.HeaderOffset, bError );
	    	
	    	if (bNoMoreHeaders) {
	    		break;
	    	}
	    	
	    	if (bError) {
	    		break;
	    	}
	    	
	    	icpCompare.CurrentBfrOffset = 0;
	    }
	    
	}
	
   	UINT32 uiNumberMismatched = 0;

   	if (icpCompare.bScaffoldingActive) {
   		EnumerateMismatched(uiNumberMismatched);
    }
      
	return TEST_LOOP_COMPLETE; 
 
}

void BlackfinDiagInstructionRam::DMA_Xfer_MDMA0(void * read_from_address, void * write_to_address)
{
    UINT NumberOf16BitUnitsToDMA = (DMA_BFR_SZ >> 1);
    
    *pMDMA_S0_X_COUNT   	= NumberOf16BitUnitsToDMA;
    *pMDMA_S0_X_MODIFY  	= 2;  // Number of Bytes per DMA Cycle
    *pMDMA_S0_Y_COUNT   	= 0;
    *pMDMA_S0_Y_MODIFY  	= 0;
    *pMDMA_S0_START_ADDR	= read_from_address;
    *pMDMA_S0_CURR_DESC_PTR	= NULL;
    *pMDMA_S0_NEXT_DESC_PTR	= NULL;
    
    
    *pMDMA_D0_X_COUNT   	= NumberOf16BitUnitsToDMA;
    *pMDMA_D0_X_MODIFY  	= 2;
    *pMDMA_D0_Y_COUNT   	= 0;
    *pMDMA_D0_Y_MODIFY  	= 0;
    *pMDMA_D0_START_ADDR	= write_to_address;
    *pMDMA_D0_CURR_DESC_PTR	= NULL;
    *pMDMA_D0_NEXT_DESC_PTR	= NULL;

    // Just so DMA can't hang the system but probably a while forever loop should be coded.
    UINT32 loop_cntr = 10000000;

    // 
    // Best source for understanding this is the ADSP-BF52X Blackfin Processor Hardware Reference, chapter 6 on DMA.
    // Also Visual DSP has some examples.
    //
    UINT32 status_D0 = *pMDMA_D0_IRQ_STATUS;

    *pMDMA_S0_CONFIG    = NDSIZE_0 | WDSIZE_16       | DMAEN;

    status_D0 = *pMDMA_D0_IRQ_STATUS;

    *pMDMA_D0_CONFIG    = NDSIZE_0 | WDSIZE_16 | WNR | DMAEN;
    
	while( loop_cntr ) {
		
		status_D0 = *pMDMA_D0_IRQ_STATUS;
		
		if (!(status_D0 & 8)) break;

		loop_cntr--;
	}
	
	*pMDMA_S0_CONFIG = 0;
	
	*pMDMA_D0_CONFIG = 0;

	status_D0 = *pMDMA_D0_IRQ_STATUS; 
}



BOOL BlackfinDiagInstructionRam::StartEnumeratingInstructionBootStreamHeaders(UINT32 & header_offset) {

	const UINT8        *boot_base = NULL;

	UINT32 offset_lookahead;
	
    GetBootStreamStartAddr( boot_base );    
    
	UINT32             offset = 0;
  
    BOOL               bSuccess = FALSE; 
    
    
    while (1) {
    	
    	if ( offset & (sizeof(UDINT) - 1) ) break; 
    	
    	ADI_BOOT_HEADER * header = (ADI_BOOT_HEADER *)(boot_base + offset);
    	
    	if ( (header->dBlockCode & HDRSGN) != 0xAD000000 ) break;
    	
    	// Is this the start of instruction memory 
    	if ( header->pTargetAddress == (void *)0xffa00000 && header->dByteCount) {
    		
    		// Yes save the header offset and exit with a true condition 
    		header_offset = offset;
    		
    		bSuccess = TRUE;
    		
    		offset_lookahead = offset;
    	    offset_lookahead += sizeof(ADI_BOOT_HEADER);
    	    
    	    if ( !(header->dBlockCode & BFLAG_FILL) ) {
                offset_lookahead += header->dByteCount;
            }
            
            header = (ADI_BOOT_HEADER *)(boot_base + offset_lookahead);
           
    		break;
    	}
    	// Have not found the start of the program memory
    	// Calculate next block address 
    	offset += sizeof(ADI_BOOT_HEADER);
    	
        if ( !(header->dBlockCode & BFLAG_FILL) ) {
          offset += header->dByteCount;
        }
    	
    	// Are there more headers to process 
    	if ( (header->dBlockCode & BFLAG_FINAL) ) break; 
    }

    return bSuccess; 
}


BOOL BlackfinDiagInstructionRam::ConfigureDMAReadOfInstructionMemory( InstructionCompareParams & icp ) {
    
    const UINT8     *boot_base = NULL;

    GetBootStreamStartAddr( boot_base );    
    
   	ADI_BOOT_HEADER * header = (ADI_BOOT_HEADER *)(boot_base + icp.HeaderOffset);
    	
	icp.pReadFromAddr = (UINT8 *)header->pTargetAddress + icp.CurrentBfrOffset; 
	
	BOOL bPartialRead = TRUE;
	
	INT32 BytesLeft = header->dByteCount;
	
	BytesLeft -= (icp.CurrentBfrOffset + DMA_BFR_SZ);
	
	if ( BytesLeft >= 0 ) bPartialRead = FALSE;
	
	if ( bPartialRead ) {
		icp.NumberOfBytesInBuffer = header->dByteCount - icp.CurrentBfrOffset;
	}
	else {
		icp.NumberOfBytesInBuffer = DMA_BFR_SZ;
	}
	
	return bPartialRead;
 }	
		 

	   	  
BOOL BlackfinDiagInstructionRam::EnumerateNextInstructionBootStreamHeader( UINT32 & HdrOffst, BOOL & bError ) {  	
   const UINT8              *boot_base = NULL;

    
    GetBootStreamStartAddr( boot_base );    
    
	UINT32             offset = 0;
  
    BOOL               bSuccess = FALSE;
    
    bError = FALSE;
    
    	
    ADI_BOOT_HEADER * header = (ADI_BOOT_HEADER *)(boot_base + HdrOffst);

    while( TRUE ) {
    	
   		HdrOffst += sizeof(ADI_BOOT_HEADER);

        if ( !(header->dBlockCode & BFLAG_FILL) ) {
          HdrOffst += header->dByteCount;
        }

        header = (ADI_BOOT_HEADER *)(boot_base + HdrOffst);

        if ( (header->dBlockCode & HDRSGN) != 0xAD000000 ) {
    	    bError = TRUE;
    	    
    	    break;
        }
        else if ( (header->dBlockCode & BFLAG_FINAL) ) 
        {
            bSuccess = FALSE;
            
            break;
        }
        else if ( ( (UINT32)header->pTargetAddress >= 0xffa00000 ) && (header->dByteCount > 0))
        {
        	break;
        }
    } 	 
    
    return bSuccess;
} 

BOOL BlackfinDiagInstructionRam::CompareInstructMemToBootStream(InstructionCompareParams &icp )
{

   	MismatchedData mdr;
    	
    if (icp.bScaffoldingActive) {
    	for (UINT32 ui32 = 0; ui32 < 10; ui32++ ) { 
    		mdr.mv[ui32].ByteNumberFromBegOfDMARead = 0;
    		mdr.mv[ui32].uiInstrMemRead = 0;
    		mdr.mv[ui32].uiInstrBootStream = 0;
    	}
    	
    	mdr.CurrentFailureNum = 0;
    	mdr.CurrentBufferOffset = 0;//CurrentBfrOffset;
    	mdr.HeaderOffset        = 0;//HeaderOffset;
    	mdr.BootRecordStart     = NULL;//(const_cast<UINT8 *>(pBootStreamStartAddr));
    	mdr.pInstrMemBootStream = NULL;//pInstrMemBootStream;
    	mdr.pInstrMemRead       = NULL;//static_cast<UINT8 *>(pReadFromAddr);
//    mdr. = {NULL, NULL, NULL, 0, 0, 0, 0, 0, 0 };
                       //   { {
    	               //     (0,0,0),
                       //     (0,0,0),
    	               //     (0,0,0),
                       //     (0,0,0),
    	               //     (0,0,0),
                       //     (0,0,0),
    	               //     (0,0,0),
                       //     (0,0,0),
    	               //     (0,0,0),
                       //     (0,0,0),
                       //    }, NULL, NULL, NULL, 0,0,0};
    }
    
    const UINT8 * pBootStreamStartAddr = NULL;
    
    GetBootStreamStartAddr( pBootStreamStartAddr );    
    
    UINT8 * pInstrMemBootStream = (UINT8 *)(pBootStreamStartAddr + icp.HeaderOffset + sizeof(ADI_BOOT_HEADER) + icp.CurrentBfrOffset);
    
    UINT8 uiInstrBootStream = 0;
    	
    UINT8 uiInstrMemRead = 0;
    
    BOOL bCheckNextByteForEmulation = FALSE;
    
    UINT8 * pPrevInstrMemBootStream = NULL;
    	
	BOOL bSuccess = TRUE;
	
	UINT8 * pCurrentInstrctnRead = icp.pInstrMemRead;
    
    for (UINT32 ui32 = 0; ui32 < icp.NumberOfBytesInBuffer; ui32++ ) {
    	
    	uiInstrBootStream = *pInstrMemBootStream;
    	
    	uiInstrMemRead = *pCurrentInstrctnRead;
    	
    	if (uiInstrBootStream == uiInstrMemRead) {

    	    pCurrentInstrctnRead++;
    	
    	    pInstrMemBootStream++;
    		
			bCheckNextByteForEmulation = FALSE;

			continue;
    	}
    	else {
    		
    		bSuccess = FALSE;
    		
    		if (icp.bEmulationActive) {
    			if ( uiInstrMemRead == EMUEXCEPT_OPCODE ) {
    				
    				bSuccess = true;
    				
    				bCheckNextByteForEmulation = TRUE;
    				
    				pPrevInstrMemBootStream = pInstrMemBootStream;
    				
    				bSuccess = TRUE;
    				
    	            pCurrentInstrctnRead++;
    	
             	    pInstrMemBootStream++;
    		
    				continue;
    			}
    			else if (bCheckNextByteForEmulation) {
    				if ( ( pPrevInstrMemBootStream + 1 ) == (pInstrMemBootStream) ) {
    					bCheckNextByteForEmulation = FALSE;
    					
    					bSuccess = true;
    					
    	                pCurrentInstrctnRead++;
    	
             	        pInstrMemBootStream++;
    		
    					continue;
    				}
    			}   			
    		}
    		
    		if ( icp.bScaffoldingActive ) {
    			mdr.CurrentBufferOffset = icp.CurrentBfrOffset;
    			
    			mdr.HeaderOffset        = icp.HeaderOffset;
    			
    			mdr.BootRecordStart     = (const_cast<UINT8 *>(pBootStreamStartAddr));
    			
    			mdr.pInstrMemBootStream = pInstrMemBootStream;
    			
    			mdr.pInstrMemRead       = pCurrentInstrctnRead;
    			
    			if ( mdr.CurrentFailureNum < 10 ) {
    				mdr.mv[ mdr.CurrentFailureNum ].ByteNumberFromBegOfDMARead = ui32;
    				
    				mdr.mv[ mdr.CurrentFailureNum ].uiInstrMemRead = uiInstrMemRead;
    				
    				mdr.mv[ mdr.CurrentFailureNum ].uiInstrBootStream = uiInstrBootStream;
    			}
    			
    			mdr.CurrentFailureNum++; 
    			
    			bSuccess = FALSE;
     	
        	    pCurrentInstrctnRead++;
    	
        	    pInstrMemBootStream++;
   		    }
   		    
   		    else break;
    	}
    }
    
    if ( icp.bScaffoldingActive && !bSuccess ) mdDataNotTheSame.push_back(mdr);    			    		

    return bSuccess; 
}

void BlackfinDiagInstructionRam::GetBootStreamStartAddr( const UINT8 * &pBootStreamStartAddr ) {
	
	pBootStreamStartAddr = (UINT8 *)0x20040000L;
}
   
void BlackfinDiagInstructionRam::EnumerateMismatched( UINT32 & NumberOfMismatches ) {

    NumberOfMismatches = 0;
	
	if ( mdDataNotTheSame.empty() ) return;
	
   	void * puiInstr            = NULL;
    	
   	void * puiInstrBootStream  = NULL;
    	
   	void * BootRecStart        = NULL;
    	
   	UINT32 HeaderOffset        = 0;
    	
   	UINT32 BufferOffset        = 0;
    	
   	UINT32 CurrentFailureNum   = 0;
   	
	UINT32 byte_num        = 0;
	UINT32 InstrMemRead    = 0;
	UINT32 BootStreamInstr = 0;
    
	UINT8 * DMABufferAddress =  NULL;
	
    UINT8 * BootStreamAddress =  NULL;
        		
    UINT8 * InstrctMemAddress =  NULL;
    	
	for(std::vector<MismatchedData>::iterator it = mdDataNotTheSame.begin(); it != mdDataNotTheSame.end(); ++it) {
    	
    	puiInstr            = it->pInstrMemRead;
    	
    	puiInstrBootStream  = it->pInstrMemBootStream;
    	
    	BootRecStart        = it->BootRecordStart;
    	
    	HeaderOffset        = it->HeaderOffset;
    	
    	BufferOffset        = it->CurrentBufferOffset;
    	
    	CurrentFailureNum   = it->CurrentFailureNum;
    	
    	NumberOfMismatches += CurrentFailureNum;
    	
    	for ( UINT32 ui32 = 0; (ui32 < 10) && (ui32 < CurrentFailureNum ); ui32++ ) {
    		byte_num        = it->mv[ ui32 ].ByteNumberFromBegOfDMARead;

    		DMABufferAddress  =  reinterpret_cast<UINT8 *>(DMABuffer);
    		DMABufferAddress  += byte_num;
    		
    		BootStreamAddress =  reinterpret_cast<UINT8 *>(puiInstrBootStream);
    		BootStreamAddress += byte_num;
    		
    		InstrctMemAddress =  reinterpret_cast<UINT8 *>(puiInstr);
    		
    		InstrctMemAddress += byte_num;
    	
    		InstrMemRead    = it->mv[ ui32 ].uiInstrMemRead;
    		BootStreamInstr = it->mv[ ui32 ].uiInstrBootStream;
    	}
    }
    
    UINT32 ui32 = NumberOfMismatches; 
}

