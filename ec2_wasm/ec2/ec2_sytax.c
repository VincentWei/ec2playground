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

#ifndef DEFINE_SOURCE_LOSU_SYNTAX_C
#define DEFINE_SOURCE_LOSU_SYNTAX_C

#include "losu_bytecode.h"
#include "losu_errmsg.h"
#include "losu_gc.h"
#include "losu_malloc.h"
#include "losu_object.h"
#include "losu_syntax.h"
#include "losu_vm.h"
#include <ctype.h>
#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef DEFINE_SOURCE_LOSU_SYNTAX_C_STATICDEC
#define DEFINE_SOURCE_LOSU_SYNTAX_C_STATICDEC
/* static segment */

/* IO */
static void __losuSyntaxError (_syntaxLex *lex, const char *msg, ...);

/* Lexer */
static void __losuSyntaxLexSetin (LosuVm *vm, _syntaxLex *lex, _syntaxIO *io,
                                  _inlineString *src);
static int16_t __losuSyntaxLexNext (_syntaxLex *lex, _syntaxTkvalue *tkv);
static void __losuSyntaxLexTtS (int16_t tk, char *s);

/* Parser */
static void __losuSyntaxParError (_syntaxLex *lex, int16_t tk);
static void __losuSyntaxParNewfunc (_syntaxLex *lex, _syntaxFunc *func);
static void __losuSyntaxParDelfunc (_syntaxLex *lex);
static void __losuSyntaxParNext (_syntaxLex *lex);
static _l_bool __losuSyntaxParCheckBlock (int16_t tk);
static void __losuSyntaxParCheck (_syntaxLex *lex, int16_t tk);
static void __losuSyntaxParCheckCondtion (_syntaxLex *lex, _l_bool c,
                                          const char *s);
static void __losuSyntaxParCheckMatch (_syntaxLex *lex, int16_t l, int16_t r,
                                       int32_t line);
static _inlineString *__losuSyntaxParCheckName (_syntaxLex *lex);
static void __losuSyntaxParCheckLimit (_syntaxLex *lex, int32_t v, int32_t l,
                                       const char *errmsg);

static void __losuSyntaxParNewlcvar (_syntaxLex *lex, _inlineString *name,
                                     int16_t i);
static void __losuSyntaxParRemovelcvar (_syntaxLex *lex, int16_t i);
static _l_bool __losuSyntaxParStat (_syntaxLex *lex);
static void __losuSyntaxParStatIf (_syntaxLex *lex, int32_t line);
static void __losuSyntaxParStatWith (_syntaxLex *lex, int32_t line);
static void __losuSyntaxParStatBlock (_syntaxLex *lex);
static void __losuSyntaxParStatFor (_syntaxLex *lex, int32_t line);
static void __losuSyntaxParStatLoop (_syntaxLex *lex, int32_t line);
static void __losuSyntaxParStatUntil (_syntaxLex *lex, int32_t line);
static void __losuSyntaxParStatFunc (_syntaxLex *lex, int32_t line,
                                     _l_bool isMain);
static void __losuSyntaxParStatVar (_syntaxLex *lex);
static void __losuSyntaxParStatName (_syntaxLex *lex, _l_bool onlyDec);
static void __losuSyntaxParStatReturn (_syntaxLex *lex);
static void __losuSyntaxParStatBreak (_syntaxLex *lex);
static void __losuSyntaxParEnterbreak (_syntaxFunc *func, _syntaxBreak *br);
static void __losuSyntaxParLeavebreak (_syntaxFunc *func, _syntaxBreak *br);
static void __losuSyntaxParStatForNum (_syntaxLex *lex, _inlineString *vname);
static void __losuSyntaxParStatForObj (_syntaxLex *lex, _inlineString *vname);
static void __losuSyntaxParStatForBody (_syntaxLex *lex, _l_bool isO);
static _l_bool __losuSyntaxParStatFuncName (_syntaxLex *lex, _syntaxExp *v,
                                            _l_bool isMain);
static void __losuSyntaxParStatFuncBody (_syntaxLex *lex, _l_bool nself,
                                         int32_t line, _l_bool isSub);
static void __losuSyntaxParStatFuncParlist (_syntaxLex *lex, _l_bool isMain);
static int32_t __losuSyntaxParStatNameAssment (_syntaxLex *lex, _syntaxExp *v,
                                               int32_t nvar, _l_bool onlyDec);
static int32_t __losuSyntaxParExplist (_syntaxLex *lex);
static void __losuSyntaxParAdStack (_syntaxLex *lex, int32_t nv, int32_t ne);
static void __losuSyntaxParAdLcvar (_syntaxLex *lex, int32_t nv);
static void __losuSyntaxParVarFunc (_syntaxLex *lex, _syntaxExp *v,
                                    _sytaxLocal getlocal);
static void __losuSyntaxParExp (_syntaxLex *lex);
static void __losuSyntaxParSvar (_syntaxLex *lex, _inlineString *n,
                                 _syntaxExp *v, _sytaxLocal getlocal);
static int32_t __losuSyntaxParStrconst (_syntaxFunc *func, _inlineString *s);
static int32_t __losuSyntaxParSubExp (_syntaxLex *lex, _syntaxExp *v, int l);
static void __losuSyntaxParExpFargs (_syntaxLex *lex, _l_bool slf);
static int32_t __losuSyntaxParExpVarlevel (_syntaxLex *lex, _inlineString *n,
                                           _syntaxExp *v);
static int32_t __losuSyntaxParExpVarlevel_function (_syntaxLex *lex,
                                                    _inlineString *n,
                                                    _syntaxExp *v);
static int32_t __losuSyntaxParExpVarlevel_left (_syntaxLex *lex,
                                                _inlineString *n,
                                                _syntaxExp *v);
static int32_t __losuSyntaxParExpVarlevel_right (_syntaxLex *lex,
                                                 _inlineString *n,
                                                 _syntaxExp *v);
static int32_t __losuSyntaxParExpVarlevel_global (_syntaxLex *lex,
                                                  _inlineString *n,
                                                  _syntaxExp *v);
static void __losuSyntaxParSimpleExp (_syntaxLex *lex, _syntaxExp *v);
static void __losuSyntaxParUnitConstructor (_syntaxLex *lex, char sflag,
                                            char eflag);
static int32_t __losuSyntaxParUnitMapfield (_syntaxLex *lex);
static int32_t __losuSyntaxParUnitListfield (_syntaxLex *lex, char eflag);

/* Codegen */
static int32_t __losuSyntaxCgenCodearg (_syntaxFunc *func, vmIns_OP o,
                                        int32_t arg1, int32_t arg2);
static void __losuSyntaxCgenConcat (_syntaxFunc *func, int32_t *l1,
                                    int32_t l2);
static int32_t __losuSyntaxCgenJump (_syntaxFunc *func);
static void __losuSyntaxCgenPalist (_syntaxFunc *func, int32_t list,
                                    int32_t target);
static int32_t __losuSyntaxCgenGetL (_syntaxFunc *func);
static void __losuSyntaxCgenGoT (_syntaxFunc *func, _syntaxExp *v,
                                 int32_t keep);
static void __losuSyntaxCgenGoF (_syntaxFunc *func, _syntaxExp *v,
                                 int32_t keep);
static void __losuSyntaxCgenTostack (_syntaxLex *lex, _syntaxExp *v,
                                     int32_t o);
static void __losuSyntaxCgenAdStack (_syntaxFunc *func, int32_t n);
static void __losuSyntaxCgenSetVar (_syntaxLex *lex, _syntaxExp *var);
static _l_bool __losuSyntaxCgenIsopen (_syntaxFunc *func);
static void __losuSyntaxCgenSetNcallr (_syntaxFunc *func, int32_t nres);
static void __losuSyntaxCgenNumber (_syntaxFunc *func, _l_number f);
static void __losuSyntaxCgenDtStack (_syntaxFunc *func, int32_t dt);
static void __losuSyntaxCgenPrefix (_syntaxLex *lex, vmIns_UnOp op,
                                    _syntaxExp *v);
static void __losuSyntaxCgenInfix (_syntaxLex *lex, vmIns_BinOp op,
                                   _syntaxExp *v);
static void __losuSyntaxCgenPosfix (_syntaxLex *lex, vmIns_BinOp op,
                                    _syntaxExp *v1, _syntaxExp *v2);
static int32_t __losuSyntaxCgenGetJump (_syntaxFunc *func, int32_t pc);
static void __losuSyntaxCgenFixJump (_syntaxFunc *func, int32_t pc,
                                     int32_t dt);
static void __losuSyntaxCgenPlistfunc (_syntaxFunc *func, int32_t list,
                                       int32_t tg, vmIns_OP ins,
                                       int32_t instg);
static void __losuSyntaxCgenGo (_syntaxFunc *func, _syntaxExp *v, int32_t inv,
                                vmIns_OP jump);
static _l_bool __losuSyntaxCgenDischarge (_syntaxFunc *func, _syntaxExp *v);
static _l_bool __losuSyntaxCgenNeedval (_syntaxFunc *func, int32_t l,
                                        vmIns_OP v);
static vmIns_OP __losuSyntaxCgenJumpInvert (vmIns_OP op);

#endif /* DEFINE_SOURCE_LOSU_SYNTAX_C_STATICDEC */

#ifndef DEFINE_SOURCE_LOSU_SYNTAX_C_IO
#define DEFINE_SOURCE_LOSU_SYNTAX_C_IO

#define EOZ (-1)

int32_t
__losu_syntaxIO_doload (LosuVm *vm, _syntaxIO *io)
{
  /*   _l_gcint oldblock; */
  int32_t sta = LosuErrorCode_Ok;
  __losu_gc_checkClt (vm);

  LosuObj *oldbase = vm->base;
  LosuObj *oldtop = vm->top;
  __losuvmJmp lj = {
    .func = NULL,
  },*oldjmp = vm->errjmp;
  vm->errjmp = &lj;
  if (setjmp (lj.jmpflag) == 0)
    {
      _inlineScode *fcd = io->bin ? __losu_syntaxIrload_load (vm, io)
                                  : __losu_syntaxParser_parse (vm, io);
      _inlineFunction *f = __losu_objFunc_new (vm, 0);
      ovtype ((vm->top)) = LosuTypeDefine_function;
      ovfunc ((vm->top)) = f;
      vm->top++;
      f->func.sdef = fcd;
      f->isC = 0;
    }
  else
    {
      vm->base = oldbase;
      vm->top = oldtop;
      sta = LosuErrorCode_Syntax;
    }
  vm->errjmp = oldjmp;

  return sta;
}

int32_t
__losu_syntaxIO_loadfile (LosuVm *vm, const char *fn)
{
  _syntaxIO io = { 0 };
  int32_t sta;
  FILE *f = fopen (fn, "rb");
  if (f == NULL)
    {
      fprintf (stderr, __config_losucore_errmsg_msgfileNotFound, fn);
      return LosuErrorCode_File;
    }
  __losu_syntaxIO_openH (&io, (void *)f, fn);

  /* bin or txt */
  int32_t c = fgetc (f);
  if (c == 0) /* bin */
    io.bin = 1;
  ungetc (c, f);

  const char *on = vm->name;
  vm->name = fn;
  sta = __losu_syntaxIO_doload (vm, &io);
  vm->name = on;
  fclose (f);
  return sta;
}

int32_t
__losu_syntaxIO_loadstring (LosuVm *vm, const char *str, _l_size_t len,
                            const char *name)
{
  _syntaxIO io = { 0 };
  __losu_syntaxIO_openS (&io, str, len, name);

  /* bin or txt */
  if (io.p[0] == 0 && len > 0)
    io.bin = 1;

  const char *on = vm->name;
  vm->name = name;
  int32_t sta = __losu_syntaxIO_doload (vm, &io);
  vm->name = on;
  return sta;
}

int16_t
__losu_syntaxIOS_fillbuff (_syntaxIO *io)
{
  return EOZ;
}
int16_t
__losu_syntaxIOH_fillbuff (_syntaxIO *io)
{
  _l_size_t n = 0;
  if (feof ((FILE *)(io->h)))
    return EOZ;
  n = fread (io->buff, sizeof (char), __config_losucore_vm_iobuff,
             (FILE *)(io->h));
  if (!n)
    return EOZ;
  io->size = n - 1;
  io->p = io->buff;
  return (int16_t)(*(io->p++));
}

void
__losu_syntaxIO_openH (_syntaxIO *io, void *h, const char *name)
{
  if (h)
    {
      io->size = 0;
      io->p = io->buff;
      io->fillbuff = __losu_syntaxIOH_fillbuff;
      io->h = h;
      io->name = name;
    }
}

void
__losu_syntaxIO_openS (_syntaxIO *io, const char *p, _l_size_t size,
                       const char *name)
{
  io->size = (p == NULL) ? 0 : size;
  io->p = (const unsigned char *)p;
  io->fillbuff = __losu_syntaxIOS_fillbuff;
  io->name = name;
}

static void
__losuSyntaxError (_syntaxLex *lex, const char *msg, ...)
{
  memset (lex->vm->staticbuff, 0, sizeof (lex->vm->staticbuff));
  va_list ap;
  va_start (ap, msg);
  vsprintf ((char *)(lex->vm->staticbuff), msg, ap);
  fprintf (stderr, "语法错误: %s\n 在第 %d 行\n", lex->vm->staticbuff,
           lex->linenumber);
  va_end (ap);
  __losu_sigThrow (lex->vm, VMSIGERR);
}

