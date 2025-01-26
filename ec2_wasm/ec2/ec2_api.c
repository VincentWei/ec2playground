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
#include "losu_api.h"
#include "losu_bytecode.h"
#include "losu_errmsg.h"
#include "losu_gc.h"
#include "losu_malloc.h"
#include "losu_object.h"
#include "losu_syntax.h"
#include "losu_vm.h"
#include <setjmp.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// vm & gc APIs
#ifndef VMGC_APIS
#define VMGC_APIS
LosuExtern LosuVm *
vm_create (int32_t size)
{
  __losuvmJmp lj = {
    .func = NULL,
  };
  if (size <= 0) /* size not right */
    return NULL;
  LosuVm *vm = __losu_mem_new (NULL, LosuVm);
  if (vm == NULL) /* don't new */
    return NULL;
  *vm = (LosuVm)
  {
    .stack = NULL,
    .top = NULL,
    .stackmax = NULL,
    .base = NULL,
    .mstack = NULL,
    .mtop = NULL,
    .mstackmax = NULL,

    .errjmp = &lj,

    .bufftmp = NULL,
    .nbufftmp = 0,

    .inspool = NULL,
    .funcpool = NULL,
    .hashpool = NULL,
    .strpool = (__losuvmStrseg){
        .size = 0,
        .nsize = 0,
        .strobj = NULL,
    },
    .callpool = NULL,
    .coropool = NULL,
    .nblocks = sizeof(LosuVm),
    
    .gcDymax = 1,
    .gcMax = SIZE_MAX,
    .gcHook = 0,
    
    .loophook = 0,
    .callhook = 0,
    .aluhook = 0,
    // .deephook = 0,

    .name = NULL, 
    .main = {
      .funcargs = { NULL },
      .funcname = NULL,
      .main = NULL,
    },
    .global_symbol = NULL,
    .ng_symbol = 0,
    .global_value = NULL,
    .ng_value = 0,

  };

  int32_t jsta = setjmp (lj.jmpflag);
  switch (jsta)
    {
    case VMSIGOK: /* ok */
      {
        vm->global = __losu_objUnit_new (vm, 0);
        __losu_vmHeap_init (vm, size);
        __losu_objStringPool_init (vm);
        __losu_syntaxLex_init (vm);
        break;
      }
    case VMSIGERR: /* error */
      {
        vm_close (vm);
        return NULL;
        break;
      }
    default: /* other */
      break;
    }

  vm->errjmp = NULL;
  return vm;
}

LosuExtern int vm_init (LosuVm *vm, LosuPackage_t *pkgs_t, int32_t argc,
                        const char **argv);

