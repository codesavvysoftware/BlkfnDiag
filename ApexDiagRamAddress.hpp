////////////////////////////////////////////////////////////////////////////////
/// @file ApexDiagRamAddress.hpp
///
/// @details Apex address ram diagnostic class definition.
///
/// @if REVISION_HISTORY_INCLUDED
/// @par Edit History
/// -   spolke     29-NOV-2013  Created.
/// @endif
///
/// @par Copyright (c) 2013 Rockwell Automation Technologies, Inc. All rights reserved.
////////////////////////////////////////////////////////////////////////////////
#ifndef APEXDIAGRAMADDRESS_HPP 
#define APEXDIAGRAMADDRESS_HPP 

// SYSTEM INCLUDES
// <none>

// C PROJECT INCLUDES
// <none>

// C++ PROJECT INCLUDES
#include "ApexStd.hpp"                          // for UINT32
#include "ApexParameters.hpp"                   // for DGB_CTL_BLK
#include "ApexDiagnostic.hpp"

// FORWARD REFERENCES
// (none)

////////////////////////////////////////////////////////////////////////////////
/// @class ApexDiagRamAddress
///
/// @ingroup 
///
/// @brief Apex ram address diagnostic class. 
////////////////////////////////////////////////////////////////////////////////
class ApexDiagRamAddress
{
public:
    // PUBLIC METHODS

    /// Initializes address ram diagnostic.
    static void Init();
    
    /// This function performs runtime internal RAM address line tests.
    static ApexDiagnostic::TestState RunTest(ApexDiagnostic::DiagControlBlock *);

protected:
    // PROTECTED METHODS

private:
    // PRIVATE METHODS

    /// Default constructor.
    ApexDiagRamAddress();
    
    /// Default destructor.
    ~ApexDiagRamAddress();

    /// Copy constructor and assignment operator not implemented.
    ApexDiagRamAddress(const ApexDiagRamAddress &);
    ApexDiagRamAddress &operator=(const ApexDiagRamAddress &);
    
    // MEMBER VARIABLES
};

#endif // #if !defined(APEXDIAGRAMADDRESS_HPP)
