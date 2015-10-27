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
    static const DiagTime_t PeriodForAllDiagnosticsCompleted_milleseconds = 4 * 60 * 60 * 1000; // 4 hours
	
	static const UINT32 DiagnosticErrorNumberMask = 0x7fffffff;

	static const UINT32 DiagnosticErrorTestTypeBitPos = 26;

#define firmExcept( uiError ) OS_Assert( uiError );//, __FILE__, __LINE__ );           

	static UINT32 current_system_time;

	static UINT32 GetCachedSystemTime() {
		return current_system_time;
	}

	static UINT32 GetSystemTime( ) {
    	
    	static const UINT32 DGN_CONVERSION_FACTOR_CYCLES_ms = CLOCKS_PER_SEC / 1000000000;

		if (DGN_CONVERSION_FACTOR_CYCLES_ms) {
			current_system_time = (clock() / DGN_CONVERSION_FACTOR_CYCLES_ms);
		}
		else { 
			current_system_time = clock();
		}

		return current_system_time;
	}

};


