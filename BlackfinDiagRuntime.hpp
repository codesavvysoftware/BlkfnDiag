//#pragma once
// #pragma once
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @file DiagnosticScheduler.hpp
///
/// Namespace for scheduling and monitoring diagnostic tests.
///
/// @par Full Description
/// 
/// Contains the namespace for Diagnostic scheduling.  The namespace includes the definition of the parameters         
/// that comprise the runtime environment for diagnostics and the templace class definition for the diagnostic scheduler;
///
/// @if REVISION_HISTORY_INCLUDED
/// @par Edit History
/// - [0]  thaley1  01-Dec-2015 Initial revision of file.
/// @endif
///
/// @ingroup Diagnostics
///
/// @par Copyright (c) 2015 Rockwell Automation Technologies, Inc.  All rights reserved.
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// SYSTEM INCLUDES
// (none)

// C PROJECT INCLUDES
// (none)
 
// C++ PROJECT INCLUDES
// (none)

// FORWARD REFERENCES
// (none)
#if !defined(BLACKFIN_DIAG_RUNTIME_HPP)
#define BLACKFIN_DIAG_RUNTIME_HPP
            

namespace BlackfinDiagRuntimeEnvironment 
{  
    class BlackfinDiagRuntime 
    {
        public:
	
            //***************************************************************************
            // PUBLIC METHODS
            //***************************************************************************
            
            ///////////////////////////////////////////////////////////////////////////////////////////////////////////
            ///	METHOD NAME: BlackfinDiagRuntime: ExecuteDiagnostics
            ///
            /// @par Full Description
            ///      Configures the runtime environment by constructing the diagnostic tests and instantiating the 
            ///      This is the function that ultimately is called.  It is called by RunDiagnostics but RunDiagnostics
            ///      only provides a linkage between C and C++.
            ///      
            ///
            ///                               
            /// @return                             Scheduler is created on first call and then called from the
            ///                                     background..
            ///
            ///////////////////////////////////////////////////////////////////////////////////////////////////////////
            static void ExecuteDiagnostics();
            
            
        private:
    
            // Hide certain members of a class that are automatically created by the compiler if not defined;
            // Don't want the default constructer, the copy constructor, or the assignment operator to be 
            // accessable.
            BlackfinDiagRuntime();
            
            BlackfinDiagRuntime(const BlackfinDiagRuntime &);

            const BlackfinDiagRuntime & operator = (const BlackfinDiagRuntime &);

            
    };
    
    // Linkage between C and C++
    extern "C" void RunDiagnostics();

}; 
    

#endif  // if !defined(DIAGNOSTIC_RUNTIME_HPP)


