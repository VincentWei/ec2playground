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

/*
 * To introduce Losu into your project, you simply reference this header file
 * in the project source code and link the Losu kernel to the target binary
 */

#ifndef DEFINE_INCLUDE_LOSU_H
#define DEFINE_INCLUDE_LOSU_H

#include <limits.h>
#include <setjmp.h>
#include <stddef.h>
#include <stdint.h>

/* include losu_config.h */
#include "losu_config.h"
#ifndef DEFINE_INCLUDE_LOSUCONFIG_H
#error "Configuration file not found 'losu_config.h'"
#endif /* define_included_losuconfig_h */

#ifndef LosuExtern
#ifdef __cplusplus
#define LosuExtern extern "C"
#else
#define LosuExtern extern
#endif
#endif /* LosuExtern */

#ifndef LosuWeak
#define LosuWeak __attribute__ ((weak))
#endif

/* LosuType */
#ifndef DEFINE_INCLUDE_LOSU_H_LOSUTYPE
#define DEFINE_INCLUDE_LOSU_H_LOSUTYPE
/* C-Losu Type */
typedef double _l_number;                       // 系统浮点数
typedef int32_t _l_int;                         // 系统整数
typedef uint32_t _l_unicode;                    // unicode（字符）
typedef uint8_t _l_bool;                        //  布尔
typedef uint64_t _l_size_t;                     // 内存大小
typedef uint32_t _l_hash;                       // 哈希
typedef int64_t _l_gcint;                       // 虚拟机内使用
typedef uint64_t vmInstruction, _l_instruction; // 虚拟机指令

/* type system */

#define LosuTypeDefine_null 0      // null - False - 未定义、假
#define LosuTypeDefine_number 1    // 系统浮点数
#define LosuTypeDefine_int 2       // 整数：新增
#define LosuTypeDefine_unicode 3   // unicode：新增
#define LosuTypeDefine_char 4   // 字节：新增
#define LosuTypeDefine_string 5    // 字符串
#define LosuTypeDefine_function 6  // 函数
#define LosuTypeDefine_unit 7      // 序列与映射
#define LosuTypeDefine_space 8     // 空
#define LosuTypeDefine_bool 9       // 布尔
#define LosuTypeDefine_coroutine 10 // 携程：冗余
#define LosuTypeDefine_callinfo 11 // 调用信息：冗余
#define LosuTypeDefine_unknown 12

#define LosuErrorCode_Over -1,  /* Catastrophic crash */
#define LosuErrorCode_Ok 0      /* Success */
#define LosuErrorCode_Runtime 2 /* Runtime Error */
#define LosuErrorCode_File 3    /* File Error */
#define LosuErrorCode_Syntax 4  /* Syntax error */
#define LosuErrorCode_Mem 5     /* Memory error */
#define LosuErrorCode_Heap 6    /* Stack Overflow */
#define LosuErrorCode_Library 7 /* Package loading */

#define VMSIGOK 0
#define VMSIGERR 1
#define VMSIGYIELD 2
#define VMSIGKILL 3

#define __losu_sigThrow(vm, sig)                                              \
  {                                                                           \
    if (vm->errjmp->jmpflag)                                                  \
      longjmp (vm->errjmp->jmpflag, sig);                                     \
  }