#undef EOZ
#endif /* DEFINE_SOURCE_LOSU_SYNTAX_C_IO */

#ifndef DEFINE_SOURCE_LOSU_SYNTAX_C_LEXER
#define DEFINE_SOURCE_LOSU_SYNTAX_C_LEXER

/* declare segment */
#define EOZ -1
#define next(Lex) (Lex->current = __losu_syntaxIO_getc (Lex->io))
#define save(vm, c, l) (vm->bufftmp[l++] = (unsigned char)c)
#define save_next(vm, Lex, l) (save (vm, Lex->current, l), next (Lex))
#define incline(Lex) ((Lex->linenumber)++)
static void checkbuffer (LosuVm *vm, _l_size_t n, _l_size_t len);

/* data segment */
struct
{
  const char *str;
  int16_t key;
} static const __keyword[] = {
  { "且", TOKEN_AND },        { "或", TOKEN_OR },
  { "非", TOKEN_NOT },

  { "若始", TOKEN_IF },       { "若否", TOKEN_ELSE },
  { "若另", TOKEN_ELSEIF },   { "若终", TOKEN_ENDIF },

  { "函始", TOKEN_FUNC },     { "...", TOKEN_ARG },
  { "返回", TOKEN_RETURN },   { "函终", TOKEN_ENDFUNC },

  { "算始", TOKEN_SUB },      { "算终", TOKEN_ENDSUB },

  { "全局", TOKEN_VAR },      { "未定义", TOKEN_NULL },
  { "假", TOKEN_FALSE },      { "真", TOKEN_TRUE },
  { "空", TOKEN_SPACE },

  { "当始", TOKEN_WHILE },    { "跳出", TOKEN_BREAK },
  { "继续", TOKEN_CONTINUE }, { "当终", TOKEN_ENDWHILE },

  { "声明", TOKEN_DECLARE },  { "导入", TOKEN_IMPORT },
  { "定义", TOKEN_DEFINE },

};

/* code segment */
static void
checkbuffer (LosuVm *vm, _l_size_t n, _l_size_t len)
{
  _l_size_t nlen = n + len;
  if (nlen <= vm->nbufftmp)
    return;
  __losu_mem_reallocvector (vm, vm->bufftmp, nlen, unsigned char);
  vm->nblocks += (nlen - vm->nbufftmp) * sizeof (char);
  vm->nbufftmp = nlen;
}

void
__losu_syntaxLex_init (LosuVm *vm)
{
  for (int16_t i = 0; i < sizeof (__keyword) / sizeof (__keyword[0]); i++)
    {
      _inlineString *s = __losu_objString_new (vm, __keyword[i].str);
      s->marked = __keyword[i].key;
    }
}

static void
__losuSyntaxLexTtS (int16_t tk, char *s)
{
  if (tk < 256)
    {
      s[0] = (unsigned char)tk;
      s[1] = '\0';
    }
  else
    {
      int16_t i;
      for (i = 0; i < sizeof (__keyword) / sizeof (__keyword[0]); i++)
        if (__keyword[i].key == tk)
          break;
      strcpy (s, __keyword[i].str);
    }
}

static void
__losuSyntaxLexSetin (LosuVm *vm, _syntaxLex *lex, _syntaxIO *io,
                      _inlineString *src)
{
  lex->vm = vm;
  lex->tkahead.token = TOKEN_EOZ;
  lex->io = io;
  lex->fs = NULL;
  lex->lastline = lex->linenumber = 1;
  lex->source = src;
  next (lex);
}

static int32_t
hex2int (char c)
{
  // hex char to int
  if ('0' <= c && c <= '9')
    return c - '0';
  if ('a' <= c && c <= 'f')
    return c - 'a' + 10;
  if ('A' <= c && c <= 'F')
    return c - 'A' + 10;
  return 0;
}
static int32_t
bin2int (char c)
{
  return c - '0';
}
static int32_t
oct2int (char c)
{
  return c - '0';
}

static uint8_t
__utf8CheckLen (uint8_t c)
{
  /** 单字节序列的 leader byte 总是在（0-127）范围内。
   * 两字节序列的 leader byte 在（194-223）范围内。
   * 三字节序列的 leader byte 在（224-239）范围内。
   * 四字节序列的 leader byte 在（240-247）范围内。
   * */
  uint8_t len = 0;
  if (c < 0x80)
    { /* then UTF-8   单字节 */
      len = 1;
    }
  else if (c >= 0xC2 && c <= 0xDF)
    { /* then 首字节   UTF-8 占用2个字节 */
      len = 2;
    }
  else if (c >= 0xE0 && c <= 0xEF)
    { /* then 首字节   UTF-8 占用3个字节 */
      len = 3;
    }
  else if (c >= 0xF0 && c <= 0xF7)
    { /* then 首字节   UTF-8 占用4个字节 */
      len = 4;
    }

  return len;
}

static unsigned char *
__lexReadname (_syntaxLex *lex)
{
  LosuVm *vm = lex->vm;
  _l_size_t l = 0;
  do
    {
      uint8_t len = __utf8CheckLen ((uint8_t)(lex->current));
      for (uint8_t i = 0; i < len; i++)
        {
          checkbuffer (vm, l, 32);
          save_next (vm, lex, l);
        }
    }
  while (isalnum (lex->current) || lex->current == '_'
         || __utf8CheckLen ((uint8_t)(lex->current)) > 1);
  save (vm, '\0', l);
  return vm->bufftmp;
}

static _l_bool
__lexReadnumber (_syntaxLex *lex, _l_bool dot, _syntaxTkvalue *tkv)
{
  _l_bool isF = 0;
  LosuVm *vm = lex->vm;
  _l_size_t l = 0;
  checkbuffer (vm, l, 32);
  if (dot)
    save (vm, '.', l);
  while (isdigit (lex->current))
    {
      checkbuffer (vm, l, 32);
      save_next (vm, lex, l);
    }
  if (lex->current == '.')
    {
      isF = 1;
      save_next (vm, lex, l);
      if (lex->current == '.')
        {
          save (vm, '\0', l);
          __losuSyntaxError (lex, __config_losucore_errmsg_msgInvalidNumber,
                             vm->bufftmp);
        }
    }
  while (isdigit (lex->current))
    {
      checkbuffer (vm, l, 32);
      save_next (vm, lex, l);
    }
  if (lex->current == 'e' || lex->current == 'E')
    {
      save_next (vm, lex, l);
      if (lex->current == '+' || lex->current == '-')
        save_next (vm, lex, l);
      while (isdigit (lex->current))
        {
          checkbuffer (vm, l, 32);
          save_next (vm, lex, l);
        }
    }
  save (vm, '\0', l);
  if (!__losu_object_str2num ((const char *)vm->bufftmp, &tkv->num))
    __losuSyntaxError (lex, __config_losucore_errmsg_msgInvalidNumber,
                       vm->bufftmp);
  return isF;
}

static _l_bool
__lexReadstring (_syntaxLex *lex, uint8_t del, _syntaxTkvalue *tkv)
{
  LosuVm *vm = lex->vm;
  _l_size_t l = 0;
  checkbuffer (vm, l, 32);
  save_next (vm, lex, l);
  while (lex->current != del)
    {
      if (lex->current == '\n' || lex->current == EOZ)
        __losuSyntaxError (lex, __config_losucore_errmsg_msgMissStrEnd, del);
      checkbuffer (vm, l, 32);
      if (lex->current == '\\')
        {
          next (lex);
          switch (lex->current)
            {
            case '0':
              {
                save (vm, '\0', l);
                next (lex);
              }
            case '"':
              {
                save (vm, '\"', l);
                next (lex);
                break;
              }
            case '\\':
              {
                save (vm, '\\', l);
                next (lex);
                break;
              }
            case '/':
              {
                save (vm, '/', l);
                next (lex);
                break;
              }
            case 'b':
              {
                save (vm, '\b', l);
                next (lex);
                break;
              }
            case 'f':
              {
                save (vm, '\f', l);
                next (lex);
                break;
              }
            case 'n':
              {
                save (vm, '\n', l);
                next (lex);
                break;
              }
            case 'r':
              {
                save (vm, '\r', l);
                next (lex);
                break;
              }
            case 't':
              {
                save (vm, '\t', l);
                next (lex);
                break;
              }
            case 'u':
              {
                // Handle Unicode escape sequence \uXXXX
                next (lex);
                _l_unicode uni = 0;
                for (int i = 0; i < 8; i++)
                  {
                    if (!isxdigit (lex->current))
                      break;
                    uni = (uni << 4) | (uint8_t)hex2int (lex->current);
                    next (lex);
                  }
                const char *us
                    = obj_tostr (vm, &(LosuObj){
                                         .type = LosuTypeDefine_unicode,
                                         .value.unicode = uni,

                                     });
                for (int i = 0; us[i] != '\0'; i++)
                  save (vm, us[i], l);
                break;
              }
            default:
              save_next (vm, lex, l);
              break;
            }
          // Move to the next character after the escaped character
          continue;
        }
      save_next (vm, lex, l);
    }
  save_next (vm, lex, l);
  save (vm, '\0', l);
  tkv->s
      = __losu_objString_newstr (vm, (const char *)(vm->bufftmp) + 1, l - 3);
  return 0;
}

