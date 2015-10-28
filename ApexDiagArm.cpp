///////////////////////////////////////////////////////////////////////////////
/// @file ApexDiagArm.cpp
///
/// @details ARM instructions and registers diagnostics.
///
/// @par Full Description
/// Contains ARM instructions and registers diagnostics tests code.
///
/// @if REVISION_HISTORY_INCLUDED
/// @par Edit History
/// -   fzembok     20-AUG-2013 ARM instructions and registers diagnostics.
/// -   dtstalte    17-OCT-2013 Interrupt locking around test call to ensure
///                             FIQ/IRQ mode registers are not used in a
///                             corrupted state.
/// -   fzembok     21-OCT-2013 ARM Cache diagnostics.
/// -   fzembok     05-NOV-2013 Fault injection fixes.
/// -   mgrad       26-NOV-2013 MISRA fixes.
/// -   pszramo     11-DEC-2013 Updates according to review #28442
///                             and Coverity defects triage.
/// @endif
///
/// @par Copyright (c) 2013 Rockwell Automation Technologies, Inc. All rights reserved.
///////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
#include <climits>              // for limits constants

// C PROJECT INCLUDES
// (none)

// C++ PROJECT INCLUDES
#include "ApexDiagArm.hpp"      // for ApexDiagArm class definition

// FORWARD REFERENCES
// (none)

// This pattern set specifically designed to detect individual memory cell shorts,
// opens, and stuck at conditions, data signal line shorts and opens,
// as well as coupling faults between bits in each register.
// This pattern set should be rotated (the order of the patterns changed)
// and inverted (all bits in each word are inverted)
// to provide full diagnostic coverage.
const UINT32 ApexDiagArm::f_patternSet[] =
{
    0x99c966c6,
    0x9c966c69,
    0xc966c699,
    0x966c699c,
};

const ApexDiagArm::TestStageFunctionPtr ApexDiagArm::f_testStageFunctionPtrs[] =
{
    TestInstructions,
    TestRegistersR0R12,
    TestRegistersSpLr,
    TestRegistersFiqCpsr,
};

/////////////////////////////////////////////////////////////////////////////
// FUNCTION NAME: TestArithmeticOperations
//
/// Tests ARM arithmetic operations instructions.
///
/// @par Full Description
/// Calls ARM arithmetic operations instructions test functions
/// with prepared parameters.
/// Checks if results are as expected.
///
/////////////////////////////////////////////////////////////////////////////
void ApexDiagArm::TestArithmeticOperations()
{
    // ADD
    ASSERT( 0xffffffff == ArmInstrAdd( 0x55555555, 0xaaaaaaaa ) );

    // ADC. Add with carry
    ASSERT( 0xffffffff == ArmInstrAdc( 0x55555555 - 1, 0xaaaaaaaa ) );

    // QADD. Signed Add with saturation.
    ASSERT( 1 == ArmInstrQadd( 2, -1 ) );
    ASSERT( LONG_MIN == ArmInstrQadd( LONG_MIN + 1, -2 ) );
    ASSERT( LONG_MAX == ArmInstrQadd( LONG_MAX - 1, 2 ) );

    // QDADD. Signed double and add with saturation
    ASSERT( 0 == ArmInstrQdadd( 2, -1 ) );
    ASSERT( LONG_MIN == ArmInstrQdadd( LONG_MIN + 1, -2 ) );
    ASSERT( LONG_MAX == ArmInstrQdadd( LONG_MAX - 1, 2 ) );

    // SUB
    ASSERT( 0x55555555 == ArmInstrSub( 0xffffffff, 0xaaaaaaaa ) );

    // SBC. Subtract with carry
    ASSERT( 0x55555555 == ArmInstrSbc( 0xffffffff, 0xaaaaaaaa - 1 ) );

    // RSB.  Reverse subtract
    ASSERT( 0x55555555 == ArmInstrRsb( 0xaaaaaaaa, 0xffffffff ) );

    // RSC. Reverse subtract with carry
    ASSERT( 0x55555555 == ArmInstrRsc( 0xaaaaaaaa - 1, 0xffffffff ) );

    // QSUB. Signed subtract with saturation
    ASSERT( 1 == ArmInstrQsub( 2, 1 ) );
    ASSERT( LONG_MIN == ArmInstrQsub( LONG_MIN + 1, 2 ) );
    ASSERT( LONG_MAX == ArmInstrQsub( LONG_MAX - 1, -2 ) );

    // QDSUB. Signed double and subtract with saturation
    ASSERT( 0 == ArmInstrQdsub( 2, 1 ) );
    ASSERT( LONG_MIN == ArmInstrQdsub( LONG_MIN + 1, 2 ) );
    ASSERT( LONG_MAX == ArmInstrQdsub( LONG_MAX - 1, -2 ) );

    // MUL
    ASSERT( 0x55555554 == ArmInstrMul( 0xaaaaaaaa, 2 ) );

    // MLA. Multiply accumulate
    ASSERT( 0x55555555 == ArmInstrMla( 0xaaaaaaaa, 2, 1) );

    // UMULL. Unsigned long multiply. 64 bit result
    ASSERT( 0x0000123456780000ull == ArmInstrUmull( 0x12345678, ( 1 << 16 ) ) );

    // UMLAL. Unsigned long multiply, with Accumulate. 64 bit result
    ASSERT( 0x0123456789abcdefull == ArmInstrUmlal(0x012300000000cdefull, 0x456789ab, ( 1 << 16 ) ) );

    // SMULL and SMLAL instructions return result of INT64 type
    // Apex building toolchain does not support INT64 type
    // Signed result of SMULL and SMLAL can not be tested

    // SMULL. Signed long multiply. 64 bit result
    ASSERT( 0x0000123456780000ull == ArmInstrSmull( -0x12345678, -( 1 << 16 ) ) );

    // SMLAL. Unsigned long multiply, with Accumulate. 64 bit result
    ASSERT( 0x0123456789abcdefull == ArmInstrSmlal(0x012300000000cdefull, -0x456789ab, -( 1 << 16 ) ) );
}

