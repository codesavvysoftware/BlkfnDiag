///////////////////////////////////////////////////////////////////////////////
/// @file ApexDiagArmCacheAsm.hpp
///
/// @details ARM cache diagnostics.
///
/// @par Full Description
/// Contains ARM cache diagnostics tests functions declarations.
///
/// @if REVISION_HISTORY_INCLUDED
/// @par Edit History
/// -   fzembok 21-Oct-2013 ARM Cache diagnostics.
/// -   fzembok 05-Nov-2013 Fault injection fixes
/// -   mgrad   28-NOV-2013 File name changed during MISRA fixes
/// @endif
///
/// @par Copyright (c) 2013 Rockwell Automation Technologies, Inc. All rights reserved.
///////////////////////////////////////////////////////////////////////////////
#ifndef APEXDIAGARMCACHEASM_HPP
#define APEXDIAGARMCACHEASM_HPP

#include "ApexStdType.hpp"	// for types

extern "C"
{

void DisableDataCache();
void FlushDataCache();
void EnableDataCache();

void DisableInstrCache();
void FlushInstrCache();
void EnableInstrCache();

void StartCacheBist( UINT32 pattern );

typedef enum CacheBistStatus
{
    SUCCESS = 0,
    DATA_CACHE_BIST_NOT_COMPLETED,
    INSTRUCTION_CACHE_BIST_NOT_COMPLETED,
    DATA_CACHE_BIST_FAILED,
    INSTRUCTION_CACHE_BIST_FAILED
} CacheBistStatus;

CacheBistStatus GetCacheBistStatus();

void EndCacheBist();

}

#endif //APEXDIAGARMCACHEASM_HPP