#include <emscripten.h>
static int16_t
__losuSyntaxLexNext (_syntaxLex *lex, _syntaxTkvalue *tkv)
{
lex_start:
  while (1)
    {
      switch (lex->current)
        {
        /* comment */
        case '#':
          {
            while (lex->current != '\n')
              {
                next (lex);
                if (lex->current == EOZ)
                  return TOKEN_EOZ;
              }
            break;
          }

        case EOZ:
          {
            return TOKEN_EOZ;
          }

        /* next */
        case ';':
        case '\r':
        case '\t':
        case ' ':
        case '\0':
          {
            next (lex);
            continue;
          }
        case '\n':
          {
            incline (lex);
            next (lex);
            continue;
          }
        /* string */
        case '\"':
          {
            __lexReadstring (lex, lex->current, tkv);
            return TOKEN_STRING;
          }

        /* < = > */
        case '<':
          {
            next (lex);
            if (lex->current == '=')
              {
                next (lex);
                return TOKEN_LE;
              }
            else if (lex->current == '<')
              {
                next (lex);
                return TOKEN_LSH;
              }
            else
              return '<';
          }
        case '=':
          {
            next (lex);
            if (lex->current != '=')
              return TOKEN_SET;
            else
              {
                next (lex);
                return TOKEN_EQ;
              }
          }
        case '>':
          {
            next (lex);
            if (lex->current == '=')
              {
                next (lex);
                return TOKEN_GE;
              }
            else if (lex->current == '>')
              {
                next (lex);
                return TOKEN_RSH;
              }
            else
              return '>';
          }

        /* & or && */
        case '/':
          {
            next (lex);
            if (lex->current == '/')
              {
                next (lex);
                return TOKEN_DIVI;
              }
            else
              return '/';
          }

        /* != or ! */
        case '!':
          {
            next (lex);
            if (lex->current == '=')
              {
                next (lex);
                return TOKEN_NE;
              }
            else
              return TOKEN_NOT;
          }
        case '.':
          {
            next (lex);
            if (lex->current == '.')
              {
                next (lex);
                if (lex->current == '.')
                  {
                    next (lex);
                    return TOKEN_ARG;
                  }
                else
                  __losuSyntaxError (
                      lex, __config_losucore_errmsg_msgUnknownSymbol, "..");
              }
            if (isdigit (lex->current))
              {
                __lexReadnumber (lex, 1, tkv);
                return TOKEN_NUMBER;
              }
            else
              return '.';
          }

        case '\'':
          {
            next (lex);
            if (lex->current != '\'')
              {
                tkv->_char = lex->current;
                next (lex);
                if (lex->current != '\'')
                  __losuSyntaxError (lex, "错误的字节格式");
                next (lex);
                return TOKEN_CHAR;
              }
            else
              {
                next (lex);
                int32_t len = __utf8CheckLen ((uint8_t)lex->current);
                _l_unicode i = 0;
                for (int j = 0; j < len; j++)
                  {
                    i = (i << 8) + (uint8_t)lex->current;
                    next (lex);
                  }
                if (lex->current != '\'')
                  __losuSyntaxError (lex, "字符缺少 `'`", lex->current);
                else
                  {
                    next (lex);
                    if (lex->current != '\'')
                      __losuSyntaxError (lex, "字符缺少 `'`", lex->current);
                    next (lex);
                  }
                tkv->unicode = i;
                return TOKEN_UNICODE;
              }
          }
        case ':':
          {
            next (lex);
            if (lex->current == '=')
              {
                next (lex);
                return '=';
              }
            return ':';
          }

        case '0':
          {
            next (lex);
            if (lex->current == 'x')
              { // hex 16
                next (lex);
                _l_int intnum = 0;
                for (int32_t i = 0; i < 8; i++)
                  {
                    if (!isxdigit (lex->current))
                      break;
                    intnum = intnum << 4 | hex2int (lex->current);
                    next (lex);
                  }
                tkv->intnum = intnum;
                return TOKEN_INT;
              }
            else if (lex->current == 'b')
              { // binary 2
                next (lex);
                _l_int intnum = 0;
                for (int32_t i = 0; i < 32; i++)
                  {
                    if (lex->current != '0' && lex->current != '1')
                      break;
                    intnum = intnum << 1 | bin2int (lex->current);
                    next (lex);
                  }
                tkv->intnum = intnum;
                return TOKEN_INT;
              }
            else if (isdigit (lex->current))
              { // octal 8
                _l_int intnum = 0;
                for (int32_t i = 0; i < 10; i++)
                  {
                    if (!(lex->current >= '0' && lex->current < '8'))
                      break;
                    intnum = intnum << 3 | oct2int (lex->current);
                    next (lex);
                  }
                tkv->intnum = intnum;
                return TOKEN_INT;
              }
            else if (lex->current == '.')
              { // float
                next (lex);
                __lexReadnumber (lex, 1, tkv);
                return TOKEN_NUMBER;
              }
            else
              tkv->intnum = 0;
            return TOKEN_INT;
            break;
          }
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
          {
            _l_bool isF = __lexReadnumber (lex, 0, tkv);
            _l_int t = (_l_int)tkv->num;
            if (t == tkv->num && isF == 0)
              {
                tkv->intnum = t;
                return TOKEN_INT;
              }
            return TOKEN_NUMBER;
          }

        case '@':
          {
            // printf ("@\n");
            next (lex);
            char *tmp = (char *)malloc (1024);
            size_t l = 0;
            memset ((void *)tmp, 0, 1024);
            switch (lex->current)
              {
              case 'a':
                {
                  next (lex);
                  while (l < 1024)
                    {
                      if (lex->current == '\n' || lex->current == ' '
                          || lex->current == EOZ)
                        break;
                      else if (lex->current == '\\')
                        {
                          // \xAB[两位16进制]
                          next (lex); // lex->current == 'x'
                          next (lex); // lex->current == 'A'
                          uint8_t c = 0;
                          if (!isxdigit (lex->current))
                            __losuSyntaxError (lex, "错误的转义字符");
                          c = hex2int (lex->current);
                          next (lex); // lex->current == 'B'
                          if (!isxdigit (lex->current))
                            __losuSyntaxError (lex, "错误的转义字符");
                          c = (c << 4) | hex2int (lex->current);
                          tmp[l++] = c;
                        }
                      else
                        {
                          tmp[l++] = lex->current;
                        }
                      next (lex);
                    }
                  tkv->s = __losu_objString_newstr (lex->vm, tmp, l);
                  break;
                }
              case 'b':
                {
                  next (lex);
                  while (l < 1024)
                    {
                      uint8_t c = 0;
                      for (int i = 0; i < 8; i++)
                        {
                          if (lex->current == '\n' || lex->current == ' '
                              || lex->current == EOZ)
                            break;
                          else if (lex->current == '\'')
                            {
                              next (lex);
                              i--;
                            }
                          else
                            {
                              if (lex->current != '0' && lex->current != '1')
                                __losuSyntaxError (lex, "错误的字节串格式");
                              c = c << 1 | (uint8_t)(lex->current - '0');
                              next (lex);
                            }
                        }
                      tmp[l++] = c;
                      if (lex->current == '\n' || lex->current == ' '
                          || lex->current == EOZ)
                        break;
                    }
                  tkv->s = __losu_objString_newstr (lex->vm, tmp, l);
                  break;
                }
              case 'x':
                {
                  next (lex);
                  while (l < 1024)
                    {
                      if (lex->current == '\n' || lex->current == ' '
                          || lex->current == EOZ)
                        break;
                      // AB 两位16进制
                      uint8_t c = 0;
                      if (!isxdigit (lex->current))
                        __losuSyntaxError (lex, "错误的字节串格式");
                      c = hex2int (lex->current);
                      next (lex);
                      if (!isxdigit (lex->current))
                        __losuSyntaxError (lex, "错误的字节串格式");
                      c = (c << 4) | hex2int (lex->current);
                      tmp[l++] = c;
                      next (lex);
                    }
                  tkv->s = __losu_objString_newstr (lex->vm, tmp, l);
                  break;
                }
              case 's':
                {
                  next (lex);
                  while (l < 1024)
                    {
                      if (lex->current == '\n' || lex->current == ' '
                          || lex->current == EOZ)
                        break;
                      else
                        tmp[l++] = lex->current;
                      next (lex);
                    }
                  char *s = malloc (2048);
                  sprintf (s, "atob('%s')", tmp);
                  char *s2 = emscripten_run_script_string (s);
                  // printf("@%p\n",s2);
                  tkv->s = __losu_objString_newstr (lex->vm, s2, strlen (s2));
                  free (s);
                  break;
                }
              default:
                __losuSyntaxError (lex, "错误的字节串格式");
                break;
              }
            free (tmp);
            return TOKEN_BYTES;
            break;
          }

        default:
          {
            if (!isalpha (lex->current) && lex->current != '_'
                && lex->current < 0x80)
              {
                int16_t c = lex->current;
                next (lex);
                return c;
              }
            _inlineString *ts = __losu_objString_new (
                lex->vm, (const char *)__lexReadname (lex));

            if (ts->marked > 255)
              {
                if (ts->marked == TOKEN_DECLARE || ts->marked == TOKEN_IMPORT
                    || ts->marked == TOKEN_DEFINE)
                  {
                    while (lex->current != '\n')
                      {
                        next (lex);
                        if (lex->current == EOZ)
                          return TOKEN_EOZ;
                      }
                    goto lex_start;
                  }
                return ts->marked;
              }

            tkv->s = ts;
            return TOKEN_NAME;
          }
        }
    }
}

/* undef segment */
#undef EOZ
#undef next
#undef save
#undef save_next
#undef incline

#endif /* DEFINE_SOURCE_LOSU_SYNTAX_C_LEXER */

#ifndef DEFINE_SOURCE_LOSU_SYNTAX_C_PARSER
#define DEFINE_SOURCE_LOSU_SYNTAX_C_PARSER

/* declare sgement */
#define NO_JUMP -1
#define __losuSyntaxParBlock(lex)                                             \
  {                                                                           \
    __losuSyntaxParStatBlock (lex);                                           \
  }

/* code segment */

_inlineScode *
__losu_syntaxParser_parse (LosuVm *vm, _syntaxIO *io)
{
  _syntaxLex lex = { ._syntaxLexIdt = {
                         .nowidt = 0,
                         .read = 0,
                         .size = 0,
                         .tmp = 0,
                     } ,
                     .isMain = 0, };
  _syntaxFunc func;

  lex.deepth = 0;
  _l_bool islast = 0;

  __losuSyntaxLexSetin (vm, &lex, io, __losu_objString_new (vm, io->name));
  __losuSyntaxParNewfunc (&lex, &func);
  __losuSyntaxParNext (&lex);
  while (!islast && !__losuSyntaxParCheckBlock (lex.tk.token))
    islast = __losuSyntaxParStat ((_syntaxLex *)(&lex));

  __losuSyntaxParCheckCondtion (&lex, lex.tk.token == TOKEN_EOZ,
                                "错误的程序结构");
  //   __losuSyntaxParCheckCondtion (&lex, (lex.tk.token != TOKEN_ELSE),
  //                                 __config_losucore_errmsg_msgInvalidElse);
  //   __losuSyntaxParCheckCondtion (&lex, (lex.tk.token == TOKEN_EOZ),
  //                                 __config_losucore_errmsg_msgMissingEOZ);
  __losuSyntaxParDelfunc (&lex);
  return func.fcode;
}

static void
__losuSyntaxParError (_syntaxLex *lex, int16_t tk)
{
  char t[8];
  __losuSyntaxLexTtS (tk, t);
  __losuSyntaxError (lex, __config_losucore_errmsg_msgParserError, t);
}

static void
__losuSyntaxParNewfunc (_syntaxLex *lex, _syntaxFunc *func)
{
  _inlineScode *f = (_inlineScode *)__losu_objFunc_scodeNew (lex->vm);
  func->prev = lex->fs;
  func->lexer = lex;
  func->vm = lex->vm;
  func->stacklevel = 0;
  func->naloc = 0;
  func->nclosure = 0;
  func->breakl = NULL;
  func->fcode = f;
  func->pc = 0;
  func->lasttarget = 0;
  func->lastline = 0;
  func->jump = NO_JUMP;

  lex->fs = func;

  /* *f = (_inlineScode){ */
  f->src = lex->source;
  f->code = NULL;
  f->maxstacksize = 0;
  f->narg = 0;
  f->isVarg = 0;
  f->marked = 0;
  /* }; */
}

static void
__losuSyntaxParDelfunc (_syntaxLex *lex)
{
  LosuVm *vm = lex->vm;
  _syntaxFunc *func = lex->fs;
  _inlineScode *f = func->fcode;
  __losuSyntaxCgenCodearg0 (func, INS_END);
  __losuSyntaxCgenGetL (func);
  __losuSyntaxParRemovelcvar (lex, func->naloc);
  __losu_mem_reallocvector (vm, f->localvar, f->nlocalvar, _inlineLocalvar);
  __losu_mem_reallocvector (vm, f->code, func->pc, vmInstruction);
  __losu_mem_reallocvector (vm, f->lcstr, f->nlcstr, _inlineString *);
  __losu_mem_reallocvector (vm, f->lcnum, f->nlcnum, _l_number);
  __losu_mem_reallocvector (vm, f->lcscode, f->nlcscode, _inlineScode *);
  __losu_mem_reallocvector (vm, f->lineinfo, f->nlineinfo + 1, int32_t);
  f->lineinfo[f->nlineinfo++] = INT32_MAX;

#define codesize(f)                                                           \
  (sizeof (_inlineScode) + f->nlcnum * sizeof (_l_number)                     \
   + f->nlcstr * sizeof (_inlineString *)                                     \
   + f->nlcscode * sizeof (_inlineScode *)                                    \
   + f->ncode * sizeof (vmInstruction)                                        \
   + f->nlocalvar * sizeof (_inlineLocalvar)                                  \
   + f->nlineinfo * sizeof (int32_t))
  f->ncode = func->pc;
  vm->nblocks += codesize (f);
#undef codesize

  lex->fs = func->prev;
}

static void
__losuSyntaxParCheck (_syntaxLex *lex, int16_t tk)
{
  if (lex->tk.token != tk)
    __losuSyntaxParError (lex, tk);
  __losuSyntaxParNext (lex);
}

static _l_bool
__losuSyntaxParCheckBlock (int16_t tk)
{
  switch (tk)
    {
    case TOKEN_ELSE:
    case TOKEN_ELSEIF:
    // endxxx
    case TOKEN_ENDIF:
    case TOKEN_ENDFUNC:
    case TOKEN_ENDSUB:
    case TOKEN_ENDWHILE:
    // eoz
    case TOKEN_EOZ:
      return 1;
    default:
      return 0;
    }
}

static void
__losuSyntaxParCheckCondtion (_syntaxLex *lex, _l_bool c, const char *s)
{
  if (!c)
    __losuSyntaxError (lex, s);
}

static void
__losuSyntaxParCheckMatch (_syntaxLex *lex, int16_t l, int16_t r, int32_t line)
{
  if (lex->tk.token != r)
    {
      char tl[8], tr[8];
      __losuSyntaxLexTtS (l, tl);
      __losuSyntaxLexTtS (r, tr);
      if (l == TOKEN_IF)
        __losuSyntaxError (
            lex, __config_losucore_errmsg_msgCheckMatch " 或 '若另' ", tl,
            line, tr);
      else
        __losuSyntaxError (lex, __config_losucore_errmsg_msgCheckMatch, tl,
                           line, tr);
    }
  __losuSyntaxParNext (lex);
}

static _inlineString *
__losuSyntaxParCheckName (_syntaxLex *lex)
{
  _inlineString *ts;
  __losuSyntaxParCheckCondtion (lex, lex->tk.token == TOKEN_NAME,
                                __config_losucore_errmsg_msgCheckName);
  ts = lex->tk.info.s;
  __losuSyntaxParNext (lex);
  return ts;
}

static void
__losuSyntaxParCheckLimit (_syntaxLex *lex, int32_t v, int32_t l,
                           const char *errmsg)
{
  if (v <= l)
    return;
  /*   sprintf ((char *)lex->vm->staticbuff,
    __config_losucore_errmsg_msgCheckLimit, errmsg, l);
    __losuSyntaxError (lex, (const char *)lex->vm->staticbuff); */
  __losuSyntaxError (lex, __config_losucore_errmsg_msgCheckLimit, errmsg, l);
}

static void
__losuSyntaxParNewlcvar (_syntaxLex *lex, _inlineString *name, int16_t i)
{
  _syntaxFunc *func = lex->fs;
  _inlineScode *f = func->fcode;

  /* check limit? */
  __losuSyntaxParCheckLimit (lex, func->naloc + i + 1, vmOl_MaxLocalvar,
                             __config_losucore_errmsg_msgCheckLimitTMlocvar);

  __losu_mem_growvector (lex->vm, f->localvar, f->nlocalvar, 1,
                         _inlineLocalvar, vmIns_MaxA,
                         __config_losucore_errmsg_msgVectorOverflow);
  f->localvar[f->nlocalvar].name = name;
  f->localvar[f->nlocalvar].type = VL;
  func->aloc[func->naloc + i] = f->nlocalvar++;
}

static void
__losuSyntaxParRemovelcvar (_syntaxLex *lex, int16_t i)
{
  _syntaxFunc *func = lex->fs;
  while (i--)
    func->fcode->localvar[func->aloc[--(func->naloc)]].endpc = func->pc;
}