/////////////////////////////////////////////////////////////////////////////
// FUNCTION NAME: TestDataMovement
//
/// Tests ARM data movement instructions.
///
/// @par Full Description
/// Calls ARM data movement instructions test functions
/// with prepared parameters.
/// Checks if results are as expected.
///
/////////////////////////////////////////////////////////////////////////////
void ApexDiagArm::TestDataMovement()
{
    // MOV
    ASSERT( 0xffffffff == ArmInstrMov( 0x00000000, 0xffffffff ) );

    // MVN. Move negative
    ASSERT( 0x00000000 == ArmInstrMvn( 0xffffffff, 0xffffffff ) );
}

/////////////////////////////////////////////////////////////////////////////
// FUNCTION NAME: TestLogicalOperations
//
/// Tests ARM logical operations instructions.
///
/// @par Full Description
/// Calls ARM logical operations instructions test functions
/// with prepared parameters.
/// Checks if results are as expected.
///
/////////////////////////////////////////////////////////////////////////////
void ApexDiagArm::TestLogicalOperations()
{
    // AND. Bitwise and
    ASSERT( 0x00000000 == ArmInstrAnd( 0x55555555, 0xaaaaaaaa ) );
    ASSERT( 0xaaaaaaaa == ArmInstrAnd( 0xffffffff, 0xaaaaaaaa ) );
    ASSERT( 0x55555555 == ArmInstrAnd( 0x55555555, 0xffffffff ) );

    // ORR. Bitwise or
    ASSERT( 0xffffffff == ArmInstrOrr( 0x55555555, 0xaaaaaaaa ) );

    // EOR. Exclusive or
    ASSERT( 0xffffffff == ArmInstrEor( 0x55555555, 0xaaaaaaaa ) );

    // BIC. Bit clear
    ASSERT( 0xaaaaaaaa == ArmInstrBic (0xffffffff, 0x55555555 ) );
    ASSERT( 0x55555555 == ArmInstrBic (0xffffffff, 0xaaaaaaaa ) );
}

