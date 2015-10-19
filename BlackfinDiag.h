#pragma once

#include "DiagSystemDefs.h"

#include <time.h>

#include <vector>

namespace DiagnosticCommon {

	//
	// Blackfin Diagnostic constants.  Variable names are intended to describe with they are use for.
	// 
	typedef UINT32 DiagSlices_t;
	typedef UINT32 DiagTime_t;


	//
	// Requirement:  All Diagnostic Tests Complete in 4 Hours.
	//
    static const DiagTime_t PeriodForAllDiagnosticsCompleted_microseconds = 4 * 60 * 60 * 1000; // 4 hours
	
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


