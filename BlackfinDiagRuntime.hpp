#pragma once
#include "BlackfinDiagTest.hpp"
#include "BlackfinDiagInstructionRam.hpp"
#include "BlackfinDiagDataRam.hpp"
#include "BlackfinDiagRegistersTest.hpp"
#include "BlackfinDiagTimerTest.hpp"
#include "BlackfinDiagInstructionsTest.hpp"

namespace BlackfinDiagRuntimeEnvironment 
{  
      //
    // For linkage to c callable assembly language register tests
    //
    extern "C" UINT32  BlackfinDiagRegSanityChk( const UINT32 *, UINT32);
    extern "C" UINT32  BlackfinDiagRegDataReg7Chk( const UINT32 *, UINT32 );
    extern "C" UINT32  BlackfinDiagRegDataReg6Chk( const UINT32 *, UINT32 );
    extern "C" UINT32  BlackfinDiagRegDataReg5Chk( const UINT32 *, UINT32 );
    extern "C" UINT32  BlackfinDiagRegDataReg4Chk( const UINT32 *, UINT32 );
    extern "C" UINT32  BlackfinDiagRegDataReg3Chk( const UINT32 *, UINT32 );
    extern "C" UINT32  BlackfinDiagRegPointerReg5Chk( const UINT32 *, UINT32 );
    extern "C" UINT32  BlackfinDiagRegPointerReg4Chk( const UINT32 *, UINT32 );
    extern "C" UINT32  BlackfinDiagRegPointerReg3Chk( const UINT32 *, UINT32 );
    extern "C" UINT32  BlackfinDiagRegPointerReg2Chk( const UINT32 *, UINT32 );
    extern "C" UINT32  BlackfinDiagRegPointerReg0Chk( const UINT32 *, UINT32 );
    extern "C" UINT32  BlackfinDiagAccum0Chk( const UINT32 *, UINT32 );
    extern "C" UINT32  BlackfinDiagAccum1Chk( const UINT32 *, UINT32 );
    extern "C" UINT32  BlackfinDiagRegModifyReg3Chk( const UINT32 *, UINT32 );
    extern "C" UINT32  BlackfinDiagRegModifyReg2Chk( const UINT32 *, UINT32 );
    extern "C" UINT32  BlackfinDiagRegModifyReg1Chk( const UINT32 *, UINT32 );
    extern "C" UINT32  BlackfinDiagRegModifyReg0Chk( const UINT32 *, UINT32 );
    extern "C" UINT32  BlackfinDiagRegLengthReg3Chk( const UINT32 *, UINT32 );
    extern "C" UINT32  BlackfinDiagRegLengthReg2Chk( const UINT32 *, UINT32 );
    extern "C" UINT32  BlackfinDiagRegLengthReg1Chk( const UINT32 *, UINT32 );
    extern "C" UINT32  BlackfinDiagRegLengthReg0Chk( const UINT32 *, UINT32 );
    extern "C" UINT32  BlackfinDiagRegIndexReg3Chk( const UINT32 *, UINT32 );
    extern "C" UINT32  BlackfinDiagRegIndexReg2Chk( const UINT32 *, UINT32 );
    extern "C" UINT32  BlackfinDiagRegIndexReg1Chk( const UINT32 *, UINT32 );
    extern "C" UINT32  BlackfinDiagRegIndexReg0Chk( const UINT32 *, UINT32 );
    extern "C" UINT32  BlackfinDiagRegBaseReg3Chk( const UINT32 *, UINT32 );
    extern "C" UINT32  BlackfinDiagRegBaseReg2Chk( const UINT32 *, UINT32 );
    extern "C" UINT32  BlackfinDiagRegBaseReg1Chk( const UINT32 *, UINT32 );
    extern "C" UINT32  BlackfinDiagRegBaseReg0Chk( const UINT32 *, UINT32 );	

    template<typename T, size_t N>
    T * end(T (&ra)[N]) 
    {
    	return ra + N;
    }
    class BlackfinDiagRuntime 
    {
        public:
	
            static void ExecuteDiagnostics();
            
            
        private:
    
            // Constructor definition
            BlackfinDiagRuntime();
    };
    
    

};