/////////////////////////////////////////////////////////////////////////////
// FUNCTION NAME: TestComparisions
//
/// Tests ARM comparison instructions.
///
/// @par Full Description
/// Calls ARM comparison instructions test functions
/// with prepared parameters.
/// Checks if results are as expected.
///
/////////////////////////////////////////////////////////////////////////////
void ApexDiagArm::TestComparisions()
{
    UINT32 cpsr = 0;

    // TEQ. EORS without result.
    // 1 ^ 1
    cpsr = ArmInstrTeq( 1, 1 );
    ASSERT( (cpsr & CPSR_Z) != 0 );

    // -1 ^ 1
    cpsr = ArmInstrTeq( -1, 1 );
    ASSERT( (cpsr & CPSR_N) != 0 );

    // 0 ^ (0x80000000 << 1)
    cpsr = ArmInstrTeqCarry( 0, 0x80000000 );
    ASSERT( (cpsr & CPSR_C) != 0 );


    // TST. ANDS without result.
    // 0x55555555 & 0xaaaaaaaa
    cpsr = ArmInstrTst( 0x55555555, 0xaaaaaaaa );
    ASSERT( (cpsr & CPSR_Z) != 0 );

    // -1 & -1
    cpsr = ArmInstrTst( -1, -1 );
    ASSERT( (cpsr & CPSR_N) != 0 );

    // 0 & (0x80000000 << 1)
    cpsr = ArmInstrTstCarry( 0, 0x80000000 );
    ASSERT( (cpsr & CPSR_C) != 0 );


    // CMP
    // 1 - 1
    cpsr = ArmInstrCmp( 1, 1 );
    ASSERT( (cpsr & CPSR_Z) != 0 );

    // 1 - 2
    cpsr = ArmInstrCmp( 1, 2 );
    ASSERT( (cpsr & CPSR_N) != 0 );

    // 0 - (0x80000000 << 1)
    cpsr = ArmInstrCmpCarry( 0, 0x80000000 );
    ASSERT( (cpsr & CPSR_C) != 0 );

    // LONG_MIN - 1
    cpsr = ArmInstrCmp( LONG_MIN, 1 );
    ASSERT( (cpsr & CPSR_V) != 0 );


    // CMN
    // -2 + 2
    cpsr = ArmInstrCmn( -2, 2 );
    ASSERT( (cpsr & CPSR_Z) != 0 );

    // -2 + 1
    cpsr = ArmInstrCmn( -2, 1 );
    ASSERT( (cpsr & CPSR_N) != 0 );

    // 0xffffffff + 0xffffffff
    cpsr = ArmInstrCmn( ULONG_MAX, ULONG_MAX );
    ASSERT( (cpsr & CPSR_C) != 0 );

    // LONG_MAX + 1
    cpsr = ArmInstrCmn( LONG_MAX, 1 );
    ASSERT( (cpsr & CPSR_V) != 0 );
}

/////////////////////////////////////////////////////////////////////////////
// FUNCTION NAME: TestConditionalExecution
//
/// Tests ARM conditional execution.
///
/// @par Full Description
/// Calls ARM conditional execution test functions.
/// Checks if results are as expected.
///
/////////////////////////////////////////////////////////////////////////////
void ApexDiagArm::TestConditionalExecution()
{
    // EQ
    ASSERT( true == ArmInstrConditionEq() );

    // NE
    ASSERT( true == ArmInstrConditionNe() );

    // HS
    ASSERT( true == ArmInstrConditionHs() );

    // LO
    ASSERT( true == ArmInstrConditionLo() );

    // MI
    ASSERT( true == ArmInstrConditionMi() );

    // PL
    ASSERT( true == ArmInstrConditionPl() );

    // VS
    ASSERT( true == ArmInstrConditionVs() );

    // VC
    ASSERT( true == ArmInstrConditionVc() );

    // HI
    ASSERT( true == ArmInstrConditionHi() );

    // LS
    ASSERT( 3 == ArmInstrConditionLs() );

    // GE
    ASSERT( 2 == ArmInstrConditionGe() );

    // LT
    ASSERT( 2 == ArmInstrConditionLt() );

    // GT
    ASSERT( 2 == ArmInstrConditionGt() );

    // LE
    ASSERT( 6 == ArmInstrConditionLe() );
}