LosuExtern void
vm_stop (LosuVm *vm, const char *estr, ...)
{
  va_list ap;
  va_start (ap, estr);
  vsprintf ((char *)(vm->staticbuff), estr, ap);
  fprintf (stderr, "Core Panic: %s\nAt line %d of '%1s'\n",
           (char *)vm->staticbuff, __losuApigetline (vm, vm->top - 1),
           vm->name != NULL ? vm->name : "<unknown>");
  va_end (ap);
  __losu_sigThrow (vm, VMSIGKILL);
}
LosuExtern void
vm_error (LosuVm *vm, const char *estr, ...)
{

  va_list ap;
  va_start (ap, estr);
  vsprintf ((char *)(vm->staticbuff), estr, ap);
  fprintf (stderr, "❗️运行时错误: %s\n在第 %d 行\n", (char *)vm->staticbuff,
           __losuApigetline (vm, vm->top - 1));
  va_end (ap);
  __losu_sigThrow (vm, VMSIGERR);
}
LosuExtern int32_t
vm_dofile (LosuVm *vm, const char *f)
{
  int32_t i;
  if (!(i = vm_loadfile (vm, f)))
    return vm_execute (vm, 0, -1, f);
  return i;
}
LosuExtern int32_t
vm_dostring (LosuVm *vm, const char *s, const char *n)
{
  int32_t i;
  if (!(i = vm_loadstring (vm, s, n)))
    return vm_execute (vm, 0, -1, n);
  return i;
}
LosuExtern int32_t
vm_dobyte (LosuVm *vm, const char *byte, size_t len, const char *name)
{
  int32_t i;
  if (!(i = vm_loadbyte (vm, byte, len, name)))
    return vm_execute (vm, 0, -1, name);
  return i;
}
int32_t
vm_loadfile (LosuVm *vm, const char *f)
{
  return __losu_syntaxIO_loadfile (vm, f);
}
LosuExtern int32_t
vm_loadstring (LosuVm *vm, const char *s, const char *name)
{
  return __losu_syntaxIO_loadstring (vm, s, strlen (s), name);
}
LosuExtern int32_t
vm_loadbyte (LosuVm *vm, const char *byte, size_t len, const char *name)
{
  return __losu_syntaxIO_loadstring (vm, byte, len, name);
}
int32_t
vm_execute (LosuVm *vm, int32_t narg, int32_t nres, const char *name)
{

  /* int32_t i = __losu_vmHeap_callS (vm, narg, nres); */
  int32_t i = 0;
  {
    LosuObj *func = (vm->top - 1) - narg;
    LosuObj *oldbase = vm->base;
    LosuObj *oldtop = vm->top;
    __losuvmJmp *oldjmp = vm->errjmp;
    const char *oldname = vm->name;

    __losuvmJmp jmp = {
      .func = NULL,
    };

    vm->name = name;
    vm->errjmp = &jmp;

    int32_t jsta = setjmp (jmp.jmpflag);
    switch (jsta)
      {
      case VMSIGOK: /* ok */
        {
          __losu_vmHeap_rawcall (vm, func, nres);
          break;
        }
      case VMSIGERR: /* error */
        {
          vm->base = oldbase;
          vm->top = oldtop;
          vm->name = oldname;
          vm->errjmp = oldjmp;
          break;
        }
      case VMSIGYIELD: /* yield */
        {
          vm->base = oldbase;
          vm->top = oldtop;
          vm->errjmp = oldjmp;
          __losu_sigThrow (vm, VMSIGYIELD);
          vm->name = oldname;
          break;
        }
      case VMSIGKILL:
        {

          vm->errjmp = oldjmp;
          __losu_sigThrow (vm, VMSIGKILL);
          vm->name = oldname;
          break;
        }
      default:
        break;
      }
    i = jsta;
  }
  return i;
}

LosuExtern LosuObj *
vm_getval (LosuVm *vm, const char *name)
{
  return (LosuObj *)__losu_objUnit_getstr (vm->global,
                                           __losu_objString_new (vm, name));
}

LosuExtern void
vm_setval (LosuVm *vm, const char *name, LosuObj val)
{
  LosuObj key = (LosuObj){
    .type = LosuTypeDefine_string,
    .value.str = __losu_objString_new (vm, name),
  };
  *__losu_objUnit_set (vm, vm->global, &key) = val;
  vm_addgsymbol (vm, name);
  vm_addgvalue (vm, name);
}

LosuExtern void
vm_addgsymbol (LosuVm *vm, const char *name)
{
  // search is exsist?
  for (int32_t i = 0; i < vm->ng_symbol; i++)
    if (strcmp (vm->global_symbol[i], name) == 0)
      return;
  // create new symbol
  vm->global_symbol = __losu_mem_realloc (
      vm, vm->global_symbol, (vm->ng_symbol + 1) * sizeof (char *));
  vm->global_symbol[vm->ng_symbol++]
      = __losu_mem_malloc (vm, strlen (name) + 1);
  memcpy (vm->global_symbol[vm->ng_symbol - 1], name, strlen (name) + 1);
}

LosuExtern void
vm_addgvalue (LosuVm *vm, const char *name)
{
  // search is exsist?
  for (int32_t i = 0; i < vm->ng_value; i++)
    if (strcmp (vm->global_value[i], name) == 0)
      return;
  // create new value
  vm->global_value = __losu_mem_realloc (vm, vm->global_value,
                                         (vm->ng_value + 1) * sizeof (char *));
  vm->global_value[vm->ng_value++] = __losu_mem_malloc (vm, strlen (name) + 1);
  memcpy (vm->global_value[vm->ng_value - 1], name, strlen (name) + 1);
}