/* LosuVm */
typedef struct __losuvmStrseg
{
  _l_hash size;
  _l_hash nsize;
  struct _inlineString **strobj;
} __losuvmStrseg;
typedef struct __losuvmJmp
{
  jmp_buf jmpflag;
  struct LosuObj *func; /* catch function */
} __losuvmJmp;
typedef struct __losuvmSTA
{
  struct LosuObj *top;
  struct __losuvmSTA *pre;
} __losuvmSTA;
typedef struct LosuVm
{
  struct LosuObj *top, *stack, *stackmax, *base, *mstack, *mtop, *mstackmax;

  struct __losuvmJmp *errjmp;

  unsigned char *bufftmp;
  _l_size_t nbufftmp;

  struct _inlineScode *inspool;      /* InstructionObj  */
  struct _inlineFunction *funcpool;  /* FunctionObj */
  struct _inlineHash *hashpool;      /* HashObj (unit) */
  struct __losuvmStrseg strpool;     /* StringObj (string) */
  struct _inlineCallinfo *callpool;  /* CallinfoObj (callinfo) */
  struct _inlineCoroutine *coropool; /* CoroutineObj (coroutine) */

  struct _inlineHash *global; /* global var-table */

  _l_gcint gcDymax; /* Dynatic Max Limit */
  _l_gcint nblocks; /* Now size */
  _l_gcint gcMax;   /* All Max */
  _l_gcint gcHook;  /* hookMax */

  int64_t loophook; // 循环计次器
  int64_t callhook; // 调用计次器
  int64_t aluhook;  // 算术计次器
  int32_t deephook;
  struct __losuvmSTA *stacksta;

  struct main
  {
    const char *funcname;
    const char *funcargs[32];
    struct LosuObj *main;
  } main;

#ifndef __config_losucore_vm_vmbuff
#define __config_losucore_vm_vmbuff 128
#endif
  unsigned char staticbuff[__config_losucore_vm_vmbuff];
  const char *name;

  char **global_symbol;
  int32_t ng_symbol;
  char **global_value;
  int32_t ng_value;

} LosuVm;

typedef int32_t (*LosuApi) (struct LosuVm *vm);

/* LosuObj 16 */
typedef struct LosuObj
{
  uint8_t type; /* type of the object */
  union
  {
    struct _inlineString *str;     /* string type*/
    struct _inlineFunction *func;  /* function type*/
    struct _inlineHash *hash;      /* hash type*/
    struct _inlineCallinfo *call;  /* callinfo type*/
    struct _inlineCoroutine *coro; /* coroutine type*/
    _l_number num;
    _l_int intnum;
    _l_unicode unicode;
    _l_bool boolnum;
    uint8_t _char;
  } value;
} LosuObj;

/* Node 40 */
typedef struct LosuNode
{
  struct LosuObj key;
  struct LosuObj value;
  struct LosuNode *next;
} LosuNode;

/* Package_t */
typedef struct LosuPackage_t
{
  const char *name;
  struct LosuModule_t *(*loadpackage) (struct LosuVm *);
} LosuPackage_t;

typedef struct LosuModule_t
{
  const char *name;
  LosuApi loadmod;
} LosuModule_t;

/* string: 32 */
typedef struct _inlineString
{
  _l_hash hash;               /* Hash Value*/
  int32_t cstidx;             /* Const Index*/
  size_t len;                 /* Length of the string*/
  struct _inlineString *next; /* Next StringObj (in equal-key link list) */
  int16_t marked;             /* Marked for GC*/
  char str[4];                /* Memory space for string value*/
} _inlineString;

/* function 48 */
typedef struct _inlineFunction
{
  /* function segment */
  struct _inlineFunction *next; /* Next FunctionObj (in equal-key link list) */
  struct _inlineFunction *mark; /* Mark for GC, default o.mark == o */
  _l_bool isC;                  /* Is C-API, 1 if is, 0 if not */
  int32_t nclosure;             /* Number of closure upvalue */
  struct LosuObj closure[1];    /* Closure upvalue segment */
  union
  {
    LosuApi capi;              /* C-API */
    struct _inlineScode *sdef; /* Losu Script `def` */
  } func;
  _l_bool isMain;
} _inlineFunction;

/* unit 40 */
typedef struct _inlineHash
{
  _l_bool isMap; // 0 is list, 1 is map
  int32_t size;
  struct LosuNode *node;
  struct LosuNode *free;    /* Free node point */
  struct _inlineHash *next; /* Next HashObj (in equal-key link list) */
  struct _inlineHash
      *mark; /* Mark link when GC working, default o.mark == o */
} _inlineHash;

/* coro 40 */
typedef struct _inlineCoroutine
{
#ifndef __config_losucore_vm_corosize
#define __config_losucore_vm_corosize 64
#endif
  LosuObj stack[__config_losucore_vm_corosize]; /* Stack of this coroutine */
  LosuObj *top;                                 /* Top of the stack */
  /* gc segment */
  uint8_t sta;
  struct _inlineCoroutine *pre, *next; /* equal-key link list */
  _l_bool marked;
} _inlineCoroutine;

