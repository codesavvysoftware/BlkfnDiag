////////////////////////////////////////////////////////////////////////////////
/// @file ApexDiagArmCache.hpp
///
/// @details ApexDiagArmCache class definition.
///
/// @if REVISION_HISTORY_INCLUDED
/// @par Edit History
/// - mgrad     27-NOV-2013 Created.
/// - dtstalte  27-JAN-2014 Remove unnecessary variable/function since
///                         completion check is now interrupt driven.
/// @endif
///
/// @par Copyright (c) 2014 Rockwell Automation Technologies, Inc. All rights reserved.
////////////////////////////////////////////////////////////////////////////////
#ifndef APEXDIAGARMCACHE_HPP 
#define APEXDIAGARMCACHE_HPP 

// SYSTEM INCLUDES
// (none)

// C PROJECT INCLUDES
// (none)

// C++ PROJECT INCLUDES
#include "Apex.hpp"				    // for HW_HandleException
#include "ApexRegisters.hpp" 	    // for registers
#include "ApexDiagArmCacheAsm.hpp"	// for ARM cache test functions declarations
#include "ApexDiagnostic.hpp"       // for LOOP_COMPLETE

// FORWARD REFERENCES
// (none)

class ApexDiagArmCache
{ 
public:
    static void ArmCachePowerUp();
    static ApexDiagnostic::TestState RunTest( ApexDiagnostic::DiagControlBlock *);
    static void ArmCacheCheckResult();
    
private:
    static const UINT32 MAX_CACHE_TEST_DURATION_US = 250;

    static bool m_isTestInProgress;

    static const UINT32 m_PatternSet[];
    
    /// Default constructor.
    ApexDiagArmCache();
    
    /// Default destructor.
    ~ApexDiagArmCache();

    /// Copy constructor and assignment operator not implemented.
    ApexDiagArmCache(const ApexDiagArmCache &);
    ApexDiagArmCache &operator=(const ApexDiagArmCache &);
    
    /// Polls for cache BIST status until status is determined or timeout happens.
    static void PollForBistStatus();

};

#endif // #if !defined(APEXDIAGARMCACHE_HPP) 