LosuExtern void
vm_close (LosuVm *vm)
{
  __losu_gc_collect (vm, 1);
  __losu_objStringPool_deinit (vm);

  if (vm->mstack)
    {
      __losu_mem_free (vm, vm->mstack);
      vm->nblocks -= (vm->mstackmax - vm->mstack + 1) * sizeof (LosuObj);
    }
  if (vm->bufftmp)
    {
      __losu_mem_free (vm, vm->bufftmp);
      vm->nblocks -= vm->nbufftmp * sizeof (char);
    }
  if (vm->main.main)
    __losu_mem_free (vm, vm->main.main);
  __losu_mem_free (vm, vm);
}
LosuExtern void
gc_setmax (LosuVm *vm, _l_gcint size)
{
  vm->gcMax = vm->gcDymax = size;
}
LosuExtern _l_gcint
gc_getmemNow (LosuVm *vm)
{
  return vm->nblocks;
}
LosuExtern _l_gcint
gc_getmemMax (LosuVm *vm)
{
  return vm->gcHook;
}
LosuExtern _l_bool
gc_collect (LosuVm *vm)
{
  return __losu_gc_checkClt (vm);
}

#endif

// obj APIs
#ifndef OBJ_APIS
#define OBJ_APIS
uint8_t
charset_utf8len (uint8_t c)
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

// obj APIs

LosuExtern int32_t
obj_type (LosuVm *vm, LosuObj *obj)
{
  return ovtype (obj);
}

LosuExtern const char *
obj_typeStr (LosuVm *vm, LosuObj *obj)
{
  const char *typeStr[16] = {
    "未定义", "浮点数", "整数", "字符", "字节", "字符串", "函数",
    "unit",   "空",     "布尔", "携程", "调用", "未知",
  };
  int32_t t = ovtype (obj);
  switch (t)
    {
    case LosuTypeDefine_unit:
      {
        if (ovhash (obj)->isMap)
          return "映射";
        else
          return "序列";
      }
    case LosuTypeDefine_bool:
      {
        return ovbool (obj) ? "真" : "假";
      }
    default:
      {
        return typeStr[t <= LosuTypeDefine_unknown ? t
                                                   : LosuTypeDefine_unknown];
      }
    }
}

LosuExtern LosuObj
obj_newnull (LosuVm *vm)
{
  return _inlineNullObj;
}

LosuExtern LosuObj
obj_newnum (LosuVm *vm, _l_number num)
{
  return (LosuObj){
    .type = LosuTypeDefine_number,
    .value.num = num,
  };
}