/* callinfo  80 */
typedef struct _inlineCallinfo
{
  /* base segment */
  struct _inlineFunction *func; /* Function of this callinfo */
  /* info segment */
  vmInstruction *pc;   /* PC of this callinfo */
  LosuObj *base;       /* Base of this callinfo */
  /*  LosuObj *top; */ /* Top of this callinfo */
  int32_t nres;
  /* recall info */
  struct LosuObj *nextobj; /* next LosuObj to recall */
  /* inline segment */
  void (*inlinecall) (struct LosuVm *vm, LosuObj *func,
                      int32_t nres);  /* Inline call function */
  _l_bool isMalloc;                   /* Is Malloc */
  struct _inlineCallinfo *pre, *next; /* equal-key link list */
  /* gc segment */
  _l_bool marked;
  _l_bool isMain;
} _inlineCallinfo;

/* script-code 128 */
typedef struct _inlineScode
{
  /* Local segment */
  /* Number */
  _l_number *lcnum; /* Number used by this function */
  int32_t nlcnum;   /* Number of lcnum */

  /* String */
  struct _inlineString **lcstr; /* String used by this function */
  int32_t nlcstr;               /* Number of  lcstr  */

  /* Function */
  struct _inlineScode **lcscode; /* Inline-function of this function */
  int32_t nlcscode;              /* Number of lcscode */

  /* Local Var */
  struct _inlineLocalvar *localvar; /* Localvar used by this function */
  int32_t nlocalvar;                /* Number of localvars */

  /* Instruction Segment */
  vmInstruction *code; /* Instruction of this function */
  int32_t ncode;       /* Number of code (MAX == INT_MAX,C-type) */

  /* Arg Segment */
  int8_t narg;    /* Number of arguments */
  _l_bool isVarg; /* is ... */
  int32_t maxstacksize;

  /* Lineinfo Segment */
  int32_t *lineinfo;
  int32_t nlineinfo;
  int32_t defedline;

  /* Data Segment */
  struct _inlineScode *next; /* Next ScodeObj (in equal-key link list) */
  struct _inlineString *src; /* Source code */

  /* Gc Segment */
  _l_bool marked;
  _l_bool isMain;
} _inlineScode;

typedef struct _inlineLocalvar
{
  struct _inlineString *name; /* Name of the local variable */
  int startpc;
  int endpc;
  _l_bool type;
} _inlineLocalvar;

LosuExtern const LosuObj _inlineNullObj;
LosuExtern const LosuObj _inlineSpaceObj;
LosuExtern const LosuObj _inlineTrueObj;
LosuExtern const LosuObj _inlineFalseObj;
LosuExtern _l_size_t LosuVersionNum;
LosuExtern const char *LosuTypeSystem[];
LosuExtern const char LosuVersion[];
LosuExtern const char LosuBuild[];
LosuExtern LosuVm *Ec2WasmVM;
LosuExtern LosuPackage_t *_inlinePackage;
#endif

/* LosuAPI */
#ifndef DEFINE_INCLUDE_LOSU_H_LOSUAPI
#define DEFINE_INCLUDE_LOSU_H_LOSUAPI

/* API: vm_xxx  create, control, and manage VM APIs */
LosuExtern LosuVm *vm_create (int stacksize);
LosuExtern int vm_init (LosuVm *vm, LosuPackage_t *pkgs_t, int32_t argc,
                        const char **argv);
LosuExtern void vm_error (LosuVm *vm, const char *estr, ...);
LosuExtern void vm_stop (LosuVm *vm, const char *estr, ...);
LosuExtern int32_t vm_dofile (LosuVm *vm, const char *filename);
LosuExtern int32_t vm_dostring (LosuVm *vm, const char *str, const char *name);
LosuExtern int32_t vm_dobyte (LosuVm *vm, const char *byte, size_t len,
                              const char *name);
LosuExtern int32_t vm_loadfile (LosuVm *vm, const char *f);
LosuExtern int32_t vm_loadstring (LosuVm *vm, const char *s, const char *name);
LosuExtern int32_t vm_loadbyte (LosuVm *vm, const char *byte, size_t len,
                                const char *name);