/////////////////////////////////////////////////////////////////////////////
// FUNCTION NAME: TestSigned16BitMultiplication
//
/// Tests ARM signed 16 multiplication instructions.
///
/// @par Full Description
/// Calls ARM signed 16 multiplication instructions test functions
/// with prepared parameters.
/// Checks if results are as expected.
///
/////////////////////////////////////////////////////////////////////////////
void ApexDiagArm::TestSigned16BitMultiplication()
{
    // SMULBB
    ASSERT( 15 == ArmInstrSmulbb( 0x00020003, 0x00040005 ) );

    // SMULBT
    ASSERT( 12 == ArmInstrSmulbt( 0x00020003, 0x00040005 ) );

    // SMULTB
    ASSERT( 10 == ArmInstrSmultb( 0x00020003, 0x00040005 ) );

    // SMULTT
    ASSERT( 8 == ArmInstrSmultt( 0x00020003, 0x00040005 ) );

    // SMLABB
    ASSERT( 16 == ArmInstrSmlabb( 0x00020003, 0x00040005, 0x00000001 ) );

    // SMLABT
    ASSERT( 13 == ArmInstrSmlabt( 0x00020003, 0x00040005, 0x00000001 ) );

    // SMLATB
    ASSERT( 11 == ArmInstrSmlatb( 0x00020003, 0x00040005, 0x00000001 ) );

    // SMLATT
    ASSERT( 9 == ArmInstrSmlatt( 0x00020003, 0x00040005, 0x00000001 ) );

    // SMULWB
    ASSERT( 0x000002FF == ArmInstrSmulwb( 0x00ffffff, 0x00020003 ) );

    // SMULWT
    ASSERT( 0x000001FF == ArmInstrSmulwt( 0x00ffffff, 0x00020003 ) );

    // SMLAWB
    ASSERT( 0x7FFF02FF == ArmInstrSmlawb( 0x00ffffff, 0x00020003, 0x7fff0000 ) );

    // SMLAWT
    ASSERT( 0x7FFF01FF == ArmInstrSmlawt( 0x00ffffff, 0x00020003, 0x7fff0000 ) );
}

/////////////////////////////////////////////////////////////////////////////
// FUNCTION NAME: TestSingleRegisterDataTransfer
//
/// Tests ARM single register data transfer instructions.
///
/// @par Full Description
/// Calls ARM single register data transfer instructions test functions
/// with prepared parameters.
/// Checks if results are as expected.
///
/////////////////////////////////////////////////////////////////////////////
void ApexDiagArm::TestSingleRegisterDataTransfer()
{
    UINT32 testValue = 0x01028384;

    // LDR
    ASSERT( 0x01028384 == ArmInstrLdr( &testValue ) );

    // LDRB
    ASSERT( 0x00000084 == ArmInstrLdrb( &testValue ) );

    // LDRH
    ASSERT( 0x00008384 == ArmInstrLdrh( &testValue ) );

    // LDRSB
    ASSERT( 0xFFFFFF84 == ArmInstrLdrsb( &testValue ) );

    // LDRSH
    ASSERT( 0xFFFF8384 == ArmInstrLdrsh( &testValue ) );

    // STR
    testValue = 0;
    ArmInstrStr( &testValue, 0x01028384 );
    ASSERT( 0x01028384 == testValue );

    // STRB
    testValue = 0;
    ArmInstrStrb( &testValue, 0x01028384 );
    ASSERT( 0x00000084 == testValue );

    // STRH
    testValue = 0;
    ArmInstrStrh( &testValue, 0x01028384 );
    ASSERT( 0x00008384 == testValue );
}

/////////////////////////////////////////////////////////////////////////////
// FUNCTION NAME: TestMultipleRegisterDataTransfer
//
/// Tests ARM LDMFD STMFD instructions.
///
/// @par Full Description
/// Calls ARM LDMFD STMFD instructions test functions.
/// Checks if result is as expected.
///
/////////////////////////////////////////////////////////////////////////////
void ApexDiagArm::TestMultipleRegisterDataTransfer()
{
    // LDMFD STMFD. Store multiple. Load multiple.
    ASSERT( 7 == ArmInstrStmfdLdmfd() );
}