LosuExtern _l_number
obj_tonum (LosuVm *vm, LosuObj *obj)
{
  switch (ovtype (obj))
    {
    case LosuTypeDefine_number:
      return ovnumber (obj);
    case LosuTypeDefine_int:
      return ovint (obj);
    case LosuTypeDefine_unicode:
      return ovunicode (obj);
    case LosuTypeDefine_string:
      return atof (ovSstr (obj));
    case LosuTypeDefine_bool:
      return ovbool (obj);
    case LosuTypeDefine_char:
      return (_l_number)ovchar (obj);
    default:
      vm_error (vm, " '%s' 不可被视作数字类型", obj_typeStr (vm, obj));
      return 0;
    }
}
LosuExtern LosuObj
obj_newint (LosuVm *vm, _l_int b)
{
  return (LosuObj){
    .type = LosuTypeDefine_int,
    .value.intnum = b,
  };
}
LosuExtern _l_int
obj_toint (LosuVm *vm, LosuObj *obj)
{
  switch (ovtype (obj))
    {
    case LosuTypeDefine_number:
      return (_l_int)ovnumber (obj);
    case LosuTypeDefine_int:
      return ovint (obj);
    case LosuTypeDefine_unicode:
      return (_l_int)ovunicode (obj);
    case LosuTypeDefine_string:
      return (_l_int)atoll (ovSstr (obj));
    case LosuTypeDefine_bool:
      return ovbool (obj);
    case LosuTypeDefine_char:
      return (_l_int)ovchar (obj);
    default:
      vm_error (vm, " '%s' 不可被视作整数类型", obj_typeStr (vm, obj));
      return 0;
    }
}
LosuExtern LosuObj
obj_newunicode (LosuVm *vm, _l_unicode b)
{
  return (LosuObj){
    .type = LosuTypeDefine_unicode,
    .value.unicode = b,
  };
}
LosuExtern _l_unicode
obj_tounicode (LosuVm *vm, LosuObj *obj)
{
  switch (ovtype (obj))
    {
    case LosuTypeDefine_number:
      return (_l_unicode)ovnumber (obj);
    case LosuTypeDefine_int:
      return (_l_unicode)ovint (obj);
    case LosuTypeDefine_unicode:
      return (_l_unicode)ovunicode (obj);
    case LosuTypeDefine_string:
      {
        uint8_t len = charset_utf8len (ovSstr (obj)[0]);
        _l_unicode unicode = 0;
        for (uint8_t i = 0; i < len; i++)
          unicode = (unicode << 8) + (uint8_t)ovSstr (obj)[i];
        return unicode;
      }
    case LosuTypeDefine_bool:
      return ovbool (obj);
    case LosuTypeDefine_char:
      return (_l_unicode)ovchar (obj);
    default:
      vm_error (vm, " '%s' 不可被视作字符类型", obj_typeStr (vm, obj));
      return 0;
    }
}

LosuExtern LosuObj
obj_newchar (LosuVm *vm, uint8_t b)
{
  return (LosuObj){
    .type = LosuTypeDefine_char,
    .value._char = b,
  };
}
LosuExtern uint8_t
obj_tochar (LosuVm *vm, LosuObj *obj)
{
  switch (ovtype (obj))
    {
    case LosuTypeDefine_number:
      return (uint8_t)ovnumber (obj);
    case LosuTypeDefine_int:
      return (uint8_t)ovint (obj);
    case LosuTypeDefine_unicode:
      return (uint8_t)ovunicode (obj);
    case LosuTypeDefine_string:
      return (uint8_t)atoll (ovSstr (obj));
    case LosuTypeDefine_bool:
      return (uint8_t)ovbool (obj);
    case LosuTypeDefine_char:
      return ovchar (obj);
    default:
      vm_error (vm, " '%s' 不可被视作字节类型", obj_typeStr (vm, obj));
      return 0;
    }
}

LosuExtern LosuObj
obj_newstr (LosuVm *vm, char *str)
{
  return (LosuObj){
    .type = LosuTypeDefine_string,
    .value.str = __losu_objString_newstr (vm, str, strlen (str)),
  };
}
LosuExtern const char *
obj_tostr (LosuVm *vm, LosuObj *obj)
{
  char stmp[64] = { 0 };
  switch (ovtype (obj))
    {
    case LosuTypeDefine_null:
      {
        return "未定义";
      }
    case LosuTypeDefine_number:
      {
        sprintf (stmp, "%.16g", ovnumber (obj));
        return (const char *)__losu_objString_new (vm, stmp)->str;
      }
    case LosuTypeDefine_int:
      {
        sprintf (stmp, "%d", ovint (obj));
        return (const char *)__losu_objString_new (vm, stmp)->str;
      }
    case LosuTypeDefine_unicode:
      {
        _l_unicode uchar = ovunicode (obj);
        char u[5] = { 0 };
        char *u_t = (char *)&uchar;
        for (int i = 0, j = 0; i < 4; i++)
          {
            // printf ("debug(2):%d\n", u_t[3-i]);
            if (u_t[3 - i] != '\0')
              u[j++] = u_t[3 - i];
          }
        // printf ("debug(1):%u\n", uchar);
        return (const char *)__losu_objString_new (vm, u)->str;
      }
    case LosuTypeDefine_string:
      {
        return (const char *)ovSstr (obj);
      }
    case LosuTypeDefine_bool:
      {
        return ovbool (obj) ? "真" : "假";
      }
    case LosuTypeDefine_space:
      {
        return "空";
      }
    case LosuTypeDefine_unit:
      {
        char tmp[1024] = { 0 };
        if (ovhash (obj)->isMap != 0) // if Map,return 0;
          vm_error (vm, " '%s' 不可被视作字符串", obj_typeStr (vm, obj));
        int i = 0;
        while (1)
          {
            LosuObj *s = obj_indexunitbynum (vm, *obj, i++);
            if (ovtype (s) == LosuTypeDefine_null)
              break;
            sprintf (tmp, "%s%s", tmp, obj_tostr (vm, s));
          }
        return (const char *)__losu_objString_new (vm, tmp)->str;
        break;
      }
    case LosuTypeDefine_char:
      {
        sprintf (stmp, "%c", ovchar (obj));
        return (const char *)__losu_objString_new (vm, stmp)->str;
      }
    default:
      vm_error (vm, " '%s' 不可被视作字符串", obj_typeStr (vm, obj));
      return "";
    }
}

