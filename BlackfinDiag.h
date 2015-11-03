#pragma once

#include "DiagSystemDefs.h"
#include <time.h>

namespace DiagnosticCommon {

	//
	// Blackfin Diagnostic constants.  Variable names are intended to describe with they are use for.
	// 
	typedef UINT32  DiagElapsedTime_t;
	typedef clock_t DiagTimestampTime_t;

	//
	// Requirement:  All Diagnostic Tests Complete in 4 Hours.
	//
    static const DiagElapsedTime_t PeriodForAllDiagnosticsCompleted_milleseconds = 4 * 60 * 60 * 1000; // 4 hours
    
    static const DiagElapsedTime_t PeriodForOneDiagnosticTestIteration_milleseconds = 50;
    
 
	static const UINT32 DiagnosticErrorNumberMask = 0x7fffffff;

	static const UINT32 DiagnosticErrorTestTypeBitPos = 26;

	#define firmExcept( uiError ) OS_Assert( uiError );//, __FILE__, __LINE__ );           

};


