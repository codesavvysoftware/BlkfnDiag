////////////////////////////////////////////////////////////////////////////////
/// @file BlackfinDiagRamData.hpp
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
#ifndef BlackfinDiagRAMDATA_HPP 
#define BlackfinDiagRAMDATA_HPP

// SYSTEM INCLUDES
// <none>

// C PROJECT INCLUDES
// <none>

// C++ PROJECT INCLUDES
//#include "ApexStd.hpp"                          // for UINT32
//#include "ApexDiagnostic.hpp"                   // for BlackfinDiagnostic::DiagControlBlock

#include "Defs.h"
#include "BlackfinDiagnostic.h"
#include "BlackfinDiag.h"
using namespace BlackfinDiagnosticGlobals;

// FORWARD REFERENCES
// (none)

////////////////////////////////////////////////////////////////////////////////
/// @class BlackfinDiagRamData
///
/// @ingroup 
///
/// @brief Apex ram data diagnostic class.
////////////////////////////////////////////////////////////////////////////////
class BlackfinDiagRamData
{
public:
    // PUBLIC METHODS
    
    /// Initializes data ram diagnostic.
    static void Init();

    /// Performs runtime RAM data pattern tests.
    static TestState RunTest(DiagControlBlock *);

protected:
    // PROTECTED METHODS

private:
    // PRIVATE METHODS

    /// Default constructor.
    BlackfinDiagRamData();
    
    /// Default destructor.
    ~BlackfinDiagRamData();

    /// Copy constructor and assignment operator not implemented.
    BlackfinDiagRamData(const BlackfinDiagRamData &);
    BlackfinDiagRamData &operator=(const BlackfinDiagRamData &);
    
    // MEMBER VARIABLES
    static const UINT32 WORD_BOUNDARY_MASK      = 0x00000003;
    static const UINT32 FOUR_WORD_BOUNDARY_MASK = 0x0000000F;

};

#endif // #if !defined(BlackfinDiagRAMDATA_HPP)
