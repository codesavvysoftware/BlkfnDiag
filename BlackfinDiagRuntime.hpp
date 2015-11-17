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
    extern "C" UINT32  BlackfinDiagRegSanityChk();
    extern "C" UINT32  BlackfinDiagRegChk();

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

