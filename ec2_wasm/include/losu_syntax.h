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
  Copyright  2020 - now  chen-chaochen

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
  This is the syntax analysis code for Losu, the complete compiler
  implementation, consisting of Lexer, Parser & Codegen. Parses script code
  written using Losu and converts it to in-memory bytecode form
*/

#ifndef DEFINE_INCLUDE_LOSU_SYNTAX_H
#define DEFINE_INCLUDE_LOSU_SYNTAX_H

#include "losu.h"
#include "losu_bytecode.h"

enum _syntaxExpType
{
  VG, /* global var */
  VL, /*  local var*/
  VI, /* index var */
  VE, /* expression (math.) */
};

typedef union _syntaxTkvalue
{
  _l_number num;
  _inlineString *s;
  _l_int intnum;
  _l_unicode unicode;
  uint8_t _char;
} _syntaxTkvalue;

typedef struct _syntaxToken
{
  int16_t token;
  union _syntaxTkvalue info;
} _syntaxToken;
typedef struct _syntaxExp
{
  uint8_t type;
  union
  {
    int32_t index;
    struct
    {
      int32_t t;
      int32_t f;
    } _bool;
  } value;
} _syntaxExp;

typedef struct _syntaxBreak
{
  struct _syntaxBreak *pre;
  int32_t breaklist;
  int32_t stacklevel;
} _syntaxBreak;

typedef struct _syntaxLex
{
  int16_t current;             /* current tk */
  int16_t deepth;              /* deepth of stack-syntax */
  struct _syntaxToken tk;      /* token now */
  struct _syntaxToken tkahead; /* token ahead */
  struct _syntaxFunc *fs;
  struct LosuVm *vm;
  struct _syntaxIO *io; /* io */
  int32_t linenumber;
  int32_t lastline; /* last line */
  _inlineString *source;
  struct _syntaxLexIdt
  {
    int32_t read;   /* readed */
    int32_t nowidt; /* now idt */
    int32_t size;   /* size of idt */
    int32_t tmp;    /* temp */
  } _syntaxLexIdt;
  _l_bool isMain;
} _syntaxLex;

typedef struct _syntaxFunc
{
  struct _inlineScode *fcode;
  struct _inlineString *source;
  struct _syntaxFunc *prev;
  struct _syntaxLex *lexer;
  struct LosuVm *vm;
  int32_t pc;
  int32_t lasttarget;
  int32_t lastline;
  int32_t jump;
  int16_t stacklevel;

  int32_t aloc[vmOl_MaxLocalvar];
  int16_t naloc;

  struct _syntaxExp closure[vmOl_MaxClosure];
  int16_t nclosure;

  struct _syntaxBreak *breakl;
  int32_t *loopStart;

} _syntaxFunc;
#ifndef __config_losucore_vm_iobuff
#define __config_losucore_vm_iobuff 32
#endif

typedef struct _syntaxIO
{
  _l_size_t size;                           /* n number */
  const unsigned char *p;                   /* p pointer for char */
  int16_t (*fillbuff) (struct _syntaxIO *); /*  */
  void *h;                                  /* f handl=ee_t */
  const char *name;
  uint8_t bin; /* binary or not , if bin. 32 or 64*/
  unsigned char buff[__config_losucore_vm_iobuff];
} _syntaxIO;

#ifndef DEFINE_INCLUDE_LOSU_SYNTAX_H_IO
#define DEFINE_INCLUDE_LOSU_SYNTAX_H_IO

int32_t __losu_syntaxIO_doload (LosuVm *vm, _syntaxIO *io);
int32_t __losu_syntaxIO_loadfile (LosuVm *vm, const char *fn);
int32_t __losu_syntaxIO_loadstring (LosuVm *vm, const char *str, _l_size_t len,
                                    const char *name);
/*
 * Losu supports two IO modes:
 *    1. Streaming IO
 *    2. Handle & buffer IO
 */

/**
 * @brief open a syntaxIO: Handle & buffer IO
 * @param io syntaxIO
 * @param h handle
 * @param name name of handel
 */
void __losu_syntaxIO_openH (_syntaxIO *io, void *h, const char *name);

/**
 * @brief open a syntaxIO: Streaming IO
 * @param io syntaxIO
 * @param p pointer of char-stream
 * @param size size of char-stream
 * @param name name of stream
 */
void __losu_syntaxIO_openS (_syntaxIO *io, const char *p, _l_size_t size,
                            const char *name);

