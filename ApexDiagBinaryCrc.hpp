////////////////////////////////////////////////////////////////////////////////
/// @file ApexDiagBinaryCrc.hpp
///
/// @details Apex binary CRC diagnostic class diagnostic definition.
///
/// @if REVISION_HISTORY_INCLUDED
/// @par Edit History
/// -   spolke     27-NOV-2013  Created.
/// @endif
///
/// @par Copyright (c) 2013 Rockwell Automation Technologies, Inc. All rights reserved.
////////////////////////////////////////////////////////////////////////////////
#ifndef APEXDIAGBINARYCRC_HPP
#define APEXDIAGBINARYCRC_HPP

// SYSTEM INCLUDES
// <none>

// C PROJECT INCLUDES
// <none>

// C++ PROJECT INCLUDES
#include "Apex.hpp"                          // for HW_HandleException
#include "ApexDiagnostic.hpp"                // for firmExcept
#include "ApexParameters.hpp"                // for DiagControlBlock

// FORWARD REFERENCES
// (none)

////////////////////////////////////////////////////////////////////////////////
/// @class ApexDiagBinaryCrc
///
/// @ingroup Diagnostics
///
/// @brief <TODO: Place a description of the class here)
////////////////////////////////////////////////////////////////////////////////
class ApexDiagBinaryCrc
{
public:
    // PUBLIC METHODS

    /// Initializes crc data diagnostic.
    static void Init();

    /// This function performs a crc test of the Apex executable RAM area at powerup.
    static void PowerupTest();

    /// This function performs runtime tests of the executable RAM area of Apex.
    static ApexDiagnostic::TestState RunTest(ApexDiagnostic::DiagControlBlock *);

protected:
    // PROTECTED METHODS

private:
    // PRIVATE METHODS

    /// Default constructor.
    ApexDiagBinaryCrc();
    
    /// Default destructor.
    ~ApexDiagBinaryCrc();

    /// Copy constructor and assignment operator not implemented.
    ApexDiagBinaryCrc(const ApexDiagBinaryCrc &);
    ApexDiagBinaryCrc &operator=(const ApexDiagBinaryCrc &);
    
    /// Resets crc data to initial values.
    static void CrcDataReset();

    /// Function that calculates a 32-bit CRC (Sum2Bin implementation based).
    static UINT32 CalculateCrc32(const UINT8 *&pData, const UINT32 byteLen, UINT32 crc);

    /// This function compares two numbers and return the smaller one.
    static inline UINT32 min(UINT32 a, UINT32 b) 
    { 
        return a > b ? b : a;
    }

    // MEMBER VARIABLES
    
    static UINT32       m_CodeSize;
    static UINT32       m_CrcExpected;
    static UINT32       m_BytesPerStep;
    static UINT32       m_Crc;
    static UINT32       m_BytesLeft;
    static const UINT8 *m_pCurrentPtr;

};

#endif // #if !defined(APEXDIAGBINARYCRC_HPP)