/////////////////////////////////////////////////////////////////////////////
// FUNCTION NAME: TestCountLeadingZeros
//
/// Tests ARM CLZ instruction.
///
/// @par Full Description
/// Calls ARM CLZ instruction test.
/// Checks if results is as expected.
///
/////////////////////////////////////////////////////////////////////////////
void ApexDiagArm::TestCountLeadingZeros()
{
    // CLZ. Count leading zeros.
    ASSERT( 31 == ArmInstrClz( 0x00000001 ) );
}

/////////////////////////////////////////////////////////////////////////////
// FUNCTION NAME: TestOperand2
//
/// Tests ARM instruction operand 2 versions.
///
/// @par Full Description
/// Calls ARM instruction operand 2 versions test functions
/// with prepared parameters.
/// Checks if results are as expected.
///
/////////////////////////////////////////////////////////////////////////////
void ApexDiagArm::TestOperand2()
{
    // Immediate
    ASSERT( true == ArmOperand2Immediate() );

    // LSL Immediate
    ASSERT( 0xaaaaaaaa == ArmOperand2LslImmediate( 0x55555555 ) );

    // LSR Immediate
    ASSERT( 0x55555555 == ArmOperand2LsrImmediate( 0xaaaaaaaa ) );

    // ASR Immediate
    ASSERT( 0xC0000001 == ArmOperand2AsrImmediate( 0x80000002 ) );

    // Register
    ASSERT( 0x00000002 == ArmOperand2Register( 0x00000001, 0x00000002 ) );

    // LSL Register
    ASSERT( 0xaaaaaaaa == ArmOperand2LslRegister( 0x55555555, 1 ) );

    // LSR Register
    ASSERT( 0x55555555 == ArmOperand2LsrRegister( 0xaaaaaaaa, 1 ) );

    // ASR Register
    ASSERT( 0xC0000001 == ArmOperand2AsrRegister( 0x80000002, 1 ) );

    // ROR Register
    ASSERT( 0xC0000000 == ArmOperand2RorRegister( 0x80000001, 1 ) );

    // RXX Immediate
    ASSERT( 0x80000001 == ArmOperand2Rxx( 0x00000002 ) );
}

/////////////////////////////////////////////////////////////////////////////
// FUNCTION NAME: TestAddressingModes
//
/// Tests ARM addressing modes.
///
/// @par Full Description
/// Calls ARM addressing modes test functions
/// with prepared parameters.
/// Checks if results are as expected.
///
/////////////////////////////////////////////////////////////////////////////
void ApexDiagArm::TestAddressingModes()
{
    UINT32 testArray[] = { 1, 2 };

    // No offset
    ASSERT( testArray[ 0 ] == ArmAddrModeNoOffset( &testArray[ 0 ] ) );

    // Immediate offset positive
    ASSERT( testArray[ 1 ] == ArmAddrModeImmediateOffsetPositive( &testArray[ 0 ] ) );

    // Immediate offset negative
    ASSERT( testArray[ 0 ] == ArmAddrModeImmediateOffsetNegative( &testArray[ 1 ] ) );

    // Offset register
    ASSERT( testArray[ 1 ] == ArmAddrModeOffsetRegister( &testArray[ 0 ], 4 ) );

    // Offset register shift
    ASSERT( testArray[ 1 ] == ArmAddrModeOffsetRegisterShift( &testArray[ 0 ], 1 ) );

    // Pre indexed immediate offset
    ASSERT( &testArray[ 1 ] == ArmAddrModePreInxedImmediateOffset( &testArray[ 0 ], testArray[ 1 ] ) );

    // Pre indexed register offset
    ASSERT( &testArray[ 1 ] == ArmAddrModePreInxedOffsetRegister( &testArray[ 0 ], 4, testArray[ 1 ] ) );

    // Pre indexed register shift offset
    ASSERT( &testArray[ 1 ] == ArmAddrModePreInxedOffsetRegisterShift( &testArray[ 0 ], 1, testArray[ 1 ] ) );

    // Post indexed immediate offset
    ASSERT( &testArray[ 1 ] == ArmAddrModePostInxedImmediateOffset( &testArray[ 0 ], testArray[ 0 ] ) );

    // Post indexed register offset
    ASSERT( &testArray[ 1 ] == ArmAddrModePostInxedOffsetRegister( &testArray[ 0 ], 4, testArray[ 0 ] ) );

    // Post indexed register shift offset
    ASSERT( &testArray[ 1 ] == ArmAddrModePostInxedOffsetRegisterShift( &testArray[ 0 ], 1, testArray[ 0 ] ) );

}

