#pragma once

#include "Defs.h"
#include "Os_iotk.h"



namespace DiagnosticCommon 
{
	//
	// Blackfin Diagnostic constants.  Variable names are intended to describe with they are use for.
	// 
    //
    // Var suffix of:
    // 
    // 	MS == milleseconds
    // 	US == microseconds
    //	
	static void BlackfinExceptions( UINT32 errorCode ) 
	{
		OS_Assert( errorCode );
	}
		
};

