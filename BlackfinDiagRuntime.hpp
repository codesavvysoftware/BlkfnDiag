#pragma once
#include "BlackfinDiagTest.hpp"
#include "BlackfinDiagInstructionRam.hpp"
#include "BlackfinDiagDataRam.hpp"
#include "BlackfinDiagRegistersTest.hpp"
#include "BlackfinDiagTimerTest.hpp"
#include "BlackfinDiagInstructionsTest.hpp"

namespace BlackfinDiagRuntimeEnvironment 
{  
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
            
            BlackfinDiagRuntime(const BlackfinDiagRuntime &);

            const BlackfinDiagRuntime & operator = (const BlackfinDiagRuntime &);

            
    };
    
    

};