/////////////////////////////////////////////////////////////////////////////
// FUNCTION NAME: TestInstructions
//
/// Tests ARM instructions.
///
/// @par Full Description
/// Calls ARM instructions test functions.
///
/////////////////////////////////////////////////////////////////////////////
void ApexDiagArm::TestInstructions()
{
    TestArithmeticOperations();
    TestDataMovement();
    TestLogicalOperations();
    TestComparisions();
    TestConditionalExecution();
    TestSigned16BitMultiplication();
    TestSingleRegisterDataTransfer();
    TestMultipleRegisterDataTransfer();
    TestCountLeadingZeros();
    TestOperand2();
    TestAddressingModes();
}

/////////////////////////////////////////////////////////////////////////////
// FUNCTION NAME: TestRegister
//
/// Tests register.
///
/// @par Full Description
/// Calls register test function with combinations of arguments from
/// pattern set. Checks if results are as expected.
///
/// @param    testRegisterFunctionPtr    Pointer register test function.
///
/////////////////////////////////////////////////////////////////////////////
void ApexDiagArm::TestRegister( TestRegisterFunctionPtr testRegisterFunctionPtr )
{
    // rotated
    ASSERT( true == (*testRegisterFunctionPtr)( f_patternSet[ 0 ], f_patternSet[ 1 ] ) );
    ASSERT( true == (*testRegisterFunctionPtr)( f_patternSet[ 1 ], f_patternSet[ 2 ] ) );
    ASSERT( true == (*testRegisterFunctionPtr)( f_patternSet[ 2 ], f_patternSet[ 3 ] ) );
    ASSERT( true == (*testRegisterFunctionPtr)( f_patternSet[ 3 ], f_patternSet[ 0 ] ) );

    // inverted
    ASSERT( true == (*testRegisterFunctionPtr)( f_patternSet[ 0 ], ~f_patternSet[ 0 ] ) );
    ASSERT( true == (*testRegisterFunctionPtr)( f_patternSet[ 1 ], ~f_patternSet[ 1 ] ) );
    ASSERT( true == (*testRegisterFunctionPtr)( f_patternSet[ 2 ], ~f_patternSet[ 2 ] ) );
    ASSERT( true == (*testRegisterFunctionPtr)( f_patternSet[ 3 ], ~f_patternSet[ 3 ] ) );
}

/////////////////////////////////////////////////////////////////////////////
// FUNCTION NAME: TestRegistersR0R12
//
/// Tests ARM registers R0 - R12.
///
/// @par Full Description
/// Calls ARM registers test functions for registers R0 to R12.
///
/////////////////////////////////////////////////////////////////////////////
void ApexDiagArm::TestRegistersR0R12()
{
    TestRegister( ArmRegisterR0 );
    TestRegister( ArmRegisterR1 );
    TestRegister( ArmRegisterR2 );
    TestRegister( ArmRegisterR3 );
    TestRegister( ArmRegisterR4 );
    TestRegister( ArmRegisterR5 );
    TestRegister( ArmRegisterR6 );
    TestRegister( ArmRegisterR7 );
    TestRegister( ArmRegisterR8 );
    TestRegister( ArmRegisterR9 );
    TestRegister( ArmRegisterR10 );
    TestRegister( ArmRegisterR11 );
    TestRegister( ArmRegisterR12 );
}

