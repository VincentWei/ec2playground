/*
  Losu Copyright Notice
  --------------------
    Losu is an open source programming language project under the MIT license
  that can be used for both academic and commercial purposes. There are no
  fees, no royalties, and no GNU-like "copyright" restrictions. Losu qualifies
  as open source software. However, Losu is not public property, and founder
  'chen-chaochen' retains its copyright.

    Losu has been registered with the National Copyright Administration of the
  People's Republic of China, and adopts the MIT license as the copyright
  licensing contract under which the right holder conditionally licenses its
  reproduction, distribution, and modification rights to an unspecified public.

    If you use Losu, please follow the public MIT agreement or choose to enter
  into a dedicated license agreement with us.

  The MIT LICENSE is as follows
  --------------------
  Copyright  2020  chen-chaochen

    Permission is hereby granted, free of charge, to any person obtaining a
  copy of this software and associated documentation files (the “Software”), to
  deal in the Software without restriction, including without limitation the
  rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
  sell copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
  SOFTWARE.
*/

/**
 * This set of code files (losuBytecode.h & losuBytecode.c) defines the
 * bytecode subsystem of Losu, including the V-MIS (Virtual Machine Instruction
 * Set), Instruction-Formats, Arithmetic&Logical Instructions.
 */

#ifndef DEFINE_INCLUDE_LOSU_BYTECODE_H
#define DEFINE_INCLUDE_LOSU_BYTECODE_H

#include "losu.h"

/**
 * Ins = {
 *    OP
 *    OP + U
 *    OP + S
 *    OP + B + A
 * } + E
 * OP: 8 bit uint8_t
 * B: 8 bit uint8_t
 * A: 32 bit uint32_t
 * U: 32 bit uint32_t
 * S: 32 bit int32_t
`* E: 16 bit any
 * */

typedef uint8_t vmIns_OP;
typedef uint32_t vmIns_U;
typedef int32_t vmIns_S;
typedef uint32_t vmIns_A;
typedef uint8_t vmIns_B;
#define vmIns_SizeIns 64
#define vmIns_SizeOP 8
#define vmIns_SizeB 8
#define vmIns_SizeA 32
#define vmIns_SizeU 32
#define vmIns_SizeS 32
#define vmIns_SizeE 16

#define vmIns_PosOP 0
#define vmIns_PosS 8
#define vmIns_PosU 8
#define vmIns_PosE 48
#define vmIns_PosB 8
#define vmIns_PosA 16

#define vmIns_MaxU (((vmInstruction)(1) << vmIns_SizeU) - 1)
#define vmIns_MaxS (vmIns_MaxU >> 1)
#define vmIns_MaxB (((vmInstruction)(1) << vmIns_SizeB) - 1)
#define vmIns_MaxA (((vmInstruction)(1) << vmIns_SizeA) - 1)

/* Vm Opereate Limits  */
#define vmOl_MaxStack 255     /* Maximum stack depth for a single expression */
#define vmOl_MaxLocalvar 128  /* < vmOl_MaxStack */
#define vmOl_MaxClosure 32    /* < vmOl_MaxStack */
#define vmOl_MaxVared2left 64 /* < vmOl_MaxReturn */
#define vmOl_MaxFuncarg 32
#define vmOl_MaxSetlist 32 /* < vmOl_MaxStack / 4  */
#define vmOl_MaxSetmap (vmOl_MaxSetlist / 2)
#define vmOl_MaxLookback 32 /* space cost to reduce time for codegen */
#define vmOl_MaxReturn 255
#define vmOl_MaxStrlen SIZE_MAX
#define vmOl_MaxSize_t SIZE_MAX
#define vmOl_MaxInt32_t INT_MAX
#define vmOl_MaxNumberRef 0 /* synatx number max */

#define cgIns_Mask1(n, p)                                                     \
  ((~((~(vmInstruction)0) << n)) << p)          /* lmov n bit 1 to p */
#define cgIns_Mask0(n, p) (~cgIns_Mask1 (n, p)) /* lmov n bit 0 to p */

#define cgIns_NewOp(op) ((vmInstruction)(op))
// #define cgIns_GetOp(i) ((vmIns_OP)((i) & cgIns_Mask1 (vmIns_SizeOP, 0)))
#define cgIns_GetOp(i) ((vmIns_OP)(i))
#define cgIns_SetOp(i, op)                                                    \
  ((i) = (((vmInstruction)(i) & cgIns_Mask0 (vmIns_SizeOP, 0))                \
          | (vmInstruction)(op)))

#define cgIns_NewU(op, u)                                                     \
  ((vmInstruction)(op) | ((vmInstruction)(u) << vmIns_PosU))
#define cgIns_GetU(i) ((vmIns_U)((vmInstruction)(i) >> vmIns_PosU))
#define cgIns_SetU(i, u)                                                      \
  ((i) = (((i) & cgIns_Mask0 (vmIns_SizeU, vmIns_PosU))                       \
          | ((vmInstruction)(u) << vmIns_PosU)))

#define cgIns_NewS(op, s) (cgIns_NewU ((op), (s) + vmIns_MaxS))
#define cgIns_GetS(i) ((vmIns_S)(cgIns_GetU (i) - vmIns_MaxS))
#define cgIns_SetS(i, s) (cgIns_SetU (i, (s) + vmIns_MaxS))

#define cgIns_NewAB(op, a, b)                                                 \
  (((vmInstruction)(op)) | ((vmInstruction)(a) << vmIns_PosA)                 \
   | ((vmInstruction)(b) << vmIns_PosB))
#define cgIns_GetA(i) (((vmIns_A)((i) >> vmIns_PosA)))
#define cgIns_SetA(i, a)                                                      \
  ((i) = (((i) & cgIns_Mask0 (vmIns_SizeA, vmIns_PosA))                       \
          | ((vmInstruction)(a) << vmIns_PosA)))
