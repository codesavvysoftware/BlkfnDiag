#pragma once

#include "BlackfinDiag.h"
#include "BlackfinDiagTest.h"
#include <vector>

using namespace DiagnosticCommon;

class BlackfinDiagInstructionRam : public BlackfinDiagTest {
	
public:
	BlackfinDiagInstructionRam( DiagnosticTestTypes TestType = DiagInstructionRamTestType ) 
									: BlackfinDiagTest   ( TestType, PeriodPerTestIteration_Milleseconds ), 
									  bScaffoldingActive ( TRUE ), 
									  bEmulationActive   ( TRUE ) {}
	
	virtual ~BlackfinDiagInstructionRam(){}

    virtual TestState RunTest( UINT32 & ErrorCode, DiagTime_t SystemTime = GetSystemTime() );

    virtual BOOL IsTestComplete();

protected:

	virtual void 		ConfigureForNextTestCycle();	

		
private:

    static const UINT32 DMA_BFR_SZ                          = 256;	
    
    static const UINT32 EMUEXCEPT_OPCODE                    = 0x25;
    
	static const UINT32 PeriodPerTestIteration_Milleseconds = 2000;

	static const UINT32 Err_UnableToStart                   = 0xfff00000;
	
	static const UINT32 Err_Mismatch                        = 0xffe00000;
	
	static const UINT32 Err_BadBootstream               = 0xffd00000;
    		
    typedef struct InstructionComparisonParams { 
	    UINT8          pInstrMemRead[DMA_BFR_SZ]; // Instruction Memory Read via DMA
        UINT32         HeaderOffset;              // Offset from begining of Current Bootstream Header
        UINT32         CurrentBfrOffset;          // Current Offset from begining of bootstream data for Instruction Ram comparisons
        UINT32         NumberOfBytesInBuffer;     // Number of Bytes to Compare
        UINT8 *        pReadFromAddr;             // Address instruction RAM start for Current DMA buffer
        BOOL           bScaffoldingActive;        // Active Status of Scaffolding for debugging.
        BOOL           bEmulationActive;          // Emulator inserts trap instructions at key places to catch breakpoints and issues.
                                                  // Instruction memory read from DMA will not compare.  Therefore to continue testing
                                                  // with the emulator this flag was introduced.
    } InstructionCompareParams;
    
	//
	// For scaffolding.  Will probably be deleted eventually.
	typedef struct MismatchVals {
		UINT32  ByteNumberFromBegOfDMARead;
		UINT8   uiInstrMemRead;
		UINT8   uiInstrBootStream;
	} MismatchedVals;

	typedef struct MismatchData {
		MismatchedVals mv[10];
		UINT8 *        pInstrMemRead;
		UINT8 *        pInstrMemBootStream;
		UINT8 *        BootRecordStart;
		UINT32         HeaderOffset;
		UINT32         CurrentBufferOffset;
		UINT32         CurrentFailureNum;
	} MismatchedData;
	
	BOOL  bEmulationActive;
	BOOL  bScaffoldingActive;
	
    UINT8 DMABuffer[DMA_BFR_SZ];
    
    std::vector <MismatchedData> mdDataNotTheSame;

    InstructionCompareParams icpCompare;
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