static void
__losuSyntaxParNext (_syntaxLex *lex)
{

  lex->lastline = lex->linenumber;
  if (lex->tkahead.token != TOKEN_EOZ)
    {
      lex->tk = lex->tkahead;
      lex->tkahead.token = TOKEN_EOZ;
    }
  else
    lex->tk.token
        = __losuSyntaxLexNext (lex, (_syntaxTkvalue *)(&(lex->tk.info)));
  /* printf ("tk:%d\n", lex->tk.token);
  if (lex->tk.token < 128)
    printf ("\tvalue:%c\n", lex->tk.token); */
}

static _l_bool
__losuSyntaxParStat (_syntaxLex *lex)
{
  int32_t line = lex->linenumber;
  /* printf ("TK:%d\n", lex->tk.token); */
  switch (lex->tk.token)
    {
    case TOKEN_IF:
      {
        lex->deepth++;
        __losuSyntaxParStatIf (lex, line);
        lex->deepth--;
        return 0;
      }
    case TOKEN_WHILE:
      {
        lex->deepth++;
        __losuSyntaxParStatWith (lex, line);
        lex->deepth--;
        return 0;
      }
    case TOKEN_FUNC:
      {
        if (lex->deepth)
          __losuSyntaxError (lex, "当前位置不允许定义函数");
        lex->deepth++;
        __losuSyntaxParStatFunc (lex, line, 0);
        lex->deepth--;
        return 0;
      }
    case TOKEN_SUB:
      {
        if (lex->isMain)
          __losuSyntaxError (lex, "重复定义的算法");
        if (lex->deepth)
          __losuSyntaxError (lex, "当前位置不允许定义算法");
        lex->deepth++;
        lex->isMain = 1;
        __losuSyntaxParStatFunc (lex, line, 1);
        lex->deepth--;
        return 0;
      }
    case TOKEN_VAR:
      {
        __losuSyntaxParStatVar (lex);
        return 0;
      }
    case TOKEN_NAME:
      {
        __losuSyntaxParStatName (lex, lex->linenumber);
        return 0;
      }
    case TOKEN_RETURN:
      {
        __losuSyntaxParStatReturn (lex);
        return 1;
      }
    case TOKEN_BREAK:
      {
        __losuSyntaxParStatBreak (lex);
        return 1;
      }
    case TOKEN_CONTINUE:
      {
        __losuSyntaxParNext (lex);
        if (lex->fs->loopStart)
          __losuSyntaxCgenCodearg2 (
              lex->fs, INS_JMP, *(lex->fs->loopStart) - lex->fs->pc - 1, 0);
        return 0;
      }
    default:
      {
        __losuSyntaxError (lex, __config_losucore_errmsg_msgInvalidExp);
        return 0;
      }
    }
}
static void
__losuSyntaxParStatIf (_syntaxLex *lex, int32_t line)
{

#define condition(lex, v)                                                     \
  {                                                                           \
    __losuSyntaxParSubExp ((lex), (v), -1);                                   \
    __losuSyntaxCgenGoT ((lex)->fs, (v), 0);                                  \
  }

#define then(lex, v)                                                          \
  {                                                                           \
    __losuSyntaxParNext ((lex));                                              \
    condition ((lex), (v));                                                   \
    __losuSyntaxParBlock ((lex));                                             \
  }

  _syntaxFunc *func = lex->fs;
  _syntaxExp v;
  int32_t el = NO_JUMP;
  then (lex, &v);
  while (lex->tk.token == TOKEN_ELSEIF)
    {
      __losuSyntaxCgenConcat (func, &el, __losuSyntaxCgenJump (func));
      __losuSyntaxCgenPalist (func, v.value._bool.f,
                              __losuSyntaxCgenGetL (func));
      then (lex, &v);
    }
  if (lex->tk.token == TOKEN_ELSE)
    {
      __losuSyntaxCgenConcat (func, &el, __losuSyntaxCgenJump (func));
      __losuSyntaxCgenPalist (func, v.value._bool.f,
                              __losuSyntaxCgenGetL (func));
      __losuSyntaxParNext (lex);
      __losuSyntaxParBlock (lex);
    }
  else
    __losuSyntaxCgenConcat (func, &el, v.value._bool.f);

  __losuSyntaxCgenPalist (func, el, __losuSyntaxCgenGetL (func));
  __losuSyntaxParCheckMatch (lex, TOKEN_IF, TOKEN_ENDIF, line);
/* undef segment */
#undef condition
#undef then
}
static void
__losuSyntaxParStatWith (_syntaxLex *lex, int32_t line)
{
#define condition(lex, v)                                                     \
  {                                                                           \
    __losuSyntaxParSubExp ((lex), (v), -1);                                   \
    __losuSyntaxCgenGoT ((lex)->fs, (v), 0);                                  \
  }

  _syntaxFunc *func = lex->fs;
  _syntaxExp v;
  __losuSyntaxCgenCodearg0 (func, EC2INS_PUSHSTA);
  int32_t with = __losuSyntaxCgenGetL (func);
  _syntaxBreak br;
  int32_t *oldloop = lex->fs->loopStart;
  lex->fs->loopStart = &with;
  __losuSyntaxParEnterbreak (func, &br);
  __losuSyntaxParNext (lex);
  __losuSyntaxCgenCodearg0 (func, EC2INS_SETSTA);
  condition (lex, &v);
  // __losuSyntaxParCheck (lex, ':');
  __losuSyntaxParBlock (lex);
  lex->fs->loopStart = oldloop;
  __losuSyntaxCgenPalist (func, __losuSyntaxCgenJump (func), with);
  __losuSyntaxCgenPalist (func, v.value._bool.f, __losuSyntaxCgenGetL (func));
  __losuSyntaxParCheckMatch (lex, TOKEN_WHILE, TOKEN_ENDWHILE, line);
  __losuSyntaxParLeavebreak (func, &br);
  __losuSyntaxCgenCodearg0 (func, EC2INS_POPSTA);

#undef condition
}
static void
__losuSyntaxParStatBlock (_syntaxLex *lex)
{
  _syntaxFunc *func = lex->fs;
  int naloc = func->naloc;
  _l_bool is = 0;
  while (!is && !__losuSyntaxParCheckBlock (lex->tk.token))
    is = __losuSyntaxParStat (lex);
  __losuSyntaxCgenAdStack (func, func->naloc - naloc);
  __losuSyntaxParRemovelcvar (lex, func->naloc - naloc);
}

static void
__losuSyntaxParStatFunc (_syntaxLex *lex, int32_t line, _l_bool isMain)
{
  _syntaxExp v;
  __losuSyntaxParNext (lex);
  __losuSyntaxParStatFuncBody (
      lex, __losuSyntaxParStatFuncName (lex, &v, isMain), line, isMain);
  __losuSyntaxCgenSetVar (lex, &v);
}
static void
__losuSyntaxParStatVar (_syntaxLex *lex)
{
  // 被修改为 global
  // #define isSet(lex, c)
  //   ((lex->tk.token == c) ? (__losuSyntaxParNext (lex), 1) : 0)

  // if (lex->deepth == 0) /* global */
  //   {
  //     __losuSyntaxParNext (lex);
  //     __losuSyntaxParStatName (lex, 0);
  //     return;
  //   }
  _syntaxFunc *func = lex->fs;
  _inlineScode *f = func->fcode;
  int32_t nv = 0, ne = 0;
  do
    {
      __losuSyntaxParNext (lex);
      _inlineString *ts = __losuSyntaxParCheckName (lex);
      vm_addgvalue (lex->vm, ts->str);
      __losuSyntaxParNewlcvar (lex, ts, nv++);
      f->localvar[f->nlocalvar - 1].type = VG;
      // {
      //   _syntaxFunc *func = lex->fs;
      //   _inlineScode *f = func->fcode;

      //   /* check limit? */
      //   __losuSyntaxParCheckLimit (
      //       lex, func->naloc + i + 1, vmOl_MaxLocalvar,
      //       __config_losucore_errmsg_msgCheckLimitTMlocvar);

      //   __losu_mem_growvector (lex->vm, f->localvar, f->nlocalvar, 1,
      //                          _inlineLocalvar, vmIns_MaxA,
      //                          __config_losucore_errmsg_msgVectorOverflow);
      //   f->localvar[f->nlocalvar].name = name;
      //   f->localvar[f->nlocalvar].type = VL;
      //   func->aloc[func->naloc + i] = f->nlocalvar++;
      // }
    }
  while (lex->tk.token == ',');
  ne = 0;
  __losuSyntaxParAdStack (lex, nv, ne);
  __losuSyntaxParAdLcvar (lex, nv);
  // if (isSet (lex, '='))
  //   ne = __losuSyntaxParExplist (lex);
  // else
  //   ne = 0;
  // __losuSyntaxParAdStack (lex, nv, ne);
  // __losuSyntaxParAdLcvar (lex, nv);

#undef isSet
}
static void
__losuSyntaxParStatName (_syntaxLex *lex, _l_bool onlyDec)
{
  _syntaxFunc *func = lex->fs;
  _syntaxExp v;
  if (lex->deepth)
    __losuSyntaxParVarFunc (lex, &v, __losuSyntaxParExpVarlevel_left);
  else // 全局
    __losuSyntaxParVarFunc (lex, &v, __losuSyntaxParExpVarlevel_global);
  if (v.type == VE)
    {
      __losuSyntaxParCheckCondtion (lex, __losuSyntaxCgenIsopen (func),
                                    __config_losucore_errmsg_msgInvalidExp);
      __losuSyntaxCgenSetNcallr (func, 0);
    }
  else
    __losuSyntaxCgenAdStack (
        func, __losuSyntaxParStatNameAssment (lex, &v, 1, onlyDec));
}
static void
__losuSyntaxParStatReturn (_syntaxLex *lex)
{
  _syntaxFunc *func = lex->fs;
  __losuSyntaxParNext (lex);
  if (!__losuSyntaxParCheckBlock (lex->tk.token))
    __losuSyntaxParExplist (lex);
  __losuSyntaxCgenCodearg1 (func, INS_RETURN, lex->fs->naloc);
  func->stacklevel = func->naloc;
}
static void
__losuSyntaxParStatBreak (_syntaxLex *lex)
{
  _syntaxFunc *func = lex->fs;
  _syntaxBreak *br = func->breakl;
  int32_t c = func->stacklevel;
  __losuSyntaxParNext (lex);
  if (br) /* break lable is exsist  */
    {
      __losuSyntaxCgenAdStack (func, c - br->stacklevel);
      __losuSyntaxCgenConcat (func, &br->breaklist,
                              __losuSyntaxCgenJump (func));
      __losuSyntaxCgenAdStack (func, br->stacklevel - c);
    }
}

static void
__losuSyntaxParEnterbreak (_syntaxFunc *func, _syntaxBreak *br)
{
  br->stacklevel = func->stacklevel;
  br->breaklist = NO_JUMP;
  br->pre = func->breakl;
  func->breakl = br;
}
static void
__losuSyntaxParLeavebreak (_syntaxFunc *func, _syntaxBreak *br)
{
  func->breakl = br->pre;
  __losuSyntaxCgenPalist (func, br->breaklist, __losuSyntaxCgenGetL (func));
}

static _l_bool
__losuSyntaxParStatFuncName (_syntaxLex *lex, _syntaxExp *v, _l_bool isMain)
{
  _l_bool i = 0;
  _inlineString *ins = __losuSyntaxParCheckName (lex);
  __losuSyntaxParSvar (lex, ins, v, __losuSyntaxParExpVarlevel_function);
  if (isMain)
    lex->vm->main.funcname
        = __losu_objString_newconst (lex->vm, (const char *)ins->str)->str;
  while (lex->tk.token == '.')
    {
      __losuSyntaxParNext (lex);
      __losuSyntaxCgenTostack (lex, v, 1);
      __losuSyntaxCgenCodearg1 (
          lex->fs, INS_PUSHSTRING,
          __losuSyntaxParStrconst (lex->fs, __losuSyntaxParCheckName (lex)));
      v->type = VI;
      i = 1;
    }
  return i;
}
static void
__losuSyntaxParStatFuncBody (_syntaxLex *lex, _l_bool nself, int32_t line,
                             _l_bool isMain)
{

  _syntaxFunc nfs;

  __losuSyntaxParNewfunc (lex, &nfs);
  nfs.fcode->defedline = line;
  nfs.fcode->isMain = isMain;
  if (lex->tk.token == '(')
    __losuSyntaxParCheck (lex, '(');
  else
    {
      int32_t np = 0;
      __losuSyntaxParAdLcvar (lex, np);
      nfs.fcode->narg = nfs.naloc;
      nfs.fcode->isVarg = 0;
      __losuSyntaxCgenDtStack (&nfs, nfs.naloc);
      goto funcarg;
    }
  if (nself)
    {
      __losuSyntaxParNewlcvar (lex,
                               __losu_objString_newconst (lex->vm, "self"), 0);
      __losuSyntaxParAdLcvar (lex, 1);
    }
  __losuSyntaxParStatFuncParlist (lex, isMain);
  __losuSyntaxParCheck (lex, ')');
funcarg:
  // __losuSyntaxParCheck (lex, ':');

  while (!__losuSyntaxParCheckBlock (lex->tk.token))
    if (__losuSyntaxParStat (lex))
      break;
  if (isMain)
    __losuSyntaxParCheckMatch (lex, TOKEN_SUB, TOKEN_ENDSUB, line);
  else
    __losuSyntaxParCheckMatch (lex, TOKEN_FUNC, TOKEN_ENDFUNC, line);
  __losuSyntaxParDelfunc (lex);

  /* push func */
  _syntaxFunc *func = lex->fs;
  _inlineScode *f = func->fcode;
  for (int32_t i = 0; i < nfs.nclosure; i++)
    __losuSyntaxCgenTostack (lex, &(nfs.closure[i]), 1);
  __losu_mem_growvector (lex->vm, f->lcscode, f->nlcscode, 1, _inlineScode *,
                         vmIns_MaxA,
                         __config_losucore_errmsg_msgVectorOverflow);
  f->lcscode[f->nlcscode++] = nfs.fcode;

  __losuSyntaxCgenCodearg2 (func, INS_PUSHFUNCTION, f->nlcscode - 1,
                            nfs.nclosure);
}
static void
__losuSyntaxParStatFuncParlist (_syntaxLex *lex, _l_bool isMain)
{
#define option(lex)                                                           \
  ((lex->tk.token == ',') ? (__losuSyntaxParNext (lex), 1) : 0)
  int32_t i = 0;
  _l_bool isV = 0;
  int32_t np = 0;
  _syntaxFunc *func = lex->fs;
  if (lex->tk.token != ')')
    {
      do
        {
          switch (lex->tk.token)
            {
            case TOKEN_ARG:
              {
                __losuSyntaxParNext (lex);
                isV = 1;
                break;
              }
            case TOKEN_NAME:
              {
                _inlineString *Istr = __losuSyntaxParCheckName (lex);
                if (isMain)
                  lex->vm->main.funcargs[i++]
                      = __losu_objString_newconst (lex->vm, Istr->str)->str;

                __losuSyntaxParNewlcvar (lex, Istr, np++);
                break;
              }
            default:
              {
                __losuSyntaxError (lex, __config_losucore_errmsg_msgCheckName);
              }
            }
        }
      while (option (lex));
    }
  __losuSyntaxParAdLcvar (lex, np);
  func->fcode->narg = func->naloc;
  func->fcode->isVarg = isV;
  if (isV)
    {
      __losuSyntaxParNewlcvar (lex, __losu_objString_newconst (lex->vm, "arg"),
                               0);
      __losuSyntaxParAdLcvar (lex, 1);
    }
  __losuSyntaxCgenDtStack (func, func->naloc);

#undef option
}