#define cgIns_GetB(i)                                                         \
  ((vmIns_B)(((i) >> vmIns_PosB) /* & cgIns_Mask1 (vmIns_SizeB, 0) */))
#define cgIns_SetB(i, b)                                                      \
  ((i) = (((i) & cgIns_Mask0 (vmIns_SizeB, vmIns_PosB))                       \
          | ((vmInstruction)(b) << vmIns_PosB)))

/* Op-code */
typedef enum
{
  INS_END,    /* iO   0   0 */
  INS_RETURN, /* iU   0   0 */

  INS_CALL, /* iBA  0   0 */

  INS_PUSHNULL, /* iU   V   0 */
  INS_POP,      /* iU   V   0 */

  INS_PUSHSTRING,   /* iU   1   0 */
  INS_PUSHNUM,      /* iU   1   0 */
  INS_PUSHUPVALUE,  /* iU   1   0 */
  INS_PUSHFUNCTION, /* iBA  V   0 */
  INS_PUSHSELF,     /* iU   2   1 */

  INS_GETLOCAL,  /* iU   1   0 */
  INS_GETGLOBAL, /* iU   1   0 */
  INS_SETLOCAL,  /* iU   0   1 */
  INS_SETGLOBAL, /* iU   0   1 */

  INS_CREATEUNIT, /* iBA   1   0 */
  INS_SETUNIT,    /* iBA  V   0 */
  INS_GETUNIT,    /* iBA  1   2 */
  INS_SETLIST,    /* iBA  V   0 */
  INS_SETMAP,     /* iU   V   0 */

  INS_ADD,    /* iO   1   2 */
  INS_SUB,    /* iO   1   2 */
  INS_MULT,   /* iO   1   2 */
  INS_DIV,    /* iO   1   2 */
  INS_POW,    /* iO   1   2 */
  INS_CONCAT, /* iO   1   2 */
  INS_NEG,    /* iO   1   1 */
  INS_NOT,    /* iO   1   1 */

  INS_JMPNE, /* iS   0   2 */
  INS_JMPEQ, /* iS   0   2 */
  INS_JMPLT, /* iS   0   2 */
  INS_JMPLE, /* iS   0   2 */
  INS_JMPGT, /* iS   0   2 */
  INS_JMPGE, /* iS   0   2 */

  INS_JMPT,   /* iS   0   1 */
  INS_JMPF,   /* iS   0   1 */
  INS_JMPONT, /* iS   0   1 */
  INS_JMPONF, /* iS   0   1 */
  INS_JMP,    /* iS   0   0 */

  INS_PUSHNULLJMP, /* iO   0   0 */

  INS_FORPREP,  /* iS   0   0 */
  INS_FORLOOP,  /* iS   0   3 */
  INS_LFORPREP, /* iS   2   0 */
  INS_LFORLOOP, /* iS   0   3 */

  INS_YIELD, /* iU   0   0 */

  EC2INS_PUSHSPACE,   /* iO   1   0 */
  EC2INS_PUSHUNICODE, /* iU   1   0 */
  EC2INS_PUSHCHAR,    /* iU   1   0 */
  EC2INS_PUSHINT,     /* iS   1   0 */
  EC2INS_PUSHBOOL,    /* iU   1   0 */

  EC2INS_BITAND, /* iO   1   2 */
  EC2INS_BITOR,  /* iO   1   2 */
  EC2INS_BITNOT, /* iO   1   1 */
  EC2INS_BITXOR, /* iO   1   2 */
  EC2INS_BITLSH, /* iO   1   2 */
  EC2INS_BITRSH, /* iO   1   2 */

  EC2INS_PUSHSTA, /*iO 0 0 */
  EC2INS_POPSTA,  /* iO 0 0 */
  EC2INS_SETSTA,  /* iO 0 0 */

  EC2INS_SETLOCAL,  /* iU   0   1 */
  EC2INS_SETGLOBAL, /* iU   0   1 */
  EC2INS_SETUNIT,   /* iBA  V   0 */
  EC2INS_PUSHBYTES, /* iU   1   0 */
  vmIns_Number,

} vmInsList;

#define vmIns_Isjump(op) ((INS_JMPNE <= (op) && (op) <= INS_JMP))

typedef enum
{
  INS_BINOP_ADD,  /* + */
  INS_BINOP_SUB,  /* - */
  INS_BINOP_MULT, /* * */
  INS_BINOP_DIV,  /* / */
  INS_BINOP_MOD,  /* ^:-> % */
  INS_BINOP_DIVI, /* &:-> // */

  // bit op
  INS_BINOP_BITAND, /* & */
  INS_BINOP_BITOR,  /* | */
  INS_BINOP_BITXOR, /* ^ */
  INS_BINOP_BITLSH, /* << */
  INS_BINOP_BITRSH, /* >> */

  INS_BINOP_NE, /* != */
  INS_BINOP_EQ, /* == */

  INS_BINOP_LT, /* < */
  INS_BINOP_LE, /* <= */

  INS_BINOP_GT, /* > */
  INS_BINOP_GE, /* >= */

  INS_BINOP_AND, /* and && */
  INS_BINOP_OR,  /* or || */

  INS_BINOP_NULL,
} vmIns_BinOp;

typedef enum
{
  INS_UNOP_NEG,    /* - */
  INS_UNOP_NOT,    /* not */
  INS_UNOP_BITNOT, /* ~ */
  INS_UNOP_NULL,
} vmIns_UnOp;

typedef enum
{
  INS_MODE_OP,
  INS_MODE_IU,
  INS_MODE_IS,
  INS_MODE_IBA,
} vmIns_Mode;
#endif