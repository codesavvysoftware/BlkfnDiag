#pragma once

//#include "BlackfinParameters.h"
#include "Defs.h"
//#include "BlackfinDiagRegTestCommon.h"

#include <time.h>
#include <vector>

namespace BlackfinDiagnosticGlobals {

	//
	// Blackfin Diagnostic constants.  Variable names are intended to describe with they are use for.
	// 

	static const UINT32 DiagnosticErrorNumberMask = 0x7fffffff;

	static const UINT32 DiagnosticErrorTestTypeBitPos = 26;

#define firmExcept( uiError ) OS_Assert( uiError );//, __FILE__, __LINE__ );           

static UINT32 GetSystemTime() {
		
		const UINT32 DGN_CONVERSION_FACTOR_CYCLES_uS = CLOCKS_PER_SEC / 1000000;
		
		if (DGN_CONVERSION_FACTOR_CYCLES_uS) {
			return (clock() / DGN_CONVERSION_FACTOR_CYCLES_uS);
		}
		else { 
			return clock();
		}
	}

};