static int32_t
__losuSyntaxParStatNameAssment (_syntaxLex *lex, _syntaxExp *v, int32_t nvar,
                                _l_bool onlyDec)
{
  _l_bool b = 0;
  int32_t left = 0;
  if (lex->tk.token == ',')
    {
      _syntaxExp nv;
      __losuSyntaxParNext (lex);
      __losuSyntaxParVarFunc (lex, &nv, __losuSyntaxParExpVarlevel_left);
      __losuSyntaxParCheckCondtion (lex, (nv.type != VE),
                                    __config_losucore_errmsg_msgInvalidExp);
      left = __losuSyntaxParStatNameAssment (lex, &nv, nvar + 1, onlyDec);
    }
  else
    {
      int32_t nexp = 0;
      if (lex->tk.token == '=') // 拷贝赋值
        {
          b = 0;
          __losuSyntaxParNext (lex);
          nexp = __losuSyntaxParExplist (lex);
        }
      else if (lex->tk.token == TOKEN_SET) // 引用赋值
        {
          b = 1;
          __losuSyntaxParNext (lex);
          nexp = __losuSyntaxParExplist (lex);
        }
      else if (onlyDec) // 引用赋值
        __losuSyntaxError (lex, "未知的关键词，起始于 %d 行", onlyDec);
      __losuSyntaxParAdStack (lex, nvar, nexp);
    }
  if (b == 0)
    {
      if (v->type != VI)
        __losuSyntaxCgenSetVar (lex, v);
      else
        {
          __losuSyntaxCgenCodearg2 (lex->fs, INS_SETUNIT, left + nvar + 2, 1);
          left += 2;
        }
    }
  else // 引用赋值
    {
      if (v->type != VI)
        {
          // __losuSyntaxCgenSetVar (lex, v);
          _syntaxFunc *func = lex->fs;
          _syntaxExp *var = v;
          switch (var->type)
            {
            case VL:
              __losuSyntaxCgenCodearg1 (func, EC2INS_SETLOCAL,
                                        var->value.index);
              break;
            case VG:
              __losuSyntaxCgenCodearg1 (func, EC2INS_SETGLOBAL,
                                        var->value.index);
              break;
            case VI:
              __losuSyntaxCgenCodearg2 (func, EC2INS_SETUNIT, 3, 3);
              break;
            default:
              break;
            }
        }
      else
        {
          __losuSyntaxCgenCodearg2 (lex->fs, EC2INS_SETUNIT, left + nvar + 2,
                                    1);
          left += 2;
        }
    }
  return left;
}

static int32_t
__losuSyntaxParExplist (_syntaxLex *lex)
{
  int32_t n = 1;
  _syntaxExp v;
  __losuSyntaxParSubExp (lex, &v, -1);
  while (lex->tk.token == ',')
    {
      __losuSyntaxCgenTostack (lex, &v, 1);
      __losuSyntaxParNext (lex);
      __losuSyntaxParSubExp (lex, &v, -1);
      n++;
    }
  __losuSyntaxCgenTostack (lex, &v, 0);
  return n;
}
static void
__losuSyntaxParAdStack (_syntaxLex *lex, int32_t nv, int32_t ne)
{
  _syntaxFunc *func = lex->fs;
  int32_t d = ne - nv;
  if (ne > 0 && __losuSyntaxCgenIsopen (func))
    {
      d--;
      if (d <= 0)
        {
          __losuSyntaxCgenSetNcallr (func, -d);
          d = 0;
        }
      else
        __losuSyntaxCgenSetNcallr (func, 0);
    }
  __losuSyntaxCgenAdStack (func, d);
}
static void
__losuSyntaxParAdLcvar (_syntaxLex *lex, int32_t nv)
{
  while (nv--)
    lex->fs->fcode->localvar[lex->fs->aloc[lex->fs->naloc++]].startpc
        = lex->fs->pc;
}
static void
__losuSyntaxParVarFunc (_syntaxLex *lex, _syntaxExp *v, _sytaxLocal getlocal)
{
#define lookahead(lex)                                                        \
  {                                                                           \
    (lex)->tkahead.token = __losuSyntaxLexNext (                              \
        lex, (_syntaxTkvalue *)(&((lex)->tkahead.info)));                     \
  }

  __losuSyntaxParSvar (lex, __losuSyntaxParCheckName (lex), v, getlocal);
  while (1)
    {
      switch (lex->tk.token)
        {
        case '.':
          {
            __losuSyntaxParNext (lex);
            lookahead (lex);
            switch (lex->tkahead.token)
              {
              case '(':
              case '{':
              case TOKEN_STRING:
                {
                  int32_t name = __losuSyntaxParStrconst (
                      lex->fs, __losuSyntaxParCheckName (lex));
                  __losuSyntaxCgenTostack (lex, v, 1);
                  __losuSyntaxCgenCodearg1 (lex->fs, INS_PUSHSELF, name);
                  __losuSyntaxParExpFargs (lex, 1);
                  v->type = VE;
                  v->value._bool.t = v->value._bool.f = NO_JUMP;
                  break;
                }
              default:
                {
                  __losuSyntaxCgenTostack (lex, v, 1);
                  __losuSyntaxCgenCodearg1 (
                      lex->fs, INS_PUSHSTRING,
                      __losuSyntaxParStrconst (
                          lex->fs, __losuSyntaxParCheckName (lex)));
                  v->type = VI;
                  break;
                }
              }
            break;
          }
        case '[':
          {
            __losuSyntaxParNext (lex);
            __losuSyntaxCgenTostack (lex, v, 1);
            v->type = VI;
            __losuSyntaxParExp (lex);
            __losuSyntaxParCheck (lex, ']');
            break;
          }
        case '(':
        case TOKEN_STRING:
        case '{':
          {
            __losuSyntaxCgenTostack (lex, v, 1);
            __losuSyntaxParExpFargs (lex, 0);
            v->type = VE;
            v->value._bool.f = v->value._bool.t = NO_JUMP;
            break;
          }
        default:
          return;
        }
    }

#undef lookahead
}

static void
__losuSyntaxParExp (_syntaxLex *lex)
{
  _syntaxExp v;
  __losuSyntaxParSubExp (lex, &v, -1);
  __losuSyntaxCgenTostack (lex, &v, 1);
}
static int32_t
__losuSyntaxParExpVarlevel (_syntaxLex *lex, _inlineString *n, _syntaxExp *v)
{
  _syntaxFunc *func;
  int32_t lev = 0;
  for (func = lex->fs; func; func = func->prev)
    {
      for (int32_t i = func->naloc - 1; i >= 0; i--)
        {
          if (n == func->fcode->localvar[func->aloc[i]].name)
            {
              v->type = VL;
              v->value.index = i;
              return lev;
            }
        }
      lev++;
    }
  v->type = VG;
  return -1;
}

static int32_t
__losuSyntaxParExpVarlevel_function (_syntaxLex *lex, _inlineString *n,
                                     _syntaxExp *v)
{
  /*
    创建函数
    修改全局作用域
    修改全局符号表
  */
  // _syntaxFunc *func;
  // int32_t lev = 0;
  // for (func = lex->fs; func; func = func->prev)
  //   {
  //     for (int32_t i = func->naloc - 1; i >= 0; i--)
  //       {
  //         if (n == func->fcode->localvar[func->aloc[i]].name)
  //           {
  //             v->type = VL;
  //             v->value.index = i;
  //             return lev;
  //           }
  //       }
  //     lev++;
  //   }
  // v->type = VG;

  vm_addgsymbol (lex->vm, n->str);
  vm_addgvalue (lex->vm, n->str);
  v->type = VG;
  return -1;
}

static int32_t
__losuSyntaxParExpVarlevel_left (_syntaxLex *lex, _inlineString *n,
                                 _syntaxExp *v)
{
  /*
    是否在全局符号表？-> VG
    |-> 是否在当前作用域? -> VG/VL
        |-> 创建局部变量 -> VL
  */
  _syntaxFunc *func = lex->fs;
  // 查找全局符号表
  for (int32_t i = 0; i < lex->vm->ng_symbol; i++)
    {
      if (strcmp (lex->vm->global_symbol[i], n->str) == 0)
        {
          v->type = VG;
          return -1;
        }
    }
  // 查找当前作用域
  for (int32_t i = func->naloc - 1; i >= 0; i--)
    {
      if (n == func->fcode->localvar[func->aloc[i]].name)
        {
          v->type = func->fcode->localvar[func->aloc[i]].type;
          if (v->type == VG)
            return -1;
          v->value.index = i;
          return 0;
        }
    }
  // 查不到，创建
  __losuSyntaxParNewlcvar (lex, n, 0);
  __losuSyntaxParAdStack (lex, 1, 0);
  __losuSyntaxParAdLcvar (lex, 1);
  // printf ("create lcvar '%s'\n", n->str);
  return __losuSyntaxParExpVarlevel_left (lex, n, v);
}

static int32_t
__losuSyntaxParExpVarlevel_right (_syntaxLex *lex, _inlineString *n,
                                  _syntaxExp *v)
{
  /*
    是否在全局符号表？-> VG
    |-> 是否在当前作用域? -> VG/VL
        |-> 是否在全局作用域？ -> VG
            |-> 报错
  */
  _syntaxFunc *func = lex->fs;
  // 查找全局符号表
  for (int32_t i = 0; i < lex->vm->ng_symbol; i++)
    {
      if (strcmp (lex->vm->global_symbol[i], n->str) == 0)
        {
          v->type = VG;
          return -1;
        }
    }
  // 查找当前作用域
  for (int32_t i = func->naloc - 1; i >= 0; i--)
    {
      if (n == func->fcode->localvar[func->aloc[i]].name)
        {

          v->type = func->fcode->localvar[func->aloc[i]].type;
          if (v->type == VG)
            return -1;
          v->value.index = i;
          return 0;
        }
    }
  // 查找全局变量域
  for (int32_t i = 0; i < lex->vm->ng_value; i++)
    {
      if (strcmp (lex->vm->global_value[i], n->str) == 0)
        {
          v->type = VG;
          return -1;
        }
    }
  // 报错
  __losuSyntaxError (lex, "找不到变量 '%s' ", n->str);
  return 2;
}

static int32_t
__losuSyntaxParExpVarlevel_global (_syntaxLex *lex, _inlineString *n,
                                   _syntaxExp *v)
{
  vm_addgvalue (lex->vm, n->str);
  v->type = VG;
  return -1;
}