LosuExtern const char *
unit2str (LosuVm *vm, LosuObj *obj)
{
  char tmp[1024] = { 0 };
  if (ovhash (obj)->isMap) // 映射
    {
      LosuNode *n = obj_unit_first (vm, *obj);
      sprintf (tmp, "{ ");
      while (n)
        {
          LosuObj *key = &n->key;
          LosuObj *value = &n->value;
          switch (ovtype (key))
            {
            case LosuTypeDefine_string:
              {
                sprintf (tmp, "%s\"%s\": ", tmp, obj_tostr (vm, key));
                break;
              }
            default:
              {
                sprintf (tmp, "%s[%s]: ", tmp, obj_tostr (vm, key));
                break;
              }
            }
          switch (ovtype (value))
            {
            case LosuTypeDefine_string:
              {
                sprintf (tmp, "%s\"%s\"", tmp, obj_tostr (vm, value));
                break;
              }
            case LosuTypeDefine_unicode:
              {
                sprintf (tmp, "%s''%s''", tmp, obj_tostr (vm, value));
                break;
              }
            case LosuTypeDefine_char:
              {
                sprintf (tmp, "%s'%s'", tmp, obj_tostr (vm, value));
                break;
              }
            default:
              {
                sprintf (tmp, "%s%s", tmp, obj_tostr (vm, value));
                break;
              }
            }
          n = obj_unit_next (vm, *obj, n);
          if (n)
            sprintf (tmp, "%s, ", tmp);
        }
      sprintf (tmp, "%s}", tmp);
      return (const char *)__losu_objString_new (vm, tmp)->str;
    }
  else // 序列
    {
      int i = 0;
      sprintf (tmp, "[");
      LosuObj *n = obj_indexunitbynum (vm, *obj, i++);
      while (1)
        {
          if (ovtype (n) == LosuTypeDefine_null)
            break;
          switch (ovtype (n))
            {
            case LosuTypeDefine_string:
              {
                sprintf (tmp, "%s\"%s\"", tmp, obj_tostr (vm, n));
                break;
              }
            case LosuTypeDefine_unicode:
              {
                sprintf (tmp, "%s''%s''", tmp, obj_tostr (vm, n));
                break;
              }
            case LosuTypeDefine_char:
              {
                sprintf (tmp, "%s'%s'", tmp, obj_tostr (vm, n));
                break;
              }
            default:
              {
                sprintf (tmp, "%s%s", tmp, obj_tostr (vm, n));
                break;
              }
            }
          n = obj_indexunitbynum (vm, *obj, i++);
          if (ovtype (n) != LosuTypeDefine_null)
            sprintf (tmp, "%s, ", tmp);
        }
      sprintf (tmp, "%s]", tmp);
      return (const char *)__losu_objString_new (vm, tmp)->str;
    }
}

