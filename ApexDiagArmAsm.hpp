///////////////////////////////////////////////////////////////////////////////
/// @file ApexDiagArmAsm.hpp
///
/// @details ARM instructions and registers tests.
///
/// @par Full Description
/// Contains ARM instructions and registers diagnostic functions declarations.
///
/// @if REVISION_HISTORY_INCLUDED
/// @par Edit History
/// -   fzembok	20-Aug-2013 ARM instructions and registers diagnostics
/// -   fzembok 21-Oct-2013 ARM Cache diagnostics
/// -   mgrad   28-NOV-2013 File name changed during MISRA fixes
/// @endif
///
/// @par Copyright (c) 2013 Rockwell Automation Technologies, Inc. All rights reserved.
///////////////////////////////////////////////////////////////////////////////
#ifndef APEXDIAGARMASM_HPP
#define APEXDIAGARMASM_HPP

#include "ApexStdType.hpp"	// for types

extern "C"
{

// Arithmetic operations
UINT32 ArmInstrAdd( UINT32 rn, UINT32 operand2 );
UINT32 ArmInstrAdc( UINT32 rn, UINT32 operand2 );
INT32 ArmInstrQadd( INT32 rm, INT32 rn );
INT32 ArmInstrQdadd( INT32 rm, INT32 rn );
UINT32 ArmInstrSub( UINT32 rn, UINT32 operand2 );
UINT32 ArmInstrSbc( UINT32 rn, UINT32 operand2 );
UINT32 ArmInstrRsb( UINT32 rn, UINT32 operand2 );
UINT32 ArmInstrRsc( UINT32 rn, UINT32 operand2 );
INT32 ArmInstrQsub( INT32 rm, INT32 rn );
INT32 ArmInstrQdsub( INT32 rm, INT32 rn );
UINT32 ArmInstrMul( UINT32 rm, UINT32 rs );
UINT32 ArmInstrMla( UINT32 rm, UINT32 rs, UINT32 rn );
UINT64 ArmInstrUmull( UINT32 rm, UINT32 rs );
UINT64 ArmInstrUmlal( UINT64 rd, UINT32 rm, UINT32 rs );

// SMULL and SMLAL instructions return result of INT64 type
// Apex building toolchain does not support INT64 type
// INT64 in SMULL and SMLAL instructions were replaced with UINT64
UINT64 ArmInstrSmull( INT32 rm, INT32 rs );
UINT64 ArmInstrSmlal( UINT64 rd, INT32 rm, INT32 rs );

// Data movement
UINT32 ArmInstrMov( UINT32 dummy, UINT32 operand2 );
UINT32 ArmInstrMvn( UINT32 dummy, UINT32 operand2 );

// Logical operations
UINT32 ArmInstrAnd( UINT32 rn, UINT32 operand2 );
UINT32 ArmInstrOrr( UINT32 rn, UINT32 operand2 );
UINT32 ArmInstrEor( UINT32 rn, UINT32 operand2 );
UINT32 ArmInstrBic( UINT32 rn, UINT32 operand2 );

// Comparisions
UINT32 ArmInstrTeq( UINT32 rn, UINT32 operand2 );
UINT32 ArmInstrTeqCarry( UINT32 rn, UINT32 operand2 );
UINT32 ArmInstrTst( UINT32 rn, UINT32 operand2 );
UINT32 ArmInstrTstCarry( UINT32 rn, UINT32 operand2 );
UINT32 ArmInstrCmp( INT32 rn, INT32 operand2 );
UINT32 ArmInstrCmpCarry( INT32 rn, INT32 operand2 );
UINT32 ArmInstrCmn( INT32 rn, INT32 operand2 );

// Conditional execution
bool ArmInstrConditionEq();
bool ArmInstrConditionNe();
bool ArmInstrConditionHs();
bool ArmInstrConditionLo();
bool ArmInstrConditionMi();
bool ArmInstrConditionPl();
bool ArmInstrConditionVs();
bool ArmInstrConditionVc();
bool ArmInstrConditionHi();
UINT32 ArmInstrConditionLs();
UINT32 ArmInstrConditionGe();
UINT32 ArmInstrConditionLt();
UINT32 ArmInstrConditionGt();
UINT32 ArmInstrConditionLe();

// Signed 16 multiplication
INT32 ArmInstrSmulbb( INT32 rn, INT32 rm );
INT32 ArmInstrSmulbt( INT32 rn, INT32 rm );
INT32 ArmInstrSmultb( INT32 rn, INT32 rm );
INT32 ArmInstrSmultt( INT32 rn, INT32 rm );
INT32 ArmInstrSmlabb( INT32 rn, INT32 rm, INT32 rs );
INT32 ArmInstrSmlabt( INT32 rn, INT32 rm, INT32 rs );
INT32 ArmInstrSmlatb( INT32 rn, INT32 rm, INT32 rs );
INT32 ArmInstrSmlatt( INT32 rn, INT32 rm, INT32 rs );
INT32 ArmInstrSmulwb( INT32 rn, INT32 rm );
INT32 ArmInstrSmulwt( INT32 rn, INT32 rm );
INT32 ArmInstrSmlawb( INT32 rn, INT32 rm, INT32 rs );
INT32 ArmInstrSmlawt( INT32 rn, INT32 rm, INT32 rs );

// Single register data transfer
UINT32 ArmInstrLdr( UINT32* address );
UINT32 ArmInstrLdrb( UINT32* address );
UINT32 ArmInstrLdrh( UINT32* address );
UINT32 ArmInstrLdrsb( UINT32* address );
UINT32 ArmInstrLdrsh( UINT32* address );
void ArmInstrStr( UINT32* address, UINT32 rd );
void ArmInstrStrb( UINT32* address, UINT32 rd );
void ArmInstrStrh( UINT32* address, UINT32 rd );

// Multiple register data transfer
UINT32 ArmInstrStmfdLdmfd();

// Count leading zeros
UINT32 ArmInstrClz( UINT32 rm );

// Operand 2
bool ArmOperand2Immediate();
UINT32 ArmOperand2LslImmediate( UINT32 rm );
UINT32 ArmOperand2LsrImmediate( UINT32 rm );
UINT32 ArmOperand2AsrImmediate( UINT32 rm );
UINT32 ArmOperand2Register( UINT32 dummy, UINT32 rm );
UINT32 ArmOperand2LslRegister( UINT32 rm, UINT32 rs );
UINT32 ArmOperand2LsrRegister( UINT32 rm, UINT32 rs );
UINT32 ArmOperand2AsrRegister( UINT32 rm, UINT32 rs );
UINT32 ArmOperand2RorRegister( UINT32 rm, UINT32 rs );
UINT32 ArmOperand2Rxx( UINT32 rm );

// Addressing modes
UINT32 ArmAddrModeNoOffset( UINT32* address );
UINT32 ArmAddrModeImmediateOffsetPositive( UINT32* address );
UINT32 ArmAddrModeImmediateOffsetNegative( UINT32* address );
UINT32 ArmAddrModeOffsetRegister( UINT32* address, UINT32 offset );
UINT32 ArmAddrModeOffsetRegisterShift( UINT32* address, UINT32 offset );
UINT32* ArmAddrModePreInxedImmediateOffset( UINT32* address, UINT32 expectedValue );
UINT32* ArmAddrModePreInxedOffsetRegister( UINT32* address, UINT32 offset, UINT32 expectedValue );
UINT32* ArmAddrModePreInxedOffsetRegisterShift( UINT32* address, UINT32 offset, UINT32 expectedValue );
UINT32* ArmAddrModePostInxedImmediateOffset( UINT32* address, UINT32 expectedValue );
UINT32* ArmAddrModePostInxedOffsetRegister( UINT32* address, UINT32 offset, UINT32 expectedValue );
UINT32* ArmAddrModePostInxedOffsetRegisterShift( UINT32* address, UINT32 offset, UINT32 expectedValue );

// Registers
bool ArmRegisterR0( UINT32 pattern1, UINT32 pattern2 );
bool ArmRegisterR1( UINT32 pattern1, UINT32 pattern2 );
bool ArmRegisterR2( UINT32 pattern1, UINT32 pattern2 );
bool ArmRegisterR3( UINT32 pattern1, UINT32 pattern2 );
bool ArmRegisterR4( UINT32 pattern1, UINT32 pattern2 );
bool ArmRegisterR5( UINT32 pattern1, UINT32 pattern2 );
bool ArmRegisterR6( UINT32 pattern1, UINT32 pattern2 );
bool ArmRegisterR7( UINT32 pattern1, UINT32 pattern2 );
bool ArmRegisterR8( UINT32 pattern1, UINT32 pattern2 );
bool ArmRegisterR9( UINT32 pattern1, UINT32 pattern2 );
bool ArmRegisterR10( UINT32 pattern1, UINT32 pattern2 );
bool ArmRegisterR11( UINT32 pattern1, UINT32 pattern2 );
bool ArmRegisterR12( UINT32 pattern1, UINT32 pattern2 );
bool ArmRegisterSpSystem( UINT32 pattern1, UINT32 pattern2 );
bool ArmRegisterLrSystem( UINT32 pattern1, UINT32 pattern2 );
bool ArmRegisterSpSupervisor( UINT32 pattern1, UINT32 pattern2 );
bool ArmRegisterLrSupervisor( UINT32 pattern1, UINT32 pattern2 );
bool ArmRegisterSpAbort( UINT32 pattern1, UINT32 pattern2 );
bool ArmRegisterLrAbort( UINT32 pattern1, UINT32 pattern2 );
bool ArmRegisterSpUndefined( UINT32 pattern1, UINT32 pattern2 );
bool ArmRegisterLrUndefined( UINT32 pattern1, UINT32 pattern2 );
bool ArmRegisterSpIrq( UINT32 pattern1, UINT32 pattern2 );
bool ArmRegisterLrIrq( UINT32 pattern1, UINT32 pattern2 );
bool ArmRegisterSpFiq( UINT32 pattern1, UINT32 pattern2 );
bool ArmRegisterLrFiq( UINT32 pattern1, UINT32 pattern2 );
bool ArmRegisterR8Fiq( UINT32 pattern1, UINT32 pattern2 );
bool ArmRegisterR9Fiq( UINT32 pattern1, UINT32 pattern2 );
bool ArmRegisterR10Fiq( UINT32 pattern1, UINT32 pattern2 );
bool ArmRegisterR11Fiq( UINT32 pattern1, UINT32 pattern2 );
bool ArmRegisterR12Fiq( UINT32 pattern1, UINT32 pattern2 );
UINT32 ArmRegisterCpsr();

}

#endif //APEXDIAGARMASM_HPP