static void
__losuSyntaxParSvar (_syntaxLex *lex, _inlineString *n, _syntaxExp *var,
                     _sytaxLocal getlocal)
{
  int32_t l = getlocal (lex, n, var);
  if (l == -1)
    var->value.index = __losuSyntaxParStrconst (lex->fs, n);
  else if (l >= 1)
    __losuSyntaxError (lex, "错误的闭包变量 '%s'", n->str);

  // int32_t l = __losuSyntaxParExpVarlevel (lex, n, var);
  // if (l >= 1)
  //   {
  //     { /* push closure */
  //       _syntaxFunc *func = lex->fs;
  //       _syntaxExp v;
  //       int32_t l = __losuSyntaxParExpVarlevel (lex, n, &v);
  //       if (l == -1)
  //         v.value.index = __losuSyntaxParStrconst (func->prev, n);
  //       else if (l != 1)
  //         __losuSyntaxError (lex,
  //         __config_losucore_errmsg_msgInvalidClosure,
  //                            n->str);

  //       /* index closure */
  //       int16_t c = -1;
  //       for (int16_t i = 0; i < func->nclosure; i++)
  //         if (func->closure[i].type == v.type
  //             && func->closure[i].value.index == v.value.index)
  //           {
  //             c = i;
  //             break;
  //           }
  //       if (c < 0)
  //         {
  //           func->closure[func->nclosure] = v;
  //           c = func->nclosure++;
  //         }
  //       __losuSyntaxCgenCodearg1 (func, INS_PUSHUPVALUE, c);
  //     }
  //     var->type = VE;
  //     var->value._bool.t = var->value._bool.f = NO_JUMP;
  //   }
  // else if (l == -1)
  //   var->value.index = __losuSyntaxParStrconst (lex->fs, n);
}
static int32_t
__losuSyntaxParStrconst (_syntaxFunc *func, _inlineString *s)
{
  _inlineScode *f = func->fcode;
  int32_t c = s->cstidx;
  if (c >= f->nlcstr || f->lcstr[c] != s)
    {
      __losu_mem_growvector (func->vm, f->lcstr, f->nlcstr, 1, _inlineString *,
                             vmIns_MaxU,
                             __config_losucore_errmsg_msgVectorOverflow);
      c = f->nlcstr++;
      s->cstidx = c;
      f->lcstr[c] = s;
    }
  return c;
}

static int32_t
__losuSyntaxParSubExp (_syntaxLex *lex, _syntaxExp *v, int l)
{
  struct
  {
    char left;
    char right;
  } const priority[] = {
    { 7, 7 }, /* + */
    { 7, 7 }, /* - */
    { 8, 8 }, /* * */
    { 8, 8 }, /* / */
    { 8, 8 }, /* % */
    { 8, 8 }, /* // */

    // bit
    { 5, 5 }, /* & */
    { 3, 3 }, /* | */
    { 4, 4 }, /* ^ */
    { 6, 6 }, /* << */
    { 6, 6 }, /* >> */

    { 2, 2 }, /* != */
    { 2, 2 }, /* == */

    { 2, 2 }, /* < */
    { 2, 2 }, /* <= */
    { 2, 2 }, /* > */
    { 2, 2 }, /* >= */

    { 1, 1 }, /* and */
    { 1, 1 }, /* or */
  };

  int op, uop;
  switch (lex->tk.token)
    {
    case TOKEN_NOT:
      uop = INS_UNOP_NOT;
      break;
    case '-':
      uop = INS_UNOP_NEG;
      break;
    case '~':
      uop = INS_UNOP_BITNOT;
      break;
    default:
      uop = INS_UNOP_NULL;
      break;
    }
  if (uop != INS_UNOP_NULL)
    {
      __losuSyntaxParNext (lex);
      __losuSyntaxParSubExp (lex, v, 9);
      __losuSyntaxCgenPrefix (lex, uop, v);
    }
  else
    __losuSyntaxParSimpleExp (lex, v);

  switch (lex->tk.token)
    {
    case '+':
      op = INS_BINOP_ADD;
      break;
    case '-':
      op = INS_BINOP_SUB;
      break;
    case '*':
      op = INS_BINOP_MULT;
      break;
    case '/':
      op = INS_BINOP_DIV;
      break;
    case '%':
      op = INS_BINOP_MOD;
      break;
    case TOKEN_DIVI:
      op = INS_BINOP_DIVI;
      break;
    case TOKEN_EQ:
      op = INS_BINOP_EQ;
      break;
    case TOKEN_NE:
      op = INS_BINOP_NE;
      break;
    case '<':
      op = INS_BINOP_LT;
      break;
    case TOKEN_LE:
      op = INS_BINOP_LE;
      break;
    case '>':
      op = INS_BINOP_GT;
      break;
    case TOKEN_GE:
      op = INS_BINOP_GE;
      break;
    case TOKEN_AND:
      op = INS_BINOP_AND;
      break;
    case TOKEN_OR:
      op = INS_BINOP_OR;
      break;
    case '&':
      op = INS_BINOP_BITAND;
      break;
    case '|':
      op = INS_BINOP_BITOR;
      break;
    case '^':
      op = INS_BINOP_BITXOR;
      break;
    case TOKEN_LSH:
      op = INS_BINOP_BITLSH;
      break;
    case TOKEN_RSH:
      op = INS_BINOP_BITRSH;
      break;
    default:
      op = INS_BINOP_NULL;
      break;
    }
  /* printf ("\top:%d\n", op); */
  while (op != INS_BINOP_NULL && priority[op].left > l)
    {
      _syntaxExp v2;
      int l2;
      __losuSyntaxParNext (lex);
      __losuSyntaxCgenInfix (lex, op, v);
      l2 = __losuSyntaxParSubExp (lex, &v2, priority[op].right);
      __losuSyntaxCgenPosfix (lex, op, v, &v2);
      op = l2;
    }
  return op;
}
static void
__losuSyntaxParExpFargs (_syntaxLex *lex, _l_bool slf)
{
  _syntaxFunc *func = lex->fs;
  int sl = func->stacklevel - slf - 1;
  int line = lex->linenumber;
  switch (lex->tk.token)
    {
    case '(':
      {
        __losuSyntaxParNext (lex);
        if (lex->tk.token != ')')
          __losuSyntaxParExplist (lex);
        __losuSyntaxParCheckMatch (lex, '(', ')', line);
        break;
      }
    case '{':
      {
        __losuSyntaxParUnitConstructor (lex, '{', '}');
        break;
      }
    case TOKEN_STRING:
      {
        __losuSyntaxCgenCodearg1 (
            lex->fs, INS_PUSHSTRING,
            __losuSyntaxParStrconst (lex->fs, lex->tk.info.s));
        __losuSyntaxParNext (lex);
        break;
      }
    default:
      {
        __losuSyntaxError (lex, __config_losucore_errmsg_msgExpectedFarg);
        break;
      }
    }
  func->stacklevel = sl;
  __losuSyntaxCgenCodearg2 (func, INS_CALL, sl, 255);
}

static void
__losuSyntaxParSimpleExp (_syntaxLex *lex, _syntaxExp *v)
{
  _syntaxFunc *func = lex->fs;
  switch (lex->tk.token)
    {
    case TOKEN_NULL:
      {
        __losuSyntaxCgenAdStack (func, -1);
        __losuSyntaxParNext (lex);
        break;
      }

    case TOKEN_NUMBER:
      {
        _l_number n = lex->tk.info.num;
        __losuSyntaxParNext (lex);
        __losuSyntaxCgenNumber (func, n);
        break;
      }
    case TOKEN_INT:
      {
        _l_int i = lex->tk.info.intnum;
        int32_t pc = __losuSyntaxCgenCodearg1 (lex->fs, EC2INS_PUSHINT, 0);
        cgIns_SetS (lex->fs->fcode->code[pc], i);
        __losuSyntaxParNext (lex);
        break;
      }
    case TOKEN_UNICODE:
      {
        __losuSyntaxCgenCodearg1 (lex->fs, EC2INS_PUSHUNICODE,
                                  lex->tk.info.unicode);
        __losuSyntaxParNext (lex);
        break;
      }
    case TOKEN_CHAR:
      {
        __losuSyntaxCgenCodearg1 (lex->fs, EC2INS_PUSHCHAR,
                                  lex->tk.info._char);
        __losuSyntaxParNext (lex);
        break;
      }
    case TOKEN_TRUE:
      {
        __losuSyntaxCgenCodearg1 (lex->fs, EC2INS_PUSHBOOL, 1);
        __losuSyntaxParNext (lex);
        break;
      }
    case TOKEN_FALSE:
      {
        __losuSyntaxCgenCodearg1 (lex->fs, EC2INS_PUSHBOOL, 0);
        __losuSyntaxParNext (lex);
        break;
      }
    case TOKEN_SPACE:
      {
        __losuSyntaxCgenCodearg0 (lex->fs, EC2INS_PUSHSPACE);
        __losuSyntaxParNext (lex);
        break;
      }
    case TOKEN_STRING:
      {
        __losuSyntaxCgenCodearg1 (
            lex->fs, INS_PUSHSTRING,
            __losuSyntaxParStrconst (lex->fs, lex->tk.info.s));
        __losuSyntaxParNext (lex);
        break;
      }
    case TOKEN_BYTES:
      {
        __losuSyntaxCgenCodearg1 (
            lex->fs, EC2INS_PUSHBYTES,
            __losuSyntaxParStrconst (lex->fs, lex->tk.info.s));
        __losuSyntaxParNext (lex);
        break;
      }

    case TOKEN_NAME:
      {
        __losuSyntaxParVarFunc (lex, v, __losuSyntaxParExpVarlevel_right);
        return;
      }
    case '{':
      {
        __losuSyntaxParUnitConstructor (lex, '{', '}');
        break;
      }
    case '[':
      {
        __losuSyntaxParUnitConstructor (lex, '[', ']');
        break;
      }
    case '(':
      {
        __losuSyntaxParNext (lex);
        __losuSyntaxParSubExp (lex, v, -1);
        __losuSyntaxParCheck (lex, ')');
        return;
      }
    default:
      {
        __losuSyntaxError (lex, __config_losucore_errmsg_msgInvalidExp);
        return;
      }
    }
  v->type = VE;
  v->value._bool.t = v->value._bool.f = NO_JUMP;
}

static void
__losuSyntaxParUnitConstructor (_syntaxLex *lex, char sflag, char eflag)
{
  _l_bool isMap = 0;
  _syntaxFunc *func = lex->fs;
  int32_t line = lex->linenumber;
  int32_t pc = __losuSyntaxCgenCodearg2 (func, INS_CREATEUNIT, 0, 0);
  int32_t niss;
  __losuSyntaxParCheck (lex, sflag);

  if (sflag == '{')
    isMap = 1;

  if (lex->tk.token == eflag)
    niss = 0;
  else if (sflag == '[')
    niss = __losuSyntaxParUnitListfield (lex, eflag);

  else
    switch (lex->tk.token)
      {
      case TOKEN_NAME:
        {
          lex->tkahead.token = __losuSyntaxLexNext (
              lex, (_syntaxTkvalue *)(&(lex->tkahead.info)));
          if (lex->tkahead.token != ':') // ahead is not ':'
            goto case_default;
          niss = __losuSyntaxParUnitMapfield (lex); // Map
          break;
        }
      case TOKEN_STRING:
        {
          lex->tkahead.token = __losuSyntaxLexNext (
              lex, (_syntaxTkvalue *)(&(lex->tkahead.info)));
          if (lex->tkahead.token != ':') // ahead is not ':'
            goto case_default;
          niss = __losuSyntaxParUnitMapfield (lex); // Map
          break;
        }
      case '[':
        {
          niss = __losuSyntaxParUnitMapfield (lex);
          break;
        }
      default:
        {
        case_default: // default is List
          __losuSyntaxError (lex, "错误的 '映射' 格式");
          break;
        }
      }
  __losuSyntaxParCheckMatch (lex, sflag, eflag, line);
  cgIns_SetB (func->fcode->code[pc], isMap);
  cgIns_SetA (func->fcode->code[pc], niss);
}

static int32_t
__losuSyntaxParUnitMapfield (_syntaxLex *lex)
{
  _syntaxFunc *func = lex->fs;
  int32_t n = 1;
  switch (lex->tk.token)
    {
    case TOKEN_NAME:
      {
        __losuSyntaxCgenCodearg1 (
            lex->fs, INS_PUSHSTRING,
            __losuSyntaxParStrconst (lex->fs, __losuSyntaxParCheckName (lex)));
        break;
      }
    case TOKEN_STRING:
      {
        __losuSyntaxCgenCodearg1 (
            lex->fs, INS_PUSHSTRING,
            __losuSyntaxParStrconst (lex->fs, lex->tk.info.s));
        __losuSyntaxParNext (lex);
        break;
      }
    case '[':
      {
        __losuSyntaxParNext (lex);
        __losuSyntaxParExp (lex);
        __losuSyntaxParCheck (lex, ']');
        break;
      }
    default:
      {
        __losuSyntaxError (lex, __config_losucore_errmsg_msgCheckName);
      }
    }

  __losuSyntaxParCheck (lex, ':');
  __losuSyntaxParExp (lex);
  while (lex->tk.token == ',')
    {
      __losuSyntaxParNext (lex);
      if (lex->tk.token == '}')
        break;
      switch (lex->tk.token)
        {
        case TOKEN_NAME:
          {
            __losuSyntaxCgenCodearg1 (
                lex->fs, INS_PUSHSTRING,
                __losuSyntaxParStrconst (lex->fs,
                                         __losuSyntaxParCheckName (lex)));
            break;
          }
        case TOKEN_STRING:
          {
            __losuSyntaxCgenCodearg1 (
                lex->fs, INS_PUSHSTRING,
                __losuSyntaxParStrconst (lex->fs, lex->tk.info.s));
            __losuSyntaxParNext (lex);
            break;
          }
        case '[':
          {
            __losuSyntaxParNext (lex);
            __losuSyntaxParExp (lex);
            __losuSyntaxParCheck (lex, ']');
            break;
          }
        default:
          {
            __losuSyntaxError (lex, __config_losucore_errmsg_msgCheckName);
          }
        }
      __losuSyntaxParCheck (lex, ':');
      __losuSyntaxParExp (lex);
      n++;
      if (n % vmOl_MaxSetmap == 0)
        __losuSyntaxCgenCodearg1 (func, INS_SETMAP, vmOl_MaxSetmap);
    }
  __losuSyntaxCgenCodearg1 (func, INS_SETMAP, n % vmOl_MaxSetmap);
  return n;
}

