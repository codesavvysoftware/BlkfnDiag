////////////////////////////////////////////////////////////////////////////////
/// @file ApexDiagIoShutdownStub.hpp
///
/// @details Stubb for ApexDiagIoShutdown class when the test is no suppose to run (on ENzT for example).
///
/// @if REVISION_HISTORY_INCLUDED
/// @par Edit History
/// -   spolke     11-DEC-2013  Created.
/// @endif
///
/// @par Copyright (c) 2013 Rockwell Automation Technologies, Inc. All rights reserved.
////////////////////////////////////////////////////////////////////////////////
#ifndef APEXDIAGIOSHUTDOWNSTUB_HPP 
#define APEXDIAGIOSHUTDOWNSTUB_HPP 

// SYSTEM INCLUDES
// <none>

// C PROJECT INCLUDES
// <none>

// C++ PROJECT INCLUDES
// (none)

// FORWARD REFERENCES
// (none)

////////////////////////////////////////////////////////////////////////////////
/// @class ApexDiagIoShutdown
///
/// @ingroup ApexDiagIoShutdown
///
/// @brief ApexDiagIoShutdown class stub.
////////////////////////////////////////////////////////////////////////////////
class ApexDiagIoShutdown 
{
public:
    
    /// Initialization of IO Shutdown diagnostics.
    inline static void Init() {};

    /// Performs IO shutdown diagnostics test if it's time for it. 
    static bool IsTimeToRun(const UINT32) { return false; };

    /// Performs IO shutdown diagnostics test. 
    static void RunTest(const UINT32) {};

    /// Checks if IO shutdown diagnostic completes in time. 
    inline static bool CheckCompletionTime() { return true; };
};

#endif // #if !defined(APEXDIAGIOSHUTDOWNSTUB_HPP)