LosuExtern LosuObj
obj_newfunction (LosuVm *vm, LosuApi func)
{
  _inlineFunction *f = __losu_objFunc_new (vm, 0);
  f->isC = 1;
  f->func.capi = func;
  f->isMain = 0;
  return (LosuObj){
    .type = LosuTypeDefine_function,
    .value.func = f,
  };
}
LosuExtern LosuApi
obj_tofunction (LosuVm *vm, LosuObj *obj)
{
  return ovtype (obj) == LosuTypeDefine_function ? ovfunc (obj)->func.capi
                                                 : NULL;
}

LosuExtern LosuObj
obj_newunit (LosuVm *vm, _l_bool isMap)
{
  _inlineHash *h = __losu_objUnit_new (vm, 0);
  h->isMap = isMap;
  return (LosuObj){
    .type = LosuTypeDefine_unit,
    .value.hash = h,
  };
}

LosuExtern LosuObj *
obj_indexunit (LosuVm *vm, LosuObj unit, LosuObj key)
{
  return ovtype ((&unit)) == LosuTypeDefine_unit
             ? (LosuObj *)__losu_objUnit_get (ovhash (&unit), &key)
             : (LosuObj *)&_inlineNullObj;
}

LosuExtern LosuObj *
obj_indexunitbynum (LosuVm *vm, LosuObj unit, _l_number i)
{
  return ovtype ((&unit)) == LosuTypeDefine_unit
             ? (LosuObj *)__losu_objUnit_getnum (ovhash (&unit), i)
             : (LosuObj *)&_inlineNullObj;
}

LosuExtern LosuObj *
obj_indexunitbystr (LosuVm *vm, LosuObj unit, char *s)
{
  return ovtype ((&unit)) == LosuTypeDefine_unit
             ? (LosuObj *)__losu_objUnit_getstr (
                   ovhash (&unit), __losu_objString_newstr (vm, s, -1))
             : (LosuObj *)&_inlineNullObj;
}

LosuExtern void
obj_setunit (LosuVm *vm, LosuObj unit, LosuObj key, LosuObj value)
{
  if (ovtype ((&unit)) == LosuTypeDefine_unit)
    *__losu_objUnit_set (vm, ovhash ((&unit)), &key) = value;
}
LosuExtern void
obj_setunitbynum (LosuVm *vm, LosuObj unit, _l_number key, LosuObj value)
{
  if (ovtype ((&unit)) == LosuTypeDefine_unit)
    *__losu_objUnit_setnum (vm, ovhash ((&unit)), key) = value;
}
LosuExtern void
obj_setunitbystr (LosuVm *vm, LosuObj unit, char *key, LosuObj value)
{
  if (ovtype ((&unit)) == LosuTypeDefine_unit)
    *__losu_objUnit_setstr (vm, ovhash ((&unit)),
                            __losu_objString_newconst (vm, key))
        = value;
}
LosuExtern LosuNode *
obj_unit_first (LosuVm *vm, LosuObj unit)
{
  if (ovtype ((&unit)) == LosuTypeDefine_unit)
    return (LosuNode *)__losu_objUnit_getnext (ovhash ((&unit)),
                                               (LosuObj *)&_inlineNullObj);
  return NULL;
}
LosuExtern LosuNode
obj_unit_location (LosuVm *vm, LosuObj unit, LosuObj key)
{
  LosuNode n;
  n.key = key;
  if (ovtype ((&unit)) == LosuTypeDefine_unit)
    n.value = *__losu_objUnit_get (ovhash ((&unit)), &key);
  else
    n.value = _inlineNullObj;
  return n;
}
LosuExtern LosuNode *
obj_unit_next (LosuVm *vm, LosuObj unit, LosuNode *n)
{
  if (ovtype ((&unit)) == LosuTypeDefine_unit)
    return (LosuNode *)__losu_objUnit_getnext (ovhash ((&unit)), (&(n->key)));
  return NULL;
}
LosuExtern LosuObj
obj_unit_nodekey (LosuVm *vm, LosuNode *n)
{
  return n->key;
}
LosuExtern LosuObj
obj_unit_nodevalue (LosuVm *vm, LosuNode *n)
{
  return n->value;
}

