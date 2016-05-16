#ifndef _COMPILER_H
#define _COMPILER_H


/******************************************************************************/
//                            Arbitrary Limits
/******************************************************************************/


#define UNQ_LABEL_SZ 12 // string lngth limit for compiler generated labels
#define NAME_MAX     64 // symbol name length limit. not enforced


/******************************************************************************/
//                            Type Definitions
/******************************************************************************/


#include <stdint.h>
typedef unsigned long long umax;
typedef unsigned int uint;

typedef enum {Real, Protected, Virtual, SMM, Compatibility, Long} mode_t;
typedef enum { void_t, byte, word, dword, qword } regsz_t;
typedef enum{
	IMM,
	R0, R1, R2 , R3 , R4 , R5 , R6 , R7 ,
	//R8, R9, R10, R11, R12, R13, R14,
	R15,
	// x86
	A , B , C  , D  , SI , DI , BP ,
	// arm
	//A1, A2, A3 , A4 , V1 , V2 , V3 , V4 ,
	//V5, V6, V7 , V8 , WR , SB , SL , FP ,
	//IP, LR, PC,
	SP // stack pointer x86=R7, arm=R13
} reg_t;
typedef struct {
	char name[NAME_MAX];
	umax initial_value;
	regsz_t size;
	uint8_t flags;
}sym_entry;

#define S_FUNCT  ((uint8_t) (1<<0))
#define S_SIGN   ((uint8_t) (1<<1))
#define S_CONST  ((uint8_t) (1<<2))
#define S_STATIC ((uint8_t) (1<<3))
#define S_USED   ((uint8_t) (1<<4))
#define S_TYPDEF ((uint8_t) (1<<5)) // symbol is a defined type


/******************************************************************************/
//                            Collected Headers
/******************************************************************************/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <data.h>

#include "globals.h"
#include "tokens.h"
#include "functions.h"


#endif // _COMPILER_H