#define __losu_syntaxIO_getc(io)                                              \
  (((io)->size--) > 0 ? ((int16_t)(*((io)->p++))) : ((io)->fillbuff (io)))

#define __losu_syntaxIO_getint32(io)                                          \
  (((io)->size--) > 0 ? ((int32_t)(*((io)->p++))) : ((io)->fillbuff (io)))

#endif /* DEFINE_INCLUDE_LOSU_SYNTAX_H_IO */

#ifndef DEFINE_INCLUDE_LOSU_SYNTAX_H_LEXER
#define DEFINE_INCLUDE_LOSU_SYNTAX_H_LEXER

enum _lexerToken
{
  /* and or not */

  TOKEN_AND = 256, // 且
  TOKEN_NOT,       // 非
  TOKEN_OR,        // 或

  TOKEN_IF,     // 若始
  TOKEN_ELSE,   // 若否
  TOKEN_ELSEIF, // 又若
  TOKEN_ENDIF,  // 若终

  // TOKEN_FUNCTION,
  TOKEN_ARG, /* .. */

  TOKEN_FUNC,    // 函始
  TOKEN_ENDFUNC, // 函终
  TOKEN_SUB,     // 算始
  TOKEN_ENDSUB,  // 算终

  /* var null */
  TOKEN_VAR,   // 令
  TOKEN_NULL,  // 假、未定义
  TOKEN_TRUE,  // 真
  TOKEN_FALSE, // 假
  TOKEN_SPACE, // 空

  /* return break */
  TOKEN_RETURN,   // 返回
  TOKEN_BREAK,    // 跳出
  TOKEN_CONTINUE, // 继续

  /* with  until  loop to*/
  TOKEN_WHILE,    // 当始
  TOKEN_ENDWHILE, // 当终

  TOKEN_DECLARE, // 声明
  TOKEN_IMPORT,  // 导入
  TOKEN_DEFINE,  // 定义
  /* == >= <= !=  &*/
  TOKEN_EQ,   // ==
  TOKEN_GE,   // >=
  TOKEN_LE,   // <=
  TOKEN_NE,   // !=
  TOKEN_DIVI, // & -> '//' 整除
  TOKEN_SET,  // =

  TOKEN_LSH, // <<
  TOKEN_RSH, // >>

  TOKEN_NAME,    // name
  TOKEN_NUMBER,  // number
  TOKEN_INT,     // int
  TOKEN_STRING,  // string
  TOKEN_UNICODE, // unicode
  TOKEN_CHAR,    // char
  TOKEN_BYTES,   // bytes

  TOKEN_EOZ,
};
/* #define TOKEN_END TOKEN_ARG */

void __losu_syntaxLex_init (LosuVm *vm);

#endif /* DEFINE_INCLUDE_LOSU_SYNTAX_H_LEXER */

#ifndef DEFINE_INCLUDE_LOSU_SYNTAX_H_PRSER
#define DEFINE_INCLUDE_LOSU_SYNTAX_H_PRSER

_inlineScode *__losu_syntaxParser_parse (LosuVm *vm, _syntaxIO *io);

#endif /* DEFINE_INCLUDE_LOSU_SYNTAX_H_PRSER */

#ifndef DEFINE_INCLUDE_LOSU_SYNTAX_H_CODEGEN
#define DEFINE_INCLUDE_LOSU_SYNTAX_H_CODEGEN

/* marco function */
#define __losuSyntaxCgenCodearg2 __losuSyntaxCgenCodearg
#define __losuSyntaxCgenCodearg1(fs, o, arg)                                  \
  (__losuSyntaxCgenCodearg (fs, o, arg, 0))
#define __losuSyntaxCgenCodearg0(fs, o) (__losuSyntaxCgenCodearg (fs, o, 0, 0))

#endif /* __inline_define_codegen_h */

#ifndef DEFINE_INCLUDE_LOSU_SYNTAX_H_IRLOAD
#define DEFINE_INCLUDE_LOSU_SYNTAX_H_IRLOAD

_inlineScode *__losu_syntaxIrload_load (LosuVm *vm, _syntaxIO *io);

#endif /* DEFINE_INCLUDE_LOSU_SYNTAX_H_IRLOAD */

typedef int32_t (*_sytaxLocal) (_syntaxLex *lex, _inlineString *n,
                                _syntaxExp *v);

LosuExtern void vm_addgsymbol (LosuVm *vm, const char *name);
LosuExtern void vm_addgvalue (LosuVm *vm, const char *name);
#endif /* DEFINE_INCLUDE_LOSU_SYNTAX_H  */