/////////////////////////////////////////////////////////////////////////////
// FUNCTION NAME: TestRegistersSpLr
//
/// Tests ARM SP and LR registers in all ARM modes (excluding fiq mode).
///
/// @par Full Description
/// Calls ARM registers test functions for SP and LR registers in all
/// ARM modes (excluding fiq mode).
///
/////////////////////////////////////////////////////////////////////////////
void ApexDiagArm::TestRegistersSpLr()
{
    TestRegister( ArmRegisterSpSystem );
    TestRegister( ArmRegisterLrSystem );
    TestRegister( ArmRegisterSpSupervisor );
    TestRegister( ArmRegisterLrSupervisor );
    TestRegister( ArmRegisterSpAbort );
    TestRegister( ArmRegisterLrAbort );
    TestRegister( ArmRegisterSpUndefined );
    TestRegister( ArmRegisterLrUndefined );
    TestRegister( ArmRegisterSpIrq );
    TestRegister( ArmRegisterLrIrq );
}

/////////////////////////////////////////////////////////////////////////////
// FUNCTION NAME: TestRegistersFiqCpsr
//
/// Tests ARM fiq mode specific registers, and CPSR register.
///
/// @par Full Description
/// Calls ARM registers test functions for all registers specific to fiq
/// mode. Calls CPSR register test.
///
/////////////////////////////////////////////////////////////////////////////
void ApexDiagArm::TestRegistersFiqCpsr()
{
    TestRegister( ArmRegisterSpFiq );
    TestRegister( ArmRegisterLrFiq );
    TestRegister( ArmRegisterR8Fiq );
    TestRegister( ArmRegisterR9Fiq );
    TestRegister( ArmRegisterR10Fiq );
    TestRegister( ArmRegisterR11Fiq );
    TestRegister( ArmRegisterR12Fiq );
    ASSERT( 2 == ArmRegisterCpsr() );
}

/////////////////////////////////////////////////////////////////////////////
// FUNCTION NAME: ArmPowerUp
//
/// Tests ARM instructions and registers at powerup.
///
/// @par Full Description
/// Calls ARM test functions.
///
/////////////////////////////////////////////////////////////////////////////
void ApexDiagArm::ArmPowerUp()
{
    // disable interrupts and remember their state before disabling
    const bool irqEnabled = HI_DisableIrq();
    const bool fiqEnabled = HI_DisableFiq();

    // call all test stages
    for ( UINT32 testStage = 0;
          testStage < ( sizeof( f_testStageFunctionPtrs ) / sizeof( *f_testStageFunctionPtrs ) );
          ++testStage )
    {
        // call test stage function
        (*f_testStageFunctionPtrs[ testStage ])();
    }

    // enable interrupts if they were enabled before
    if ( fiqEnabled )
    {
        HI_EnableFiq();
    }
    if ( irqEnabled )
    {
        HI_EnableIrq();
    }

    // Start Fault Injection Point 1
    // Code which resets InjectFaultFlag will be injected here.
    // End Fault Injection Point 1
}

/////////////////////////////////////////////////////////////////////////////
// FUNCTION NAME: RunTest
//
/// Tests ARM instructions and registers.
///
/// @par Full Description
/// Calls ARM test functions.
///
/// @param  not used
///
/// @return         ApexDiagnostic::TestState
///     @retval     TEST_LOOP_COMPLETE   if test has finished. 
///     @retval     TEST_IN_PROGRESS     if test is still in progress.
/////////////////////////////////////////////////////////////////////////////
ApexDiagnostic::TestState ApexDiagArm::RunTest( ApexDiagnostic::DiagControlBlock * )
{
    static UINT32 testStage = 0;

    // disable interrupts and remember their state before disabling
    const bool irqEnabled = HI_DisableIrq();
    const bool fiqEnabled = HI_DisableFiq();

    // call test stage function
    (*f_testStageFunctionPtrs[ testStage ])();

    // enable interrupts if they were enabled before
    if ( fiqEnabled )
    {
        HI_EnableFiq();
    }
    if ( irqEnabled )
    {
        HI_EnableIrq();
    }

    ++testStage;

    // if all stages are complete
    if ( testStage == ( sizeof( f_testStageFunctionPtrs ) / sizeof( *f_testStageFunctionPtrs ) ) )
    {
        testStage = 0;
        return ApexDiagnostic::TEST_LOOP_COMPLETE;
    }

    return ApexDiagnostic::TEST_IN_PROGRESS;
}

