////////////////////////////////////////////////////////////////////////////////
/// @file ApexDiagArm.hpp
///
/// @details ApexDiagArm class definition.
///
/// @if REVISION_HISTORY_INCLUDED
/// @par Edit History
/// -   mgrad     26-NOV-2013  Created.
/// @endif
///
/// @par Copyright (c) 2013 Rockwell Automation Technologies, Inc. All rights reserved.
////////////////////////////////////////////////////////////////////////////////
#ifndef APEXDIAGARM_HPP 
#define APEXDIAGARM_HPP 

// SYSTEM INCLUDES
// (none)

// C PROJECT INCLUDES
// (none)

// C++ PROJECT INCLUDES
#include "Apex.hpp"				// for HW_HandleException
#include "ApexRegisters.hpp" 	// for registers
#include "ApexDiagArmAsm.hpp"	// for ARM test functions declarations
#include "ApexDiagnostic.hpp"   // for TEST_LOOP_COMPLETE

// FORWARD REFERENCES
// (none)

class ApexDiagArm
{ 
public:
    static void ArmPowerUp();
    static ApexDiagnostic::TestState RunTest( ApexDiagnostic::DiagControlBlock * dgnControlBlock );

private:
    enum
    {
        CPSR_N      = 0x80000000,
        CPSR_Z      = 0x40000000,
        CPSR_C      = 0x20000000,
        CPSR_V      = 0x10000000,
        CPSR_NZCV   = 0xf0000000
    }; 
    
    typedef bool (*TestRegisterFunctionPtr)( UINT32 pattern1, UINT32 pattern2 );
    typedef void (*TestStageFunctionPtr)();
    
    static const UINT32 f_patternSet[];

    static const TestStageFunctionPtr f_testStageFunctionPtrs[];
    
    /// Default constructor.
    ApexDiagArm();
    
    /// Default destructor.
    ~ApexDiagArm();

    /// Copy constructor and assignment operator not implemented.
    ApexDiagArm(const ApexDiagArm &);
    ApexDiagArm &operator=(const ApexDiagArm &);
    
    static void TestInstructions();
    static void TestRegistersR0R12();
    static void TestRegistersSpLr();
    static void TestRegistersFiqCpsr();
    
    static void TestRegister( TestRegisterFunctionPtr testRegisterFunctionPtr );
    static void TestArithmeticOperations();
    static void TestDataMovement();
    static void TestLogicalOperations();
    static void TestComparisions();
    static void TestConditionalExecution();
    static void TestSigned16BitMultiplication();
    static void TestSingleRegisterDataTransfer();
    static void TestMultipleRegisterDataTransfer();
    static void TestCountLeadingZeros();
    static void TestOperand2();
    static void TestAddressingModes();
};

#endif // #if !defined(APEXDIAGARM_HPP) 
