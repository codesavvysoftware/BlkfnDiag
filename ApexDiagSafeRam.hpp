////////////////////////////////////////////////////////////////////////////////
/// @file ApexDiagSafeRam.hpp
///
/// @details Apex safe ram diagnostic class definition.
///
/// @if REVISION_HISTORY_INCLUDED
/// @par Edit History
/// -   spolke     28-NOV-2013  Created. 
/// @endif
///
/// @par Copyright (c) 2013 Rockwell Automation Technologies, Inc. All rights reserved.
////////////////////////////////////////////////////////////////////////////////
#ifndef APEXDIAGSAFERAM_HPP
#define APEXDIAGSAFERAM_HPP 

// SYSTEM INCLUDES
// <none>

// C PROJECT INCLUDES
// <none>

// C++ PROJECT INCLUDES
#include "ApexStd.hpp"                  // for UINT32
#include "ApexParameters.hpp"           // for DiagControlBlock
#include "ApexDiagnostic.hpp"

// FORWARD REFERENCES
// (none)

struct DGN_SAFE_RAM {
    UINT32 intDataramSize;
    UINT32 intDataramSizePrime;
    UINT32 intShramSize;
    UINT32 intShramSizePrime;
    UINT32 extShramSize;
    UINT32 extShramSizePrime;
    UINT32 curDataTestAddr;
    UINT32 curDataTestAddrPrime;
    UINT32 curAddrTestAddr1;
    UINT32 curAddrTestAddr1Prime;
    UINT32 curAddrTestAddr2;
    UINT32 curAddrTestAddr2Prime;
    UINT32 dataIterations;
    UINT32 dataIterationsPrime;
    UINT32 addrIterations;
    UINT32 addrIterationsPrime;
    UINT32 dataRamSize;
    UINT32 dataRamSizePrime;
    UINT32 curShadowRamAddr;
    UINT32 curShadowRamAddrPrime;
};

extern "C" DGN_SAFE_RAM DgnSafeRam;

////////////////////////////////////////////////////////////////////////////////
/// @class ApexDiagSafeRam
///
/// @ingroup 
///
/// @brief Apex safe ram diagnostic class.
////////////////////////////////////////////////////////////////////////////////
class ApexDiagSafeRam
{
public:
    // PUBLIC METHODS
    
    /// Initializes safe ram diagnostic.
    static void Init(void);

    /// Verifies safe ram data validity.
    static ApexDiagnostic::TestState Check(ApexDiagnostic::DiagControlBlock * );

protected:
    // PROTECTED METHODS

private:
    // PRIVATE METHODS

    /// Default constructor.
    ApexDiagSafeRam();
    
    /// Default destructor.
    ~ApexDiagSafeRam();

    /// Copy constructor and assignment operator not implemented.
    ApexDiagSafeRam(const ApexDiagSafeRam &);
    ApexDiagSafeRam &operator=(const ApexDiagSafeRam &);

    /// Verifies address test-related variable doesn't match its complement.
    static bool IsAddrSafeRamInvalid();
    
    /// Verifies data test-related variable doesn't match its complement.
    static bool IsDataSafeRamInvalid();

    // MEMBER VARIABLES
};

#endif // #if !defined(APEXDIAGSAFERAM_HPP) 
