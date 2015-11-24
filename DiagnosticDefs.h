#pragma once

typedef enum _TestState 
{
    TEST_LOOP_COMPLETE,
    TEST_IN_PROGRESS,
    TEST_FAILURE,
    TEST_IDLE
} 
TestState;

#define DIAG_ERROR_MASK          0x7fffffff

#define DIAG_ERROR_TYPE_BIT_POS  26