static int32_t
__losuSyntaxParUnitListfield (_syntaxLex *lex, char eflag)
{
  _syntaxFunc *func = lex->fs;
  int32_t n = 1;
  __losuSyntaxParExp (lex);
  while (lex->tk.token == ',')
    {
      __losuSyntaxParNext (lex);
      if (lex->tk.token == eflag)
        break;
      __losuSyntaxParExp (lex);
      n++;
      if (n % vmOl_MaxSetlist == 0)
        __losuSyntaxCgenCodearg2 (func, INS_SETLIST, n / vmOl_MaxSetlist - 1,
                                  vmOl_MaxSetlist);
    }
  __losuSyntaxCgenCodearg2 (func, INS_SETLIST, n / vmOl_MaxSetlist,
                            n % vmOl_MaxSetlist);
  return n;
}

#undef NO_JUMP
#undef __losuSyntaxParBlock
#endif /* DEFINE_SOURCE_LOSU_SYNTAX_C_PARSER */

#ifndef DEFINE_SOURCE_LOSU_SYNTAX_C_CODEGEN
#define DEFINE_SOURCE_LOSU_SYNTAX_C_CODEGEN
#define NO_JUMP -1

static const struct
{
  vmIns_OP op;
  int arg;
} _alops[] = {
  // num
  { INS_ADD, 0 },
  { INS_SUB, 0 },
  { INS_MULT, 0 },
  { INS_DIV, 0 },
  { INS_POW, 0 },
  { INS_CONCAT, 0 },
  // bit
  { EC2INS_BITAND, 0 },
  { EC2INS_BITOR, 0 },
  { EC2INS_BITXOR, 0 },
  { EC2INS_BITLSH, 0 },
  { EC2INS_BITRSH, 0 },
  // jmp
  { INS_JMPNE, NO_JUMP },
  { INS_JMPEQ, NO_JUMP },
  { INS_JMPLT, NO_JUMP },
  { INS_JMPLE, NO_JUMP },
  { INS_JMPGT, NO_JUMP },
  { INS_JMPGE, NO_JUMP }
};

#define iO INS_MODE_OP
#define iBA INS_MODE_IBA
#define iU INS_MODE_IU
#define iS INS_MODE_IS
#define V 0 /* V means variable */
static const struct
{
  vmIns_Mode op;
  int8_t push;
  int8_t pop;
} _vmops[] = {
  { iO, 0, 0 }, /* INS_END */
  { iU, 0, 0 }, /* INS_RETURN */

  { iBA, 0, 0 }, /* INS_CALL */

  { iU, V, 0 }, /* INS_PUSHNULL */
  { iU, V, 0 }, /* INS_POP */

  { iU, 1, 0 },  /* INS_PUSHSTRING */
  { iU, 1, 0 },  /* INS_PUSHNUM */
  { iU, 1, 0 },  /* INS_PUSHUPVALUE */
  { iBA, V, 0 }, /* INS_PUSHFUNCTION */
  { iU, 2, 1 },  /* INS_PUSHSELF */

  { iU, 1, 0 }, /* INS_GETLOCAL */
  { iU, 1, 0 }, /* INS_GETGLOBAL */
  { iU, 0, 1 }, /* INS_SETLOCAL */
  { iU, 0, 1 }, /* INS_SETGLOBAL */

  { iBA, 1, 0 }, /* INS_CREATEUNIT */
  { iBA, V, 0 }, /* INS_SETUNIT */
  { iO, 1, 2 },  /* INS_GETUNIT */
  { iBA, V, 0 }, /* INS_SETLIST */
  { iU, V, 0 },  /* INS_SETMAP */

  { iO, 1, 2 }, /* INS_ADD */
  { iO, 1, 2 }, /* INS_SUB */
  { iO, 1, 2 }, /* INS_MULT */
  { iO, 1, 2 }, /* INS_DIV */
  { iO, 1, 2 }, /* INS_POW */
  { iO, 1, 2 }, /* INS_CONCAT */
  { iO, 1, 1 }, /* INS_NEG */
  { iO, 1, 1 }, /* INS_NOT */

  { iS, 0, 2 }, /* INS_JMPNE */
  { iS, 0, 2 }, /* INS_JMPEQ */
  { iS, 0, 2 }, /* INS_JMPLT */
  { iS, 0, 2 }, /* INS_JMPLE */
  { iS, 0, 2 }, /* INS_JMPGT */
  { iS, 0, 2 }, /* INS_JMPGE */

  { iS, 0, 1 }, /* INS_JMPT */
  { iS, 0, 1 }, /* INS_JMPF */
  { iS, 0, 1 }, /* INS_JMPONT */
  { iS, 0, 1 }, /* INS_JMPONF */
  { iS, 0, 0 }, /* INS_JMP */
  { iO, 0, 0 }, /* INS_PUSHNULLJMP */

  { iS, 0, 0 }, /* INS_FORPREP */
  { iS, 0, 3 }, /* INS_FORLOOP */
  { iS, 2, 0 }, /* INS_LFORPREP */
  { iS, 0, 3 }, /* INS_LFORLOOP */

  { iU, 0, 0 }, /* INS_YIELD */
  { iO, 1, 0 }, /* EC2INS_PUSHSPACE */
  { iU, 1, 0 }, /* EC2INS_PUSHUNICODE */
  { iU, 1, 0 }, /* EC2INS_ PUSHCHAR*/
  { iS, 1, 0 }, /* EC2INS_PUSHINT */
  { iU, 1, 0 }, /* EC2INS_PUSHBOOL */

  { iO, 1, 2 }, /* EC2INS_BITAND */
  { iO, 1, 2 }, /* EC2INS_BITOR */
  { iO, 1, 1 }, /* EC2INS_BITNOT */
  { iO, 1, 2 }, /* EC2INS_BITXOR */
  { iO, 1, 2 }, /* EC2INS_BITLSH */
  { iO, 1, 2 }, /* EC2INS_BITRSH */

  { iO, 0, 0 }, /* EC2INS_PUSHSTA */
  { iO, 0, 0 }, /* EC2INS_POPSTA */
  { iO, 0, 0 }, /* EC2INS_SETSTA */

  { iU, 0, 1 },  /* EC2INS_SETLOCAL */
  { iU, 0, 1 },  /* EC2INS_SETGLOBAL */
  { iBA, V, 0 }, /* EC2INS_SETUNIT */
  { iU, 1, 0 },  /* EC2INS_PUSHBYTES */

};

#undef iO
#undef iBA
#undef iU
#undef iS
#undef V

static int32_t
__losuSyntaxCgenCodearg (_syntaxFunc *func, vmIns_OP o, int32_t arg1,
                         int32_t arg2)
{
  _syntaxLex *lex = func->lexer;
  _inlineScode *f = func->fcode;
  vmInstruction i = func->pc > func->lasttarget
                        ? func->fcode->code[func->pc - 1]
                        : ((vmInstruction)INS_END);
  int32_t dt = _vmops[o].push - _vmops[o].pop;
  _l_bool optd = 0;

  switch (o)
    {
    case INS_PUSHFUNCTION: /* V */
      {
        dt = -arg2 + 1;
        break;
      }
    case INS_SETUNIT:
    case EC2INS_SETUNIT:
      {
        dt = -arg2;
        break;
      }

    case INS_SETLIST:
      {
        if (arg2 == 0)
          return NO_JUMP;
        dt = -arg2;
        break;
      }
    case INS_SETMAP:
      {
        if (arg1 == 0)
          return NO_JUMP;
        dt = -2 * arg1;
        break;
      }
    case INS_PUSHNULL:
      {
        if (arg1 == 0)
          return NO_JUMP;
        dt = arg1;
        switch (cgIns_GetOp (i))
          {
          case INS_PUSHNULL:
            cgIns_SetU (i, cgIns_GetU (i) + arg1);
            optd = 1;
            break;
          default:
            break;
          }
        break;
      }
    case INS_POP: /* V */
      {
        if (arg1 == 0)
          return NO_JUMP;
        dt = -arg1;
        switch (cgIns_GetOp (i))
          {
          case INS_SETUNIT:
          case EC2INS_SETUNIT:
            {
              cgIns_SetB (i, cgIns_GetB (i) + arg1);
              optd = 1;
              break;
            }
          default:
            break;
          }
        break;
      }
    case INS_JMPNE:
      {
        if (i == cgIns_NewU (INS_PUSHNULL, 1))
          {
            i = cgIns_NewS (INS_JMPT, NO_JUMP);
            optd = 1;
          }
        break;
      }
    case INS_JMPEQ:
      {
        if (i == cgIns_NewU (INS_PUSHNULL, 1))
          {
            i = cgIns_NewOp (INS_NOT);
            dt = -1;
            optd = 1;
          }
        break;
      }
    case INS_JMPT:
    case INS_JMPONT:
      {
        switch (cgIns_GetOp (i))
          {
          case INS_NOT:
            {
              i = cgIns_NewS (INS_JMPF, NO_JUMP);
              optd = 1;
              break;
            }
          case INS_PUSHNULL:
            {
              if (cgIns_GetU (i) == 1)
                {
                  func->pc--;
                  __losuSyntaxCgenDtStack (func, -1);
                  return NO_JUMP;
                }
              break;
            }
          default:
            break;
          }
        break;
      }
    case INS_JMPF:
    case INS_JMPONF:
      {
        switch (cgIns_GetOp (i))
          {
          case INS_NOT:
            {
              i = cgIns_NewS (INS_JMPT, NO_JUMP);
              optd = 1;
              break;
            }
          case INS_PUSHNULL:
            {
              if (cgIns_GetU (i) == 1)
                {
                  i = cgIns_NewS (INS_JMP, NO_JUMP);
                  optd = 1;
                }
              break;
            }
          default:
            break;
          }
        break;
      }
    default:
      break;
    }

  __losuSyntaxCgenDtStack (func, dt);
  if (optd)
    {
      func->fcode->code[func->pc - 1] = i;
      return func->pc - 1;
    }
  switch (_vmops[o].op)
    {
    case INS_MODE_OP:
      {
        i = cgIns_NewOp (o);
        break;
      }
    case INS_MODE_IU:
      {
        i = cgIns_NewU (o, arg1);
        break;
      }
    case INS_MODE_IS:
      {
        i = cgIns_NewS (o, arg1);
        break;
      }
    case INS_MODE_IBA:
      {
        i = cgIns_NewAB (o, arg1, arg2);
        break;
      }
    default:
      break;
    }

  /* lineinfo */
  {
    if (lex->lastline > func->lastline)
      {
        __losu_mem_growvector (func->vm, f->lineinfo, f->nlineinfo, 2, int32_t,
                               vmOl_MaxInt32_t,
                               __config_losucore_errmsg_msgVectorOverflow);
        if (lex->lastline > func->lastline + 1)
          f->lineinfo[f->nlineinfo++]
              = -(lex->lastline - (func->lastline + 1));
        f->lineinfo[f->nlineinfo++] = func->pc;
        func->lastline = lex->lastline;
      }
  }

  __losu_mem_growvector (func->vm, func->fcode->code, func->pc, 1,
                         vmInstruction, vmOl_MaxInt32_t,
                         __config_losucore_errmsg_msgVectorOverflow);
  func->fcode->code[func->pc] = i;
  return func->pc++;
}

static void
__losuSyntaxCgenConcat (_syntaxFunc *func, int32_t *l1, int32_t l2)
{
  if (*l1 == NO_JUMP)
    *l1 = l2;
  else
    {
      int32_t l = *l1;
      while (1)
        {
          int32_t n = __losuSyntaxCgenGetJump (func, l);
          if (n == NO_JUMP)
            {
              __losuSyntaxCgenFixJump (func, l, l2);
              return;
            }
          l = n;
        }
    }
}

