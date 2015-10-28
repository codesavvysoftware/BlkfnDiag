////////////////////////////////////////////////////////////////////////////////
/// @file ApexDiagRamData.hpp
///
/// @details Apex data ram diagnostic class definition.
///
/// @if REVISION_HISTORY_INCLUDED
/// @par Edit History
/// -   spolke     29-NOV-2013  Created.
/// @endif
///
/// @par Copyright (c) 2013 Rockwell Automation Technologies, Inc. All rights reserved.
////////////////////////////////////////////////////////////////////////////////
#ifndef APEXDIAGRAMDATA_HPP 
#define APEXDIAGRAMDATA_HPP

// SYSTEM INCLUDES
// <none>

// C PROJECT INCLUDES
// <none>

// C++ PROJECT INCLUDES
#include "ApexStd.hpp"                          // for UINT32
#include "ApexDiagnostic.hpp"                   // for ApexDiagnostic::DiagControlBlock

// FORWARD REFERENCES
// (none)

////////////////////////////////////////////////////////////////////////////////
/// @class ApexDiagRamData
///
/// @ingroup 
///
/// @brief Apex ram data diagnostic class.
////////////////////////////////////////////////////////////////////////////////
class ApexDiagRamData
{
public:
    // PUBLIC METHODS
    
    /// Initializes data ram diagnostic.
    static void Init();

    /// Performs runtime RAM data pattern tests.
    static ApexDiagnostic::TestState RunTest(ApexDiagnostic::DiagControlBlock *);

protected:
    // PROTECTED METHODS

private:
    // PRIVATE METHODS

    /// Default constructor.
    ApexDiagRamData();
    
    /// Default destructor.
    ~ApexDiagRamData();

    /// Copy constructor and assignment operator not implemented.
    ApexDiagRamData(const ApexDiagRamData &);
    ApexDiagRamData &operator=(const ApexDiagRamData &);
    
    // MEMBER VARIABLES
    static const UINT32 WORD_BOUNDARY_MASK      = 0x00000003;
    static const UINT32 FOUR_WORD_BOUNDARY_MASK = 0x0000000F;

};

#endif // #if !defined(APEXDIAGRAMDATA_HPP)