LosuExtern LosuObj
obj_newspace (LosuVm *vm)
{
  return _inlineSpaceObj;
}

LosuExtern LosuObj
obj_newbool (LosuVm *vm, _l_bool b)
{
  return b ? _inlineTrueObj : _inlineFalseObj;
}
LosuExtern _l_bool
obj_tobool (LosuVm *vm, LosuObj *obj)
{
  switch (ovtype (obj))
    {
    case LosuTypeDefine_null:
      return 0;
    case LosuTypeDefine_bool:
      return ovbool (obj);
    default:
      return 1;
    }
}

#endif

// arg APIs
#ifndef ARG_APIS
#define ARG_APIS

LosuExtern int32_t
arg_num (LosuVm *vm)
{
  return (int32_t)(vm->top - vm->base);
}
LosuExtern LosuObj *
arg_get (LosuVm *vm, int idx)
{
  if (idx > 0)
    {
      LosuObj *o = vm->base + (idx - 1);
      if (o >= vm->top)
        ovtype (o) = LosuTypeDefine_null;
      return o;
    }
  return (LosuObj *)(&_inlineNullObj);
}
LosuExtern void
arg_return (LosuVm *vm, LosuObj obj)
{
  *(vm->top) = (obj);
  if (vm->top == vm->stackmax)
    vm_error (vm, __config_losucore_errmsg_msgStackOverflow);
  vm->top++;
}
#endif

#ifndef STACK_APIS
#define STACK_APIS
LosuExtern void
stack_push (LosuVm *vm, LosuObj o)
{
  *(vm->top) = o;
  if (vm->top == vm->stackmax)
    vm_error (vm, __config_losucore_errmsg_msgStackOverflow);
  vm->top++;
}
LosuExtern void
stack_pop (LosuVm *vm, int32_t i)
{
  vm->top -= i;
  if (vm->top < vm->stack)
    vm->top = vm->stack;
}
LosuExtern void
stack_call (LosuVm *vm, int32_t argnum, int32_t resnum)
{
  __losu_vmHeap_rawcall (vm, vm->top - (argnum + 1), resnum);
}

#endif

#ifndef STATIC_SEG
#define STATIC_SEG
static int32_t
__losuApigetline (LosuVm *vm, LosuObj *obj)
{
  while (1)
    {
      /* printf ("stack:%p\t%p\n", vm->stack, obj); */
      if (!(obj && ovtype (obj) == LosuTypeDefine_callinfo
            && !ovcall (obj)->func->isC))
        obj--;
      else
        break;
      if (obj < vm->stack)
        return -1;
    }

  return __losuApitryline (ovcall (obj)->func->func.sdef->lineinfo,
                           __losuApicurrentpc (obj));
}

static int32_t
__losuApicurrentpc (LosuObj *obj)
{

  if (ovcall (obj)->pc)
    return ((ovcall (obj)->pc) - ovcall (obj)->func->func.sdef->code) - 1;
  else
    return -1;
}
static int32_t
__losuApitryline (int32_t *lineinfo, int32_t pc)
{

  int32_t refi = 0;
  int32_t refline = 1;
  if (lineinfo == NULL || pc == -1)
    return -1;
  if (lineinfo[refi] < 0)
    refline += -lineinfo[refi++];
  while (lineinfo[refi] > pc)
    {
      refline--;
      refi--;
      if (lineinfo[refi] < 0)
        refline -= -lineinfo[refi++];
    }

  while (1)
    {
      int32_t nextline = refline + 1;
      int32_t nexref = refi + 1;
      if (lineinfo[nexref] < 0)
        nextline += -lineinfo[nexref++];
      if (lineinfo[nexref] > pc)
        break;
      refline = nextline;
      refi = nexref;
    }

  return refline;
}

#endif