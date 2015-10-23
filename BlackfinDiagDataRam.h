#pragma once
#include "BlackfinDiag.h"
#include "BlackfinDiagTest.h"

#include <ccblkfn.h>                              /* Added for ssync( ), cli/sti( ) */

using namespace DiagnosticCommon;

class BlackfinDiagDataRam : public BlackfinDiagTest {

public:
	BlackfinDiagDataRam( BlackfinDataRamTestSuite * DataRamTestSuite,
	                     const UINT8 *              TestPatternsForRamTesting,
	                     UINT32                     NumberOfRamTestingPatterns,
	                     DiagnosticTestTypes TestType = DiagDataRamTestType   ) 
	                     : 	DataRamTestSuite       ( DataRamTestSuite ),
	                        BlackfinDiagTest       ( TestType, PeriodPerTestIteration_Milleseconds ), 
							RamTestPatterns        ( TestPatternsForRamTesting ),
							NumberOfTestPatterns   ( NumberOfRamTestingPatterns )
	{}

	virtual ~BlackfinDiagDataRam() {}

	virtual TestState RunTest(UINT32 & ErrorCode, DiagTime_t TimeTestStarted_microseconds = GetSystemTime()  );

	virtual BOOL IsTestComplete();

 	typedef struct {
		      UINT8  * pByteToTest;
		      UINT8  * pPatternThatFailed;
		const UINT8  * pTestPatterns;
		      UINT32   NumberOfTestPatterns;
	} ByteTestParameters;
	
protected:

	virtual void ConfigureForNextTestCycle();
	
private:
	static const UINT32        NumberOfBytesToTestPerIteration     = 0x400;

	static const UINT32        TotalNumberOfRamBytesToTest         = 0x10000;
	
	static const UINT32        PeriodPerTestIteration_Milleseconds = 500;
	
	static const UINT32        ErrorRegionBitPos = 30;
	
	static const UINT32        ErrorTestPatternBitPos = 16;




	const UINT8 *              RamTestPatterns;

	typedef enum { BankA = 1, BankB = 2, BankC = 3 } DataRamMemoryRegions;
	
	UINT32                     NumberOfTestPatterns;
	
	BlackfinDataRamTestSuite * DataRamTestSuite;

	ByteTestParameters         btp;
	
	INT                        Critical;                    

	void DisableInterrupts() {
		Critical = cli();
	}

	void EnableInterrupts() {
		sti(Critical);
	}

    void EncodeErrorInfo( UINT32 &             ErrorInfo, 
                          DataRamMemoryRegions Region, 
                          UINT32               OffsetFromBankStart, 
                          UINT32 FailurePattern );
                          
	BOOL RunRamTest( DataRamTestDescriptor * RamDescriptor, 
                     UINT32 &                OffsetFromBankStart, 
                     UINT32 &                FailurePattern );
                      
	BOOL TestAByte(ByteTestParameters * pbtp);

	BOOL TestByteForAllTestPatterns(ByteTestParameters * pbtp);
};


