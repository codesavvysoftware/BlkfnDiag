#pragma once

#include "BlackfinDiag.h"
#include "BlackfinDiagTest.h"
#include <vector>

namespace BlackfinDiagTests {
	
class BlackfinDiagInstructionRam : public BlackfinDiagTest {
	
public:
	BlackfinDiagInstructionRam( BlackfinExecTestData &     testData ) 
		    		    :	BlackfinDiagTest   ( testData ),
                            err_BadBootstream_ ( 0xffd00000 ),
                            err_UnableToStart_ ( 0xfff00000 ),
                            err_Mismatch_      ( 0xffe00000 ),
							scaffoldingActive  ( TRUE ), 
							emulationActive    ( TRUE ) 
	{}
	
	
	virtual ~BlackfinDiagInstructionRam(){}

    virtual TestState RunTest( UINT32 & ErrorCode );

protected:

	virtual void 		ConfigureForNextTestCycle();	

		
private:

    static const UINT32 	DMA_BFR_SZ                          = 256;	
    UINT8 * bfrDMA[ DMA_BFR_SZ ];
    
    typedef struct InstructionComparisonParams { 
	    UINT8          ptrInstrMemRead[DMA_BFR_SZ]; // Instruction Memory Read via DMA
        UINT32         headerOffset;                // Offset from begining of Current Bootstream Header
        UINT32         currentBfrOffset;            // Current Offset from begining of bootstream data for Instruction Ram comparisons
        UINT32         nmbrOfBytesInBuffer;         // Number of Bytes to Compare
        UINT8 *        readFromAddr;                // Address instruction RAM start for Current DMA buffer
        BOOL           scaffoldingActive;           // Active Status of Scaffolding for debugging.
        BOOL           emulationActive;             // Emulator inserts trap instructions at key places to catch breakpoints and issues.
                                                    // Instruction memory read from DMA will not compare.  Therefore to continue testing
                                                    // with the emulator this flag was introduced.
    } InstructionCompareParams;
    
	//
	// For scaffolding.  Will probably be deleted eventually.
	typedef struct MismatchVals {
		UINT32  byteNumberFromBegOfDMARead;
		UINT8   instrMemRead;
		UINT8   instrBootStream;
	} MismatchedVals;

	typedef struct MismatchData {
		MismatchedVals mv[10];
		UINT8 *        ptrInstrMemRead;
		UINT8 *        ptrInstrMemBootStream;
		UINT8 *        bootRecordStart;
		UINT32         headerOffset;
		UINT32         currentBufferOffset;
		UINT32         currentFailureNum;
	} MismatchedData;
	
    static const UINT32 EMUEXCEPT_OPCODE                    = 0x25;
    
    std::vector <MismatchedData> mdDataNotTheSame;

	const UINT32 err_BadBootstream_;
    		
	const UINT32 err_UnableToStart_;
	
	const UINT32 err_Mismatch_;
	
	BOOL                     emulationActive;
	
	BOOL                     scaffoldingActive;
	
    InstructionCompareParams icpCompare_;
	//
	// Inhibit copy construction and assignments of this class by putting the declarations in private portion.
	// If using C++ 11 and later use the delete keyword to do this.
	//
	BlackfinDiagInstructionRam(const BlackfinDiagInstructionRam & other);
	
	const BlackfinDiagInstructionRam & operator = (const BlackfinDiagInstructionRam & );
	

    BOOL CompareInstructMemToBootStream( InstructionCompareParams &icpCompare );

    BOOL ConfigureDMAReadOfInstructionMemory( InstructionCompareParams & icp );

    void DMA_Xfer_MDMA0(void * read_from_address, void * write_to_address);

    void EnumerateMismatched( UINT32 & NumberOfMismatches );
    
    BOOL EnumerateNextInstructionBootStreamHeader( UINT32 & HdrOffst, BOOL & bError );   	

    void GetBootStreamStartAddr( const UINT8 * &pBootStreamStartAddr );

	BlackfinDiagTest::TestState 
	RunInstructionRamTestIteration(	InstructionCompareParams & icpCompare,
									UINT32 & ErrorCode	);
																
	BOOL StartEnumeratingInstructionBootStreamHeaders(UINT32 & header_offset);
	};

};