static int32_t
__losuSyntaxCgenJump (_syntaxFunc *func)
{
  int32_t j = __losuSyntaxCgenCodearg1 (func, INS_JMP, NO_JUMP);
  if (j == func->lasttarget)
    {
      __losuSyntaxCgenConcat (func, &j, func->jump);
      func->jump = NO_JUMP;
    }
  return j;
}
static void
__losuSyntaxCgenPalist (_syntaxFunc *func, int32_t list, int32_t tg)
{
  if (tg == func->lasttarget)
    __losuSyntaxCgenConcat (func, &func->jump, list);
  else
    __losuSyntaxCgenPlistfunc (func, list, tg, INS_END, 0);
}
static int32_t
__losuSyntaxCgenGetL (_syntaxFunc *func)
{
  if (func->pc != func->lasttarget)
    {
      int32_t ltg = func->lasttarget;
      func->lasttarget = func->pc;
      __losuSyntaxCgenPalist (func, func->jump, ltg);
      func->jump = NO_JUMP;
    }
  return func->pc;
}
static void
__losuSyntaxCgenGoT (_syntaxFunc *func, _syntaxExp *v, int32_t keep)
{
  __losuSyntaxCgenGo (func, v, 1, keep ? INS_JMPONF : INS_JMPF);
}
static void
__losuSyntaxCgenGoF (_syntaxFunc *func, _syntaxExp *v, int32_t keep)
{
  __losuSyntaxCgenGo (func, v, 0, keep ? INS_JMPONT : INS_JMPT);
}
static void
__losuSyntaxCgenTostack (_syntaxLex *lex, _syntaxExp *v, int32_t o)
{
#define codelab(func, op, arg)                                                \
  (__losuSyntaxCgenGetL (func), __losuSyntaxCgenCodearg1 (func, op, arg))

  _syntaxFunc *func = lex->fs;
  if (!__losuSyntaxCgenDischarge (func, v))
    {
      vmIns_OP pre = cgIns_GetOp (func->fcode->code[func->pc - 1]);
      if (!vmIns_Isjump (pre) && v->value._bool.f == NO_JUMP
          && v->value._bool.t == NO_JUMP)
        {
          if (o)
            __losuSyntaxCgenSetNcallr (func, 1);
        }
      else
        {
          int32_t final;
          int32_t j = NO_JUMP;
          int32_t pn = NO_JUMP;
          int32_t pl = NO_JUMP;
          if (vmIns_Isjump (pre)
              || __losuSyntaxCgenNeedval (func, v->value._bool.f, INS_JMPONF)
              || __losuSyntaxCgenNeedval (func, v->value._bool.t, INS_JMPONT))
            {
              if (vmIns_Isjump (pre))
                __losuSyntaxCgenConcat (func, &v->value._bool.t, func->pc - 1);
              else
                {
                  j = codelab (func, INS_JMP, NO_JUMP);
                  __losuSyntaxCgenAdStack (func, 1);
                }
              pn = codelab (func, INS_PUSHNULLJMP, 0);
              pl = codelab (func, EC2INS_PUSHBOOL, 1);
              __losuSyntaxCgenPalist (func, j, __losuSyntaxCgenGetL (func));
            }
          final = __losuSyntaxCgenGetL (func);
          __losuSyntaxCgenPlistfunc (func, v->value._bool.f, pn, INS_JMPONF,
                                     final);
          __losuSyntaxCgenPlistfunc (func, v->value._bool.t, pl, INS_JMPONT,
                                     final);
          v->value._bool.f = v->value._bool.t = NO_JUMP;
        }
    }

#undef codelab
}
static void
__losuSyntaxCgenAdStack (_syntaxFunc *func, int32_t n)
{
  if (n > 0)
    __losuSyntaxCgenCodearg1 (func, INS_POP, n);
  else
    __losuSyntaxCgenCodearg1 (func, INS_PUSHNULL, -n);
}
static void
__losuSyntaxCgenSetVar (_syntaxLex *lex, _syntaxExp *var)
{
  _syntaxFunc *func = lex->fs;
  switch (var->type)
    {
    case VL:
      __losuSyntaxCgenCodearg1 (func, INS_SETLOCAL, var->value.index);
      break;
    case VG:
      __losuSyntaxCgenCodearg1 (func, INS_SETGLOBAL, var->value.index);
      break;
    case VI:
      __losuSyntaxCgenCodearg2 (func, INS_SETUNIT, 3, 3);
      break;
    default:
      break;
    }
}
static _l_bool
__losuSyntaxCgenIsopen (_syntaxFunc *func)
{
  vmInstruction i = func->pc > func->lasttarget
                        ? func->fcode->code[func->pc - 1]
                        : ((vmInstruction)INS_END);
  if (cgIns_GetOp (i) == INS_CALL && cgIns_GetB (i) == 255)
    return 1;
  else
    return 0;
}
static void
__losuSyntaxCgenSetNcallr (_syntaxFunc *func, int32_t nres)
{
  if (__losuSyntaxCgenIsopen (func))
    {
      cgIns_SetB (func->fcode->code[func->pc - 1], nres);
      __losuSyntaxCgenDtStack (func, nres);
    }
}
static void
__losuSyntaxCgenNumber (_syntaxFunc *func, _l_number n)
{
  _inlineScode *fc = func->fcode;
  int32_t c = fc->nlcnum;
  /*
    int lim = c < LOOKBACKNUMS ? 0 : c - LOOKBACKNUMS;
    while (--c >= lim)
  */
  int32_t lim = c < vmOl_MaxNumberRef ? 0 : c - vmOl_MaxNumberRef;
  while (--c >= lim)
    if (fc->lcnum[c] == n)
      {
        __losuSyntaxCgenCodearg1 (func, INS_PUSHNUM, c);
        return;
      }

  __losu_mem_growvector (func->vm, fc->lcnum, fc->nlcnum, 1, _l_number,
                         vmIns_MaxU,
                         __config_losucore_errmsg_msgVectorOverflow);
  c = fc->nlcnum++;
  fc->lcnum[c] = n;
  __losuSyntaxCgenCodearg1 (func, INS_PUSHNUM, c);
}

static void
__losuSyntaxCgenDtStack (_syntaxFunc *func, int32_t dt)
{
  func->stacklevel += dt;
  if (func->stacklevel > func->fcode->maxstacksize)
    {
      if (func->stacklevel > vmOl_MaxStack)
        __losuSyntaxError (func->lexer,
                           __config_losucore_errmsg_msgTooComplexExp);
      func->fcode->maxstacksize = func->stacklevel;
    }
}
static void
__losuSyntaxCgenPrefix (_syntaxLex *lex, vmIns_UnOp op, _syntaxExp *v)
{
#define swap(v)                                                               \
  {                                                                           \
    int32_t tmp = v->value._bool.f;                                           \
    v->value._bool.f = v->value._bool.t;                                      \
    v->value._bool.t = tmp;                                                   \
  }

  _syntaxFunc *func = lex->fs;
  if (op == INS_UNOP_NEG)
    {
      __losuSyntaxCgenTostack (lex, v, 1);
      __losuSyntaxCgenCodearg0 (func, INS_NEG);
    }
  else if (op == INS_UNOP_BITNOT)
    {
      __losuSyntaxCgenTostack (lex, v, 1);
      __losuSyntaxCgenCodearg0 (func, EC2INS_BITNOT);
    }
  else
    {
      vmInstruction *pre;
      __losuSyntaxCgenDischarge (func, v);
      if (v->value._bool.t == NO_JUMP && v->value._bool.f == NO_JUMP)
        __losuSyntaxCgenSetNcallr (func, 1);

      pre = &(func->fcode->code[func->pc - 1]);
      if (vmIns_Isjump (cgIns_GetOp ((*pre))))
        cgIns_SetOp ((*pre),
                     __losuSyntaxCgenJumpInvert (cgIns_GetOp ((*pre))));
      else
        __losuSyntaxCgenCodearg0 (func, INS_NOT);

      swap (v);
    }

#undef swap
}
static void
__losuSyntaxCgenInfix (_syntaxLex *lex, vmIns_BinOp op, _syntaxExp *v)
{
  _syntaxFunc *func = lex->fs;
  switch (op)
    {
    case INS_BINOP_AND:
      {
        __losuSyntaxCgenGoT (func, v, 1);
        break;
      }
    case INS_BINOP_OR:
      {
        __losuSyntaxCgenGoF (func, v, 1);
        break;
      }
    default:
      {
        __losuSyntaxCgenTostack (lex, v, 1);
        break;
      }
    }
}
static void
__losuSyntaxCgenPosfix (_syntaxLex *lex, vmIns_BinOp op, _syntaxExp *v1,
                        _syntaxExp *v2)
{
  _syntaxFunc *func = lex->fs;
  switch (op)
    {
    case INS_BINOP_AND:
      {
        __losuSyntaxCgenDischarge (func, v2);
        if (v2->value._bool.t == NO_JUMP && v2->value._bool.f == NO_JUMP)
          __losuSyntaxCgenSetNcallr (func, 1);
        v1->value._bool.t = v2->value._bool.t;
        __losuSyntaxCgenConcat (func, &v1->value._bool.f, v2->value._bool.f);
        break;
      }
    case INS_BINOP_OR:
      {
        __losuSyntaxCgenDischarge (func, v2);
        if (v2->value._bool.t == NO_JUMP && v2->value._bool.f == NO_JUMP)
          __losuSyntaxCgenSetNcallr (func, 1);
        v1->value._bool.f = v2->value._bool.f;
        __losuSyntaxCgenConcat (func, &v1->value._bool.t, v2->value._bool.t);
        break;
      }
    default:
      {
        __losuSyntaxCgenTostack (lex, v2, 1);
        __losuSyntaxCgenCodearg1 (func, _alops[op].op, _alops[op].arg);
        break;
      }
    }
}

static int32_t
__losuSyntaxCgenGetJump (_syntaxFunc *func, int32_t pc)
{
  int32_t o = cgIns_GetS (func->fcode->code[pc]);
  if (o == NO_JUMP)
    return NO_JUMP;
  else
    return (pc + 1) + o;
}
static void
__losuSyntaxCgenFixJump (_syntaxFunc *func, int32_t pc, int32_t dt)
{
  vmInstruction *jmp = &func->fcode->code[pc];
  if (dt == NO_JUMP)
    cgIns_SetS ((*jmp), NO_JUMP);
  else
    {
      int32_t o = dt - (pc + 1);
      if (abs (o) > vmIns_MaxS)
        __losuSyntaxError (func->lexer,
                           __config_losucore_errmsg_msgInvalidJump);
      cgIns_SetS ((*jmp), o);
    }
}
static void
__losuSyntaxCgenPlistfunc (_syntaxFunc *func, int32_t list, int32_t tg,
                           vmIns_OP ins, int32_t instg)
{
  vmInstruction *code = func->fcode->code;
  while (list != NO_JUMP)
    {
      int32_t next = __losuSyntaxCgenGetJump (func, list);
      vmInstruction *i = &code[list];
      vmIns_OP op = cgIns_GetOp ((*i));
      if (op == ins)
        __losuSyntaxCgenFixJump (func, list, instg);
      else
        {
          __losuSyntaxCgenFixJump (func, list, tg);
          if (op == INS_JMPONT)
            cgIns_SetOp ((*i), INS_JMPT);
          else if (op == INS_JMPONF)
            cgIns_SetOp ((*i), INS_JMPF);
        }
      list = next;
    }
}
static void
__losuSyntaxCgenGo (_syntaxFunc *func, _syntaxExp *v, int32_t inv,
                    vmIns_OP jump)
{
  int32_t prepc;
  vmInstruction *pre;
  int32_t *golist, *exitlist;
  if (!inv)
    {
      golist = &v->value._bool.f;
      exitlist = &v->value._bool.t;
    }
  else
    {
      golist = &v->value._bool.t;
      exitlist = &v->value._bool.f;
    }
  __losuSyntaxCgenDischarge (func, v);
  if (v->value._bool.t == NO_JUMP && v->value._bool.f == NO_JUMP)
    __losuSyntaxCgenSetNcallr (func, 1);
  prepc = func->pc - 1;
  pre = &func->fcode->code[prepc];
  if (!vmIns_Isjump (cgIns_GetOp (*pre)))
    prepc = __losuSyntaxCgenCodearg1 (func, jump, NO_JUMP);
  else if (inv)
    cgIns_SetOp (*pre, __losuSyntaxCgenJumpInvert (cgIns_GetOp (*pre)));
  __losuSyntaxCgenConcat (func, exitlist, prepc);
  __losuSyntaxCgenPalist (func, *golist, __losuSyntaxCgenGetL (func));
  *golist = NO_JUMP;
}
static _l_bool
__losuSyntaxCgenDischarge (_syntaxFunc *func, _syntaxExp *v)
{
  switch (v->type)
    {
    case VL:
      {
        __losuSyntaxCgenCodearg1 (func, INS_GETLOCAL, v->value.index);
        break;
      }
    case VG:
      {
        __losuSyntaxCgenCodearg1 (func, INS_GETGLOBAL, v->value.index);
        break;
      }
    case VI:
      {
        __losuSyntaxCgenCodearg0 (func, INS_GETUNIT);
        break;
      }
    case VE:
      {
        return 0;
      }
    default:
      break;
    }
  v->type = VE;
  v->value._bool.t = v->value._bool.f = NO_JUMP;
  return 1;
}
static _l_bool
__losuSyntaxCgenNeedval (_syntaxFunc *func, int32_t l, vmIns_OP op)
{
  for (; l != NO_JUMP; l = __losuSyntaxCgenGetJump (func, l))
    if (cgIns_GetOp (func->fcode->code[l]) != op)
      return 1;
  return 0;
}
static vmIns_OP
__losuSyntaxCgenJumpInvert (vmIns_OP op)
{
  switch (op)
    {
    case INS_JMPNE:
      return INS_JMPEQ;
    case INS_JMPEQ:
      return INS_JMPNE;

    case INS_JMPLT:
      return INS_JMPGE;
    case INS_JMPLE:
      return INS_JMPGT;

    case INS_JMPGT:
      return INS_JMPLE;
    case INS_JMPGE:
      return INS_JMPLT;

    case INS_JMPT:
    case INS_JMPONT:
      return INS_JMPF;

    case INS_JMPF:
    case INS_JMPONF:
      return INS_JMPT;

    default:
      return INS_END;
    }
}

#undef NO_JUMP

#endif /* DEFINE_SOURCE_LOSU_SYNTAX_C_CODEGEN */

#ifndef DEFINE_SOURCE_LOSU_SYNTAX_C_IRLOAD
#define DEFINE_SOURCE_LOSU_SYNTAX_C_IRLOAD

#define EOZ (-1)

_inlineScode *
__losu_syntaxIrload_load (LosuVm *vm, _syntaxIO *io)
{
  return NULL;
}

#undef EOZ

#endif

#endif
