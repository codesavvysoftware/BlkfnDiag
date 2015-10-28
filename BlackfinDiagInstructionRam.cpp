#include "BlackfinDiagInstructionRam.h"

#include <vector>
using namespace DiagnosticCommon;
namespace BlackfinDiagTests {

#include "Nvs_Obj.h"
#include <bfrom.h>

BlackfinDiagTest::TestState BlackfinDiagInstructionRam::RunTest( UINT32 & errorCode ) {

	ConfigForAnyNewDiagCycle( this );
	    
    return RunInstructionRamTestIteration( icpCompare_, errorCode );
}
    
    
BlackfinDiagTest::TestState 
BlackfinDiagInstructionRam::RunInstructionRamTestIteration(	InstructionCompareParams & icpCompare,
															UINT32 & errorCode	) {
 	
	BlackfinDiagTest::TestState ts = TEST_IN_PROGRESS;

	BOOL hasError = TRUE;	  
	
	BOOL isPartialDMABuffer = ConfigureDMAReadOfInstructionMemory( icpCompare );
	
	DMA_Xfer_MDMA0( icpCompare.readFromAddr, icpCompare.ptrInstrMemRead );
	
	hasError = !CompareInstructMemToBootStream( icpCompare ); 

    if (icpCompare.scaffoldingActive) {
    	//
        // If scaffolding is active record all mismatches. Resetting error will result in reading
        // all of the instruction RAM.
        //
        //if (bError) bError = FALSE;
    }

    // Miscomparison results in an error return.
	if (hasError) {
		
   		errorCode  = GetTestType() << DiagnosticErrorTestTypeBitPos;
    
   		errorCode |= err_Mismatch_;
   		
   		void * currentAddr= icpCompare.readFromAddr + icpCompare.currentBfrOffset;
   		
   		errorCode |= reinterpret_cast<UINT32>(currentAddr) & 0xffffff;
   		
   		ts = TEST_FAILURE;
	}
	
	icpCompare.currentBfrOffset += DMA_BFR_SZ;
		
	if (isPartialDMABuffer) {
		
		hasError = FALSE;
	    	
	    BOOL noMoreHeaders = 	!EnumerateNextInstructionBootStreamHeader( icpCompare.headerOffset, hasError );
	    	
	    if ( noMoreHeaders ) {
	    	
	    	UINT32 nmbrMismatched = 0;
	    	
	    	if (icpCompare.scaffoldingActive) {
	    		EnumerateMismatched( nmbrMismatched );
	    	}

	    	ts = TEST_LOOP_COMPLETE;  	
	    }
	    else if ( hasError ) {
	    	
	    	errorCode  = GetTestType() << DiagnosticErrorTestTypeBitPos;
	    	
	    	errorCode |= err_BadBootstream_;   		
	
	    	ts = TEST_FAILURE;
	    }
	    	
	    icpCompare.currentBfrOffset = 0;
	    
	}
	
	return ts;
}


void BlackfinDiagInstructionRam::DMA_Xfer_MDMA0(void * read_from_address, void * write_to_address)
{
    UINT nmbrOf16BitUnitsToDMA = (DMA_BFR_SZ >> 1);
    
    *pMDMA_S0_X_COUNT   	= nmbrOf16BitUnitsToDMA;
    *pMDMA_S0_X_MODIFY  	= 2;  // Number of Bytes per DMA Cycle
    *pMDMA_S0_Y_COUNT   	= 0;
    *pMDMA_S0_Y_MODIFY  	= 0;
    *pMDMA_S0_START_ADDR	= read_from_address;
    *pMDMA_S0_CURR_DESC_PTR	= NULL;
    *pMDMA_S0_NEXT_DESC_PTR	= NULL;
    
    
    *pMDMA_D0_X_COUNT   	= nmbrOf16BitUnitsToDMA;
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

    status_D0           = *pMDMA_D0_IRQ_STATUS;

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
    GetBootStreamStartAddr( boot_base );    
    
	UINT32  offset_lookahead = 0;	
	UINT32  offset           = 0;  
    BOOL    bSuccess         = FALSE; 
    
    
    while (1) {
    	
    	if ( offset & (sizeof(UDINT) - 1) ) break; 
    	
    	ADI_BOOT_HEADER * header = (ADI_BOOT_HEADER *)(boot_base + offset);
    	
    	if ( (header->dBlockCode & HDRSGN) != 0xAD000000 ) break;
    	
    	// Is this the start of instruction memory 
    	if ( header->pTargetAddress == (void *)0xffa00000 && header->dByteCount) {
    		
    		// Yes save the header offset and exit with a true condition 
    		header_offset     = offset;
    		bSuccess          = TRUE;    		
    		offset_lookahead  = offset;
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
    
   	ADI_BOOT_HEADER * header = (ADI_BOOT_HEADER *)(boot_base + icp.headerOffset);    	
	icp.readFromAddr        = (UINT8 *)header->pTargetAddress + icp.currentBfrOffset; 	
	BOOL isPartialRead        = TRUE;	
	INT32 BytesLeft          = header->dByteCount;	
	BytesLeft               -= (icp.currentBfrOffset + DMA_BFR_SZ);
	
	if ( BytesLeft >= 0 ) isPartialRead = FALSE;
	
	if ( isPartialRead ) {
		icp.nmbrOfBytesInBuffer = header->dByteCount - icp.currentBfrOffset;
	}
	else {
		icp.nmbrOfBytesInBuffer = DMA_BFR_SZ;
	}
	
	return isPartialRead;
 }	
		 

	   	  
BOOL BlackfinDiagInstructionRam::EnumerateNextInstructionBootStreamHeader( UINT32 & HdrOffst, BOOL & bError ) {  	
    const UINT8              *boot_base = NULL;
    GetBootStreamStartAddr( boot_base );    
    
	UINT32             offset   = 0; 
    BOOL               success = FALSE;    
    bError                      = FALSE;
    
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
            success = FALSE;
            
            break;
        }
        else if ( ( (UINT32)header->pTargetAddress >= 0xffa00000 ) && (header->dByteCount > 0))
        {
        	break;
        }
    } 	 
    
    return success;
} 

BOOL BlackfinDiagInstructionRam::CompareInstructMemToBootStream(InstructionCompareParams &icp )
{

   	MismatchedData mdr;
    	
    if (icp.scaffoldingActive) {
    	for (UINT32 ui32 = 0; ui32 < 10; ++ui32 ) { 
    		mdr.mv[ui32].byteNumberFromBegOfDMARead = 0;
    		mdr.mv[ui32].instrMemRead = 0;
    		mdr.mv[ui32].instrBootStream = 0;
    	}
    	
    	mdr.currentFailureNum   = 0;
    	mdr.currentBufferOffset = 0;//currentBfrOffset;
    	mdr.headerOffset        = 0;//headerOffset;
    	mdr.bootRecordStart     = NULL;//(const_cast<UINT8 *>(pBootStreamStartAddr));
    	mdr.ptrInstrMemBootStream = NULL;//ptrInstrMemBootStream;
    	mdr.ptrInstrMemRead       = NULL;//static_cast<UINT8 *>(readFromAddr);
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
    
    const UINT8 * bootStreamStartAddr = NULL;
    
    GetBootStreamStartAddr( bootStreamStartAddr );    
    
    UINT8 * ptrInstrMemBootStream        = (UINT8 *)(bootStreamStartAddr + icp.headerOffset + sizeof(ADI_BOOT_HEADER) + icp.currentBfrOffset);
    UINT8   instrBootStream              = 0;
    UINT8   instrMemRead                 = 0;
    BOOL    checkNextByteForEmulation    = FALSE;
    UINT8 * prevInstrMemBootStreamAddr   = NULL;
	BOOL    success                      = TRUE;
	UINT8 * currentInstrctnRead          = icp.ptrInstrMemRead;
    
    for (UINT32 ui32 = 0; ui32 < icp.nmbrOfBytesInBuffer; ++ui32 ) {
    	
    	instrBootStream = *ptrInstrMemBootStream;
    	instrMemRead    = *currentInstrctnRead;
    	
    	if (instrBootStream == instrMemRead) {

    	    ++currentInstrctnRead;    	
    	    ++ptrInstrMemBootStream;
    		
			checkNextByteForEmulation = FALSE;

			continue;
    	}
    	else {
    		
    		success = FALSE;
    		
    		if (icp.emulationActive) {
    			if ( EMUEXCEPT_OPCODE == instrMemRead ) {
    				
    				success                    = TRUE;
    				checkNextByteForEmulation  = TRUE;
    				prevInstrMemBootStreamAddr = ptrInstrMemBootStream;
    				success                    = TRUE;
    				
    	            ++currentInstrctnRead;
             	    ++ptrInstrMemBootStream;
    		
    				continue;
    			}
    			else if (checkNextByteForEmulation) {
    				if ( ( prevInstrMemBootStreamAddr + 1 ) == (ptrInstrMemBootStream) ) {
    					
    					checkNextByteForEmulation = FALSE;
    					success                   = TRUE;
    					
    	                ++currentInstrctnRead;
             	        ++ptrInstrMemBootStream;
    		
    					continue;
    				}
    			}   			
    		}
    		
    		if ( icp.scaffoldingActive ) {

    			mdr.currentBufferOffset = icp.currentBfrOffset;
    			mdr.headerOffset        = icp.headerOffset;
    			mdr.bootRecordStart     = (const_cast<UINT8 *>(bootStreamStartAddr));
    			mdr.ptrInstrMemBootStream = ptrInstrMemBootStream;
    			mdr.ptrInstrMemRead       = currentInstrctnRead;
    			
    			if ( mdr.currentFailureNum < 10 ) {

    				mdr.mv[ mdr.currentFailureNum ].byteNumberFromBegOfDMARead = ui32;
    				mdr.mv[ mdr.currentFailureNum ].instrMemRead             = instrMemRead;
    				mdr.mv[ mdr.currentFailureNum ].instrBootStream          = instrBootStream;
    			}
    			
    			++mdr.currentFailureNum; 
    			
    			success = FALSE;
     	
        	    ++currentInstrctnRead;
    	
        	    ++ptrInstrMemBootStream;
   		    }
   		    
   		    else break;
    	}
    }
    
    if ( icp.scaffoldingActive && !success ) mdDataNotTheSame.push_back(mdr);    			    		

    return success; 
}

void BlackfinDiagInstructionRam::GetBootStreamStartAddr( const UINT8 * &bootStreamStartAddr ) {
	
	bootStreamStartAddr = (UINT8 *)0x20040000L;
}
   
void BlackfinDiagInstructionRam::EnumerateMismatched( UINT32 & NumberOfMismatches ) {

    NumberOfMismatches = 0;
	
	if ( mdDataNotTheSame.empty() ) return;
	
   	void * puiInstr            = NULL;
   	void * puiInstrBootStream  = NULL;
   	void * BootRecStart        = NULL;
   	UINT32 headerOffset        = 0;
   	UINT32 BufferOffset        = 0;
   	UINT32 CurrentFailureNum   = 0;
	UINT32 byte_num            = 0;
	UINT32 InstrMemRead        = 0;
	UINT32 BootStreamInstr     = 0;
   	UINT8 * DMABufferAddress   =  NULL;
	UINT8 * BootStreamAddress  =  NULL;
        		
    UINT8 * InstrctMemAddress =  NULL;
    	
	for(std::vector<MismatchedData>::iterator it = mdDataNotTheSame.begin(); it != mdDataNotTheSame.end(); ++it) {
    	
    	puiInstr            = it->ptrInstrMemRead;
    	puiInstrBootStream  = it->ptrInstrMemBootStream;
    	BootRecStart        = it->bootRecordStart;
    	headerOffset        = it->headerOffset;
    	BufferOffset        = it->currentBufferOffset;
    	CurrentFailureNum   = it->currentFailureNum;
    	NumberOfMismatches += it->currentFailureNum;
    	
    	for ( UINT32 ui32 = 0; (ui32 < 10) && (ui32 < CurrentFailureNum ); ++ui32 ) {
    		byte_num        = it->mv[ ui32 ].byteNumberFromBegOfDMARead;

    		DMABufferAddress  =  reinterpret_cast<UINT8 *>(bfrDMA);
    		DMABufferAddress  += byte_num;
    		
    		BootStreamAddress =  reinterpret_cast<UINT8 *>(puiInstrBootStream);
    		BootStreamAddress += byte_num;
    		
    		InstrctMemAddress =  reinterpret_cast<UINT8 *>(puiInstr);
    		InstrctMemAddress += byte_num;
    	
    		InstrMemRead    = it->mv[ ui32 ].instrMemRead;
    		BootStreamInstr = it->mv[ ui32 ].instrBootStream;
    	}
    }
    
    UINT32 ui32 = NumberOfMismatches; 
}

void BlackfinDiagInstructionRam::ConfigureForNextTestCycle() {
	
	icpCompare_.headerOffset          = 0;
	
	icpCompare_.currentBfrOffset      = 0;
	
	icpCompare_.nmbrOfBytesInBuffer   = 0;
	
	icpCompare_.readFromAddr          = NULL;
	
	icpCompare_.scaffoldingActive      = scaffoldingActive;
	
	icpCompare_.emulationActive        = emulationActive;
	
	BOOL enumerationNotStarted = !StartEnumeratingInstructionBootStreamHeaders( icpCompare_.headerOffset );
	
	if ( enumerationNotStarted) {
		//
    	// An error report it to scheduler
    	//
    	UINT32 errorCode  = GetTestType() << DiagnosticErrorTestTypeBitPos;
    
    	errorCode |= err_UnableToStart_;
    		
		firmExcept( errorCode );
   	}
    	
}

};