LosuExtern int32_t vm_execute (LosuVm *vm, int32_t narg, int32_t nres,
                               const char *name);
LosuExtern LosuObj *vm_getval (LosuVm *vm, const char *name);
LosuExtern void vm_setval (LosuVm *vm, const char *name, LosuObj val);
LosuExtern void vm_close (LosuVm *vm);

/* API: gc_xxx: Manage GC workflow APIs */
LosuExtern void gc_setmax (LosuVm *vm, _l_gcint size);
LosuExtern _l_gcint gc_getmemNow (LosuVm *vm);
LosuExtern _l_gcint gc_getmemMax (LosuVm *vm);
LosuExtern _l_bool gc_collect (LosuVm *vm);

LosuExtern int32_t obj_type (LosuVm *vm, LosuObj *obj);
LosuExtern const char *obj_typeStr (LosuVm *vm, LosuObj *obj);

LosuExtern LosuObj obj_newnull (LosuVm *vm);

LosuExtern LosuObj obj_newnum (LosuVm *vm, _l_number num);
LosuExtern _l_number obj_tonum (LosuVm *vm, LosuObj *obj);

LosuExtern LosuObj obj_newint (LosuVm *vm, _l_int b);
LosuExtern _l_int obj_toint (LosuVm *vm, LosuObj *obj);

LosuExtern LosuObj obj_newunicode (LosuVm *vm, _l_unicode b);
LosuExtern _l_unicode obj_tounicode (LosuVm *vm, LosuObj *obj);

LosuExtern LosuObj obj_newchar (LosuVm *vm, uint8_t b);
LosuExtern uint8_t obj_tochar (LosuVm *vm, LosuObj *obj);

LosuExtern LosuObj obj_newstr (LosuVm *vm, char *str);
LosuExtern const char *obj_tostr (LosuVm *vm, LosuObj *obj);

LosuExtern LosuObj obj_newfunction (LosuVm *vm, LosuApi func);
LosuExtern LosuApi obj_tofunction (LosuVm *vm, LosuObj *obj);

LosuExtern LosuObj obj_newunit (LosuVm *vm, _l_bool isMap);
LosuExtern LosuObj *obj_indexunit (LosuVm *vm, LosuObj unit, LosuObj key);
LosuExtern LosuObj *obj_indexunitbynum (LosuVm *vm, LosuObj unit, _l_number i);
LosuExtern LosuObj *obj_indexunitbystr (LosuVm *vm, LosuObj unit, char *s);
LosuExtern void obj_setunit (LosuVm *vm, LosuObj unit, LosuObj key,
                             LosuObj value);
LosuExtern void obj_setunitbynum (LosuVm *vm, LosuObj unit, _l_number key,
                                  LosuObj value);
LosuExtern void obj_setunitbystr (LosuVm *vm, LosuObj unit, char *key,
                                  LosuObj value);
LosuExtern LosuNode *obj_unit_first (LosuVm *vm, LosuObj unit);
LosuExtern LosuNode obj_unit_location (LosuVm *vm, LosuObj unit, LosuObj key);
LosuExtern LosuNode *obj_unit_next (LosuVm *vm, LosuObj unit, LosuNode *n);
LosuExtern LosuObj obj_unit_nodekey (LosuVm *vm, LosuNode *n);
LosuExtern LosuObj obj_unit_nodevalue (LosuVm *vm, LosuNode *n);

LosuExtern LosuObj obj_newspace (LosuVm *vm);

LosuExtern LosuObj obj_newbool (LosuVm *vm, _l_bool b);
LosuExtern _l_bool obj_tobool (LosuVm *vm, LosuObj *obj);

LosuExtern int32_t arg_num (LosuVm *vm);
LosuExtern LosuObj *arg_get (LosuVm *vm, int idx);
LosuExtern void arg_return (LosuVm *vm, LosuObj val);

LosuExtern void stack_push (LosuVm *vm, LosuObj o);
LosuExtern void stack_pop (LosuVm *vm, int32_t i);
LosuExtern void stack_call (LosuVm *vm, int32_t argnum, int32_t resnum);

uint8_t charset_utf8len (uint8_t c);
#endif

#endif
