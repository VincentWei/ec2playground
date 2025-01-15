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

/**
 * An ultra-lightweight stacked virtual machine
 * consisting of vmHeap,vmCore
 */

#ifndef DEFINE_SOURCE_LOSU_VM_C
#define DEFINE_SOURCE_LOSU_VM_C

#include "losu.h"
#include "losu_bytecode.h"
#include "losu_errmsg.h"
#include "losu_gc.h"
#include "losu_malloc.h"
#include "losu_object.h"
#include "losu_syntax.h"
#include "losu_vm.h"
#include <math.h>
#include <setjmp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef DEFINE_SOURCE_LOSU_VM_C_STATICDEC
#define DEFINE_SOURCE_LOSU_VM_C_STATICDEC

static LosuObj *__losuVmcoreCallCapi (LosuVm *vm, _inlineCallinfo *cinfo);
static const LosuObj *__losuVmcoreGetunit (LosuVm *vm, LosuObj *obj);
static void __losuVmcoreSetunit (LosuVm *vm, LosuObj *t, LosuObj *key);
static void __losuVmcoreSetglobal (LosuVm *vm, _inlineString *name);

static void __losuVmcoreAdjVarg (LosuVm *vm, LosuObj *base, int32_t nfixarg);
static _inlineFunction *__losuVmcoreFunction (LosuVm *vm, int32_t niss);

#endif

#ifndef DEFINE_SOURCE_LOSU_VM_C_VMHEAP
#define DEFINE_SOURCE_LOSU_VM_C_VMHEAP
void
__losu_vmHeap_init (LosuVm *vm, int32_t size)
{
  vm->base = vm->top = vm->stack = vm->mtop = vm->mstack
      = __losu_mem_newvector (vm, size, LosuObj);
  vm->nblocks += size * sizeof (LosuObj);
  vm->stackmax = vm->stack + size - 1;
}

void
__losu_vmHeap_check (LosuVm *vm, int32_t size)
{
  if (vm->stackmax - vm->top <= size)
    vm_error (vm, __config_losucore_errmsg_msgStackOverflow);
}

void
__losu_vmHeap_adjTop (LosuVm *vm, LosuObj *base, int32_t size)
{
  int32_t diff = size - (vm->top - base);
  if (diff <= 0)
    vm->top = base + size;
  else
    {
      __losu_vmHeap_check (vm, diff);
      while (diff--)
        ovtype ((vm->top++)) = LosuTypeDefine_null;
    }
}

void
__losu_vmHeap_rawcall (LosuVm *vm, LosuObj *func, int32_t nres)
{
  vm->callhook++;
  vm->deephook++;
  if (vm->deephook > 998)
    vm_error (vm, "超出最大递归限制");
  ;
  _inlineCallinfo cinfo = { NULL };
  if (ovtype (func) != LosuTypeDefine_function)
    vm_error (vm, __config_losucore_errmsg_msgCalledNotFunc);
  cinfo.nres = nres;
  cinfo.func = ovfunc (func);
  cinfo.inlinecall = (void (*) (LosuVm *vm, LosuObj *func,
                                int32_t nres))__losu_vmHeap_rawcall;
  cinfo.base = func + 1;

  ovtype (func) = LosuTypeDefine_callinfo;
  ovcall (func) = &cinfo;

  LosuObj *fres = (cinfo.func->isC) ? __losuVmcoreCallCapi (vm, &cinfo)
                                    : __losu_vmCore_exec (vm, &cinfo, NULL);
  if (nres == -1)
    {
      while (fres < vm->top)
        *func++ = *fres++;
      vm->top = func;
    }
  else
    {
      for (; nres > 0 && fres < vm->top; nres--)
        *func++ = *fres++;
      vm->top = func;
      for (; nres > 0; nres--)
        ovtype ((vm->top++)) = LosuTypeDefine_null;
    }
  vm->deephook--;
}

void
__losu_vmHeap_procall (LosuVm *vm, LosuObj *func, int32_t nres)
{
  vm->callhook++;
  _inlineCallinfo *cinfo = __losu_objCallinfo_new (vm);
  if (ovtype (func) != LosuTypeDefine_function)
    vm_error (vm, __config_losucore_errmsg_msgCalledNotFunc);

  cinfo->nres = nres;
  cinfo->func = ovfunc (func);
  cinfo->inlinecall = (void (*) (LosuVm *vm, LosuObj *func,
                                 int32_t nres))__losu_vmHeap_procall;
  cinfo->base = func + 1;
  ovtype (func) = LosuTypeDefine_callinfo;
  ovcall (func) = cinfo;
  LosuObj *fres = (cinfo->func->isC) ? __losuVmcoreCallCapi (vm, cinfo)
                                     : __losu_vmCore_exec (vm, cinfo, NULL);
  if (nres == -1)
    {
      while (fres < vm->top)
        *func++ = *fres++;
      vm->top = func;
    }
  else
    {
      for (; nres > 0 && fres < vm->top; nres--)
        *func++ = *fres++;
      vm->top = func;
      for (; nres > 0; nres--)
        ovtype ((vm->top++)) = LosuTypeDefine_null;
    }
  __losu_objCallinfo_free (vm, cinfo);
}

#endif

#ifndef DEFINE_SOURCE_LOSU_VM_C_VMCORE
#define DEFINE_SOURCE_LOSU_VM_C_VMCORE

static LosuObj *
__losuVmcoreCallCapi (LosuVm *vm, _inlineCallinfo *cinfo)
{
  int32_t nc = cinfo->func->nclosure, n = 0;
  LosuObj *oldbase = vm->base;
  vm->base = cinfo->base;
  __losu_vmHeap_check (vm, nc);
  for (; n < nc; n++)
    *(vm->top++) = cinfo->func->closure[n];
  n = (*cinfo->func->func.capi) (vm);
  vm->base = oldbase;
  return vm->top - n;
}

_l_bool
__losu_vmCore_Tonum (LosuVm *vm, LosuObj *obj, _l_bool isCore)
{
  LosuObj *oldtop = vm->top;
  switch (ovtype (obj))
    {
    case LosuTypeDefine_string:
      {
        if (!__losu_object_str2num (ovSstr (obj), &ovnumber (obj)))
          return 1;
        ovtype (obj) = LosuTypeDefine_number;
        return 0;
        break;
      }
    case LosuTypeDefine_unit:
      {
        if (isCore)
          return 1;
        LosuObj *func = obj_indexunitbystr (vm, *obj, "__num__");
        if (ovtype (func) != LosuTypeDefine_function)
          return 1;
        __losu_vmHeap_check (vm, 16);
        stack_push (vm, *func);
        stack_push (vm, *obj);
        stack_call (vm, 1, 1);
        *obj = *(vm->top - 1);
        vm->top = oldtop;
        return 0;
        break;
      }
    default:
      return 1;
      break;
    }
  return 1;
}

_l_bool
__losu_vmCore_Tostr (LosuVm *vm, LosuObj *obj, _l_bool isCore)
{
  LosuObj *oldtop = vm->top;
  switch (ovtype (obj))
    {
    case LosuTypeDefine_number:
      {
        char s[32] = { 0 };
        sprintf (s, "%.16g", ovnumber (obj));
        ovtype (obj) = LosuTypeDefine_string;
        ovIstr (obj) = __losu_objString_new (vm, s);
        return 0;
      }
    case LosuTypeDefine_unit:
      {
        if (isCore)
          return 1;
        LosuObj *func = obj_indexunitbystr (vm, *obj, "__str__");
        if (ovtype (func) != LosuTypeDefine_function)
          return 1;
        __losu_vmHeap_check (vm, 16);
        stack_push (vm, *func);
        stack_push (vm, *obj);
        stack_call (vm, 1, 1);
        *obj = *(vm->top - 1);
        vm->top = oldtop;
        return 0;
        break;
      }
    default:
      return 1;
      break;
    }
  return 1;
}

static const LosuObj *
__losuVmcoreGetunit (LosuVm *vm, LosuObj *obj)
{

  if (ovtype (obj) == LosuTypeDefine_unit)
    {
      if (!ovhash (obj)->isMap)
        return __losu_objUnit_getnum (ovhash (obj),
                                      obj_toint (vm, vm->top - 1));
      return __losu_objUnit_get (ovhash (obj), vm->top - 1);
    }
  return &_inlineNullObj;
}

static void
__losuVmcoreSetunit (LosuVm *vm, LosuObj *t, LosuObj *key)
{
  if (ovtype (t) == LosuTypeDefine_unit)
    {
      if (!ovhash (t)->isMap) // list
        {
          _l_int keynum = obj_toint (vm, key);
          if (keynum < 0)
            goto label_invalid_key;
          if (ovtype ((vm->top - 1)) == LosuTypeDefine_null)
            { // sort this unit
              // printf("debug\n");
              for (int i = keynum;; i++)
                {
                  LosuObj *n
                      = (LosuObj *)__losu_objUnit_getnum (ovhash (t), i);
                  LosuObj *nnext
                      = (LosuObj *)__losu_objUnit_getnum (ovhash (t), i + 1);

                  obj_setunitbynum (vm, *t, i, *nnext);
                  if (ovtype (nnext) == LosuTypeDefine_null)
                    break;
                }

              return;
            }
          else // generic
            *__losu_objUnit_setnum (vm, ovhash (t), (_l_number)keynum)
                = *(vm->top - 1);
        }
      else
        *__losu_objUnit_set (vm, ovhash (t), key) = *(vm->top - 1);
    }
  // else if (ovtype (t) == LosuTypeDefine_string)
  //   {
  //     const char *oldstr = ovSstr (t);
  //     char *newstr = __losu_mem_malloc (vm, strlen (oldstr) + 4);
  //     const char *setstr = obj_tostr (vm, (vm->top - 1));
  //     memset (newstr, 0, strlen (oldstr) + 4);
  //     _l_int keynum = obj_toint (vm, key);
  //     if (keynum < 0)
  //       goto label_invalid_key;
  //     int p = 0, pl = 0;
  //     while (oldstr[0] != '\0')
  //       {
  //         int ul = charset_utf8len (oldstr[0]);
  //         if (pl != keynum) // 复制
  //           for (int32_t i = 0; i < ul; i++)
  //             {
  //               newstr[p++] = oldstr[0];
  //               oldstr++;
  //             }
  //         else // 替换
  //           {
  //             for (int32_t i = 0; i < strlen (setstr); i++)
  //               newstr[p++] = setstr[i];
  //             oldstr += ul;
  //           }
  //         pl++;
  //       }
  //     *t->value.hash = *obj_newstr (vm, newstr).value.hash;
  //   }
  else
    vm_error (vm, __config_losucore_errmsg_msgInvalidSetobj);
  return;
label_invalid_key:
  vm_error (vm, "不允许的键值");
}

static void
__losuVmcoreSetglobal (LosuVm *vm, _inlineString *name)
{
  const LosuObj *oval = __losu_objUnit_getstr (vm->global, name);
  if (oval != &_inlineNullObj)
    *((LosuObj *)oval) = *(vm->top - 1);
  else
    {
      LosuObj key = (LosuObj){
        .type = LosuTypeDefine_string,
        .value.str = name,
      };
      *__losu_objUnit_set (vm, vm->global, &key) = *(vm->top - 1);
    }
}

void
__losuVmcoreAdjVarg (LosuVm *vm, LosuObj *base, int32_t nfixarg)
{
  int32_t nv = (vm->top - base) - nfixarg;
  if (nv < 0)
    __losu_vmHeap_adjTop (vm, base, nfixarg);

  {
    LosuObj *felem = base + nfixarg;
    int32_t i;
    _inlineHash *hunit = __losu_objUnit_new (vm, 0);
    for (i = 0; felem + i < vm->top; i++)
      *__losu_objUnit_setnum (vm, hunit, i + 1) = *(felem + i);

    *__losu_objUnit_setstr (vm, hunit, __losu_objString_new (vm, "n"))
        = (LosuObj){
            .type = LosuTypeDefine_number,
            .value.num = i,
          };

    vm->top = felem;
    ovtype ((vm->top)) = LosuTypeDefine_unit;
    ovhash ((vm->top)) = hunit;
    if (vm->top == vm->stackmax)
      __losu_vmHeap_check (vm, 1);
    vm->top++;
  }
}

static _inlineFunction *
__losuVmcoreFunction (LosuVm *vm, int32_t niss)
{
  _inlineFunction *f = __losu_objFunc_new (vm, niss);
  vm->top -= niss;
  while (niss--)
    f->closure[niss] = *(vm->top + niss);
  ovfunc ((vm->top)) = f;
  ovtype ((vm->top)) = LosuTypeDefine_function;
  if (vm->top == vm->stackmax)
    __losu_vmHeap_check (vm, 1);
  vm->top++;
  return f;
}

static void
checktype (LosuVm *vm, LosuObj *o1, LosuObj *o2)
{
  if (strcmp (obj_typeStr (vm, o1), obj_typeStr (vm, o2)))
    vm_error (vm, " '%s' 与 '%s' 不可进行关系运算", obj_typeStr (vm, o1),
              obj_typeStr (vm, o2));
}
static void
checktype1 (LosuVm *vm, LosuObj *o1, LosuObj *o2)
{
  if ((ovtype (o1) != ovtype (o2))
      && (ovtype (o1) != LosuTypeDefine_bool
          && ovtype (o2) != LosuTypeDefine_bool))
    vm_error (vm, " '%s' 与 '%s' 不可进行关系运算", obj_typeStr (vm, o1),
              obj_typeStr (vm, o2));
}
#include <emscripten.h>
LosuObj *
__losu_vmCore_exec (LosuVm *vm, _inlineCallinfo *cinfo, LosuObj *recall)
{
#define tonumber(o)                                                           \
  ((ovtype (o) != LosuTypeDefine_number)                                      \
   && (__losu_vmCore_Tonum (vm, o, 1)) != 0)
#define tostring(o)                                                           \
  ((ovtype (o) != LosuTypeDefine_string)                                      \
   && (__losu_vmCore_Tostr (vm, o, 1) != 0))
#define dojmp(pc, i)                                                          \
  {                                                                           \
    (pc) += cgIns_GetS (i);                                                   \
  }

  // if (recall)
  // cinfo = ovcall (recall);
  cinfo = recall ? ovcall (recall) : cinfo;

  _inlineScode *fcode = cinfo->func->func.sdef;
  _inlineString **lcstr = fcode->lcstr;
  _l_number *lcnum = fcode->lcnum;
  LosuObj *closure = cinfo->func->closure;
  _inlineScode **lcfcode = fcode->lcscode;
  LosuObj *top;
  LosuObj *base = cinfo->base;

  // printf ("heap:%ld\n", vm->top - vm->stack);
  // printf ("#\n");
  if (recall)
    {
      LosuObj *next = ovcall (recall)->nextobj;
      if (next)
        {
          int32_t nres = ovcall (next)->nres;
          LosuObj *fres = __losu_vmCore_exec (vm, NULL, next);
          if (nres == -1)
            {
              while (fres < vm->top)
                *next++ = *fres++;
              vm->top = next;
            }
          else
            {
              for (; nres > 0 && fres < vm->top; nres--)
                *next++ = *fres++;
              vm->top = next;
              for (; nres > 0; nres--)
                ovtype ((vm->top++)) = LosuTypeDefine_null;
            }
        }
    }
  else
    {

      __losu_vmHeap_check (vm, fcode->maxstacksize);
      if (fcode->isVarg)
        __losuVmcoreAdjVarg (vm, base, fcode->narg);
      else
        __losu_vmHeap_adjTop (vm, base, fcode->narg);
      cinfo->pc = fcode->code;
    }
  top = vm->top;
  // vmInstruction *pc = cinfo->pc;
  // while (cgIns_GetOp (*pc) != INS_END)
  //   {
  //     switch (cgIns_GetOp (*pc))
  //       {
  //       case INS_JMP:
  //         printf ("%ld:jmp %d\n", pc - cinfo->pc, cgIns_GetS (*pc));
  //         break;
  //       default:
  //         printf ("%ld:%d\n", pc - cinfo->pc, cgIns_GetOp (*pc));
  //       }
  //     pc++;
  //   }
  // printf ("--------\n");

  while (1)
    {
      vmInstruction i = *(cinfo->pc++);
      if (emscripten_run_script_int ("WasmMutex.runLock")
          == 0) // 如果没有运行锁，打断
        __losu_sigThrow (vm, VMSIGYIELD);

      // printf ("@:%d\n", cgIns_GetOp (i));
      switch (cgIns_GetOp (i))
        {
        case INS_END:
          {
            vm->top = top;
            return top;
          }
        case INS_RETURN:
          {
            vm->top = top;
            return base + cgIns_GetU (i);
          }
        case INS_CALL:
          {
            if (vm->callhook > 65535 || vm->loophook > 65535)
              {
                vm_error (vm, "运行资源超出最大限制:\n\t调用[%s]\t循环[%s]\n",
                          vm->callhook > 65535 ? "❗️" : "",
                          vm->loophook > 65535 ? "❗️" : "");
              }
            vm->top = top; // update top

            int16_t nres = cgIns_GetB (i);
            if (nres == 255)
              nres = -1;
            LosuObj *func = base + cgIns_GetA (i);
            if (ovtype (func) != LosuTypeDefine_function)
              vm_error (vm, __config_losucore_errmsg_msgCalledNotFunc);
            __losu_vmHeap_rawcall (vm, func, nres);
            top = vm->top;
            __losu_gc_checkClt (vm);
            break;
          }
        case INS_PUSHNULL:
          {
            vmIns_U n = cgIns_GetU (i);
            do
              {
                ovtype ((top++)) = LosuTypeDefine_null;
              }
            while (--n > 0);
            break;
          }
        case INS_POP:
          {
            top -= cgIns_GetU (i);
            break;
          }
        case INS_PUSHSTRING:
          {
            ovtype (top) = LosuTypeDefine_string;
            ovIstr (top) = (_inlineString *)lcstr[cgIns_GetU (i)];
            top++;
            break;
          }
        case INS_PUSHNUM:
          {
            ovtype (top) = LosuTypeDefine_number;
            ovnumber (top) = lcnum[cgIns_GetU (i)];
            top++;
            break;
          }
        case INS_PUSHUPVALUE:
          {
            *top++ = closure[cgIns_GetU (i)];
            break;
          }
        case INS_GETLOCAL:
          {
            *top++ = *(base + cgIns_GetU (i));
            break;
          }
        case INS_GETGLOBAL:
          {
            vm->top = top;
            *top = *__losu_objUnit_getstr (
                vm->global, (_inlineString *)lcstr[cgIns_GetU (i)]);
            // fprintf(stderr,"@%s\n", obj_typeStr(vm, top));
            top++;
            break;
          }
        case INS_GETUNIT:
          {
            vm->aluhook++;
            vm->top = top;
            top--;
            if (ovtype ((top - 1)) == LosuTypeDefine_string) // string
              { // get index number
                // 截取 str 中第 pos 个uincode字符，从0 开始
                _l_int pos = obj_toint (vm, top);
                _l_unicode u = 0;
                const char *str = obj_tostr (vm, top - 1);
                while (*str && pos > 0)
                  {
                    uint8_t len = charset_utf8len (str[0]);
                    if (len == 0)
                      break; // 非法 UTF-8 序列
                    str += len;
                    pos--;
                  }
                uint8_t len = charset_utf8len (str[0]);
                for (int i = 0; i < len; i++)
                  {
                    u = (u << 8) + (uint8_t)str[i];
                  }
                ovtype ((top - 1)) = LosuTypeDefine_unicode;
                ovunicode ((top - 1)) = u;
              }
            else
              *(top - 1) = *__losuVmcoreGetunit (vm, top - 1);

            break;
          }
        case INS_PUSHSELF:
          {
            LosuObj tmp;
            tmp = *(top - 1);
            ovtype (top) = LosuTypeDefine_string;
            ovIstr ((top++)) = (_inlineString *)lcstr[cgIns_GetU (i)];
            vm->top = top;
            *(top - 2) = *__losuVmcoreGetunit (vm, top - 2);
            *(top - 1) = tmp;
            break;
          }
        case INS_CREATEUNIT:
          {
            vm->top = top;
            __losu_gc_checkClt (vm);
            ovhash (top) = __losu_objUnit_new (vm, cgIns_GetA (i));
            ovtype (top) = LosuTypeDefine_unit;
            ovhash (top)->isMap = cgIns_GetB (i);
            top++;
            break;
          }
        case INS_SETLOCAL:
          {
            vm->aluhook++;
            if (ovtype (top - 1) == LosuTypeDefine_unit)
              {
                // copy index
                LosuObj new = obj_newunit (vm, 0);
                LosuObj obj = *(--top);
                if (ovhash (&obj)->isMap) // map
                  {
                    ovhash (&new)->isMap = 1;
                    LosuNode *n = obj_unit_first (vm, obj);
                    while (n)
                      {
                        obj_setunit (vm, new, n->key, n->value);
                        n = obj_unit_next (vm, obj, n);
                      }
                  }
                else // list
                  {
                    ovhash (&new)->isMap = 0;
                    int i = 0;
                    while (1)
                      {
                        LosuObj *v = obj_indexunitbynum (vm, obj, i);
                        if (ovtype (v) == LosuTypeDefine_null)
                          break;
                        obj_setunitbynum (vm, new, i, *v);
                        i++;
                      }
                  }
                *(base + cgIns_GetU (i)) = new;
              }
            else
              *(base + cgIns_GetU (i)) = *(--top);
            break;
          }
        case INS_SETGLOBAL:
          {
            vm->aluhook++;
            if (ovtype (top - 1) == LosuTypeDefine_unit)
              {
                // copy index
                LosuObj new = obj_newunit (vm, 0);
                LosuObj obj = *(top - 1);
                if (ovhash (&obj)->isMap) // map
                  {
                    ovhash (&new)->isMap = 1;
                    LosuNode *n = obj_unit_first (vm, obj);
                    while (n)
                      {
                        obj_setunit (vm, new, n->key, n->value);
                        n = obj_unit_next (vm, obj, n);
                      }
                  }
                else // list
                  {
                    ovhash (&new)->isMap = 0;
                    int i = 0;
                    while (1)
                      {
                        LosuObj *v = obj_indexunitbynum (vm, obj, i);
                        if (ovtype (v) == LosuTypeDefine_null)
                          break;
                        obj_setunitbynum (vm, new, i, *v);
                        i++;
                      }
                  }
                *(top - 1) = new;
              }
            vm->top = top;
            __losuVmcoreSetglobal (vm, (_inlineString *)lcstr[cgIns_GetU (i)]);
            top--;
            break;
          }
        case INS_SETUNIT:
          {
            vm->aluhook++;
            LosuObj *t = top - cgIns_GetA (i);
            vm->top = top;
            __losuVmcoreSetunit (vm, t, t + 1);
            top -= cgIns_GetB (i);
            break;
          }
        case INS_SETLIST:
          {
            uint32_t aux = cgIns_GetA (i) * vmOl_MaxSetlist;
            vmIns_B n = cgIns_GetB (i);
            _inlineHash *arr = ovhash ((top - n - 1));
            // arr->isMap = 0;
            vm->top = top - n;
            for (; n; n--)
              *__losu_objUnit_setnum (vm, arr, aux + n - 1) = *(--top);
            break;
          }
        case INS_SETMAP:
          {
            vmIns_U n = cgIns_GetU (i);
            LosuObj *ftop = top - 2 * n;
            _inlineHash *arr = ovhash ((ftop - 1));
            // arr->isMap = 1;
            vm->top = ftop;
            for (; n; n--)
              {
                top -= 2;
                *__losu_objUnit_set (vm, arr, top) = *(top + 1);
              }
            break;
          }
        case INS_ADD: // +
          {
            vm->aluhook++;
            vm->top = top;

            if (ovtype ((top - 1)) != ovtype ((top - 2)))
              {
                if (ovtype (top - 2) == LosuTypeDefine_unit
                    && ovhash (top - 2)->isMap == 0)
                  {
                    int32_t len = 0;
                    LosuNode *n = obj_unit_first (vm, *(top - 2));
                    while (n)
                      {
                        n = obj_unit_next (vm, *(top - 2), n);
                        len++;
                      }
                    obj_setunitbynum (vm, *(top - 2), len, *(top - 1));
                  }
                else if (ovtype (top - 1) == LosuTypeDefine_unit
                         && ovhash (top - 1)->isMap == 0)
                  {
                    LosuObj newlist = obj_newunit (vm, 0);
                    obj_setunitbynum (vm, newlist, 0, *(top - 2));
                    for (int32_t i = 0;; i++)
                      {
                        LosuObj *v = obj_indexunitbynum (vm, *(top - 1), i);
                        if (ovtype (v) == LosuTypeDefine_null)
                          break;
                        obj_setunitbynum (vm, newlist, i + 1, *v);
                        *(top - 2) = newlist;
                      }
                  }
                else
                  {
                    // number + int
                    _l_number n1 = obj_tonum (vm, top - 2);
                    _l_number n2 = obj_tonum (vm, top - 1);
                    ovtype (top - 2) = LosuTypeDefine_number;
                    ovnumber (top - 2) = n1 + n2;
                  }
              }
            else
              switch (ovtype (top - 2))
                {
                case LosuTypeDefine_number:
                  {
                    ovnumber (top - 2) += ovnumber (top - 1);
                    break;
                  }
                case LosuTypeDefine_int:
                  {
                    ovint (top - 2) += ovint (top - 1);
                    break;
                  }
                case LosuTypeDefine_unicode:
                  {
                    ovunicode (top - 2) += ovunicode (top - 1);
                    break;
                  }
                case LosuTypeDefine_string:
                  {
                    char *s1, *s2, *s0;
                    s1 = ovSstr ((top - 2));
                    s2 = ovSstr ((top - 1));
                    s0 = __losu_mem_malloc (vm, strlen (s1) + strlen (s2) + 1);
                    snprintf (s0, strlen (s1) + strlen (s2) + 1, "%s%s", s1,
                              s2);
                    ovIstr (top - 2) = __losu_objString_new (vm, s0);
                    __losu_mem_free (vm, s0);
                    break;
                  }
                case LosuTypeDefine_unit:
                  {
                    if (ovhash (top - 2)->isMap + ovhash (top - 1)->isMap
                        == 0) // all list
                      {
                        LosuObj newlist = obj_newunit (vm, 0);
                        int32_t i = 0;
                        for (int32_t j = 0;; j++)
                          {
                            LosuObj *o
                                = obj_indexunitbynum (vm, *(top - 2), j);
                            if (ovtype (o) == LosuTypeDefine_null)
                              break;
                            obj_setunitbynum (vm, newlist, i++, *o);
                          }
                        for (int32_t j = 0;; j++)
                          {
                            LosuObj *o
                                = obj_indexunitbynum (vm, *(top - 1), j);
                            if (ovtype (o) == LosuTypeDefine_null)
                              break;
                            obj_setunitbynum (vm, newlist, i++, *o);
                          }
                        *(top - 2) = newlist;
                        break;
                      }
                    if (ovhash (top - 2)->isMap + ovhash (top - 1)->isMap
                        == 2) // allmap
                      {
                        LosuObj newmap = obj_newunit (vm, 1);
                        LosuNode *n = obj_unit_first (vm, *(top - 2));
                        while (n)
                          {
                            obj_setunit (vm, newmap, n->key, n->value);
                            n = obj_unit_next (vm, *(top - 2), n);
                          }
                        n = obj_unit_first (vm, *(top - 1));
                        while (n)
                          {
                            obj_setunit (vm, newmap, n->key, n->value);
                            n = obj_unit_next (vm, *(top - 1), n);
                          }
                        *(top - 2) = newmap;
                        break;
                      }
                    vm_error (vm, " '%s' 与 '%s' 不能进行 '+' 操作",
                              obj_typeStr (vm, top - 2),
                              obj_typeStr (vm, top - 1));
                  }
                default:
                  {
                    vm_error (vm, " '%s' 与 '%s' 不能进行 '+' 操作",
                              obj_typeStr (vm, top - 2),
                              obj_typeStr (vm, top - 1));
                  }
                }

            top--;
            break;
          }
        case INS_SUB:
          {
            vm->aluhook++;
            if (ovtype (top - 1) != ovtype (top - 2))
              {
                _l_number n1, n2;
                n1 = obj_tonum (vm, top - 2);
                n2 = obj_tonum (vm, top - 1);
                ovtype (top - 2) = LosuTypeDefine_number;
                ovnumber (top - 2) = n1 - n2;
              }
            else
              switch (ovtype (top - 2))
                {
                case LosuTypeDefine_number:
                  {
                    ovnumber (top - 2) -= ovnumber (top - 1);
                    break;
                  }
                case LosuTypeDefine_int:
                  {
                    ovint (top - 2) -= ovint (top - 1);
                    break;
                  }
                case LosuTypeDefine_unicode:
                  {
                    ovunicode (top - 2) -= ovunicode (top - 1);
                    break;
                  }
                case LosuTypeDefine_string:
                  {
                    ovnumber (top - 2)
                        = obj_tonum (vm, top - 2) - obj_tonum (vm, top - 1);
                    ovtype (top - 2) = LosuTypeDefine_number;
                    break;
                  }
                default:
                  {
                    vm_error (vm, " '%s' 与 '%s' 不能进行 '-' 操作",
                              obj_typeStr (vm, top - 2),
                              obj_typeStr (vm, top - 1));
                  }
                }

            top--;
            break;
          }
        case INS_MULT:
          {
            vm->aluhook++;
            if (ovtype (top - 1) != ovtype (top - 2))
              {
                _l_number n1, n2;
                n1 = obj_tonum (vm, top - 2);
                n2 = obj_tonum (vm, top - 1);
                ovtype (top - 2) = LosuTypeDefine_number;
                ovnumber (top - 2) = n1 * n2;
              }
            else
              switch (ovtype (top - 2))
                {
                case LosuTypeDefine_number:
                  {
                    ovnumber (top - 2) *= ovnumber (top - 1);
                    break;
                  }
                case LosuTypeDefine_int:
                  {
                    ovint (top - 2) *= ovint (top - 1);
                    break;
                  }
                case LosuTypeDefine_unicode:
                  {
                    ovunicode (top - 2) *= ovunicode (top - 1);
                    break;
                  }
                case LosuTypeDefine_string:
                  {
                    ovnumber (top - 2)
                        = obj_tonum (vm, top - 2) * obj_tonum (vm, top - 1);
                    ovtype (top - 2) = LosuTypeDefine_number;
                    break;
                  }
                default:
                  {
                    vm_error (vm, " '%s' 与 '%s' 不能进行 '*' 操作",
                              obj_typeStr (vm, top - 2),
                              obj_typeStr (vm, top - 1));
                  }
                }

            top--;
            break;
          }
        case INS_DIV:
          {
            vm->aluhook++;
            if (ovtype (top - 1) != ovtype (top - 2))
              {
                _l_number n1, n2;
                n1 = obj_tonum (vm, top - 2);
                n2 = obj_tonum (vm, top - 1);
                ovtype (top - 2) = LosuTypeDefine_number;
                ovnumber (top - 2) = n1 / n2;
              }
            else
              switch (ovtype (top - 2))
                {
                case LosuTypeDefine_number:
                  {
                    _l_number n1, n2, n;
                    n1 = ovnumber (top - 2);
                    n2 = ovnumber (top - 1);
                    n = n1 / n2;
                    if ((_l_int)n == n)
                      {
                        ovint (top - 2) = (_l_int)n;
                        ovtype (top - 2) = LosuTypeDefine_int;
                      }
                    else
                      ovnumber (top - 2) = n;
                    break;
                  }
                case LosuTypeDefine_int:
                  {
                    double i
                        = (double)ovint (top - 2) / (double)ovint (top - 1);
                    _l_int j = (_l_int)i;
                    if (j != i)
                      {
                        ovtype (top - 2) = LosuTypeDefine_number;
                        ovnumber (top - 2) = i;
                      }
                    else
                      ovint (top - 2) = j;
                    break;
                  }
                case LosuTypeDefine_unicode:
                  {
                    ovunicode (top - 2) /= ovunicode (top - 1);
                    break;
                  }
                case LosuTypeDefine_string:
                  {
                    ovnumber (top - 2)
                        = obj_tonum (vm, top - 2) / obj_tonum (vm, top - 1);
                    ovtype (top - 2) = LosuTypeDefine_number;
                    break;
                  }
                default:
                  {
                    vm_error (vm, " '%s' 与 '%s' 不能进行 '/' 操作",
                              obj_typeStr (vm, top - 2),
                              obj_typeStr (vm, top - 1));
                  }
                }
            top--;
            break;
          }
        case INS_POW: // 被重写为 取模
          {
            vm->aluhook++;
            if (ovtype (top - 1) != ovtype (top - 2))
              {
                _l_int n1, n2;
                n1 = obj_toint (vm, top - 2);
                n2 = obj_toint (vm, top - 1);
                ovtype (top - 2) = LosuTypeDefine_int;
                ovint (top - 2) = n1 % n2;
              }
            else
              switch (ovtype (top - 2))
                {
                case LosuTypeDefine_number:
                  {
                    _l_int n1 = obj_toint (vm, top - 2);
                    _l_int n2 = obj_toint (vm, top - 1);
                    ovtype (top - 2) = LosuTypeDefine_int;
                    ovint (top - 2) = n1 % n2;
                    break;
                  }
                case LosuTypeDefine_int:
                  {
                    ovint (top - 2) = ovint (top - 2) % ovint (top - 1);
                    break;
                  }
                case LosuTypeDefine_unicode:
                  {
                    ovunicode (top - 2)
                        = ovunicode (top - 2) % ovunicode (top - 1);
                    break;
                  }
                case LosuTypeDefine_string:
                  {
                    ovint (top - 2)
                        = obj_toint (vm, top - 2) % obj_toint (vm, top - 1);
                    ovtype (top - 2) = LosuTypeDefine_int;
                    break;
                  }
                default:
                  {
                    vm_error (vm, " '%s' 与 '%s' 不能进行 '%' 操作",
                              obj_typeStr (vm, top - 2),
                              obj_typeStr (vm, top - 1));
                  }
                }

            top--;
            break;
          }
        case INS_CONCAT: // 被重写为 整除
          {
            vm->aluhook++;
            if (ovtype (top - 1) != ovtype (top - 2))
              {
                _l_int n1, n2;
                n1 = obj_toint (vm, top - 2);
                n2 = obj_toint (vm, top - 1);
                ovtype (top - 2) = LosuTypeDefine_int;
                ovint (top - 2) = n1 / n2;
              }
            else
              switch (ovtype (top - 2))
                {
                case LosuTypeDefine_number:
                  {
                    _l_number n = ovnumber (top - 2) / ovnumber (top - 1);
                    ovtype (top - 2) = LosuTypeDefine_int;
                    ovint (top - 2) = (_l_int)n;
                    break;
                  }
                case LosuTypeDefine_int:
                  {
                    ovint (top - 2) /= ovint (top - 1);
                    break;
                  }
                case LosuTypeDefine_unicode:
                  {
                    ovunicode (top - 2) /= ovunicode (top - 1);
                    break;
                  }
                case LosuTypeDefine_string:
                  {
                    ovint (top - 2)
                        = obj_toint (vm, top - 2) / obj_toint (vm, top - 1);
                    ovtype (top - 2) = LosuTypeDefine_int;
                    break;
                  }
                default:
                  {
                    vm_error (vm, " '%s' 与 '%s' 不能进行 '//' 操作",
                              obj_typeStr (vm, top - 2),
                              obj_typeStr (vm, top - 1));
                  }
                }
            top--;
            break;
          }
        case INS_NEG:
          {
            vm->aluhook++;
            switch (ovtype (top - 1))
              {
              case LosuTypeDefine_number:
                {
                  ovnumber (top - 1) = -ovnumber (top - 1);
                  break;
                }
              case LosuTypeDefine_int:
                {
                  ovint (top - 1) = -ovint (top - 1);
                  break;
                }
              case LosuTypeDefine_string:
                {
                  ovnumber (top - 1) = -obj_tonum (vm, top - 1);
                  ovtype (top - 1) = LosuTypeDefine_number;
                  break;
                }
                // case LosuTypeDefine_unit:
                //   {
                //     if (ovhash (top - 1)->isMap)
                //       goto ins_neg_err;
                //     LosuObj newlist = obj_newunit (vm, 0);

                //   }
              default:
                {
                ins_neg_err:
                  vm_error (vm, " '%s' 不能进行 '取负' 操作",
                            obj_typeStr (vm, top - 1));
                }
              }
            break;
          }
        case INS_NOT:
          {
            vm->aluhook++;
            switch (ovtype (top - 1))
              {
              case LosuTypeDefine_bool:
                {
                  ovbool (top - 1) = !ovbool (top - 1);
                  break;
                }
              case LosuTypeDefine_null:
                {
                  ovtype (top - 1) = LosuTypeDefine_bool;
                  ovbool (top - 1) = 1;
                  break;
                }
              default:
                {
                  ovtype (top - 1) = LosuTypeDefine_bool;
                  ovbool (top - 1) = 0;
                  break;
                }
              }
            break;
          }
        case INS_JMPNE:
          {
            // printf ("debug\n");
            vm->aluhook++;
            top -= 2;
            checktype1 (vm, top, top + 1);
            if (!__losu_object_isObjEqual (top, top + 1))
              dojmp (cinfo->pc, i);
            break;
          }
        case INS_JMPEQ:
          {
            vm->aluhook++;
            top -= 2;
            checktype1 (vm, top, top + 1);
            if (__losu_object_isObjEqual (top, top + 1))
              {
                // printf
                // ("debug(1):%d\n",cgIns_GetOp(*(cinfo->pc+cgIns_GetS(i))));
                dojmp (cinfo->pc, i);
              }
            break;
          }
        case INS_JMPLT:
          {
            vm->aluhook++;
            top -= 2;
            checktype (vm, top, top + 1);
            if (__losu_object_isObjLess (top, top + 1))
              dojmp (cinfo->pc, i);
            break;
          }
        case INS_JMPLE:
          {
            vm->aluhook++;
            top -= 2;
            checktype (vm, top, top + 1);
            if (!__losu_object_isObjLess (top + 1, top))
              dojmp (cinfo->pc, i);
            break;
          }
        case INS_JMPGT:
          {
            vm->aluhook++;
            top -= 2;
            checktype (vm, top, top + 1);
            if (__losu_object_isObjLess (top + 1, top))
              dojmp (cinfo->pc, i);
            break;
          }
        case INS_JMPGE:
          {
            vm->aluhook++;
            top -= 2;
            checktype (vm, top, top + 1);
            if (!__losu_object_isObjLess (top, top + 1))
              dojmp (cinfo->pc, i);
            break;
          }
        case INS_JMPT:
          {
            vm->aluhook++;
            top--;
            if (ovtype ((top)) == LosuTypeDefine_null
                || (ovtype (top) == LosuTypeDefine_bool && ovbool (top) == 0))
              ;
            else
              dojmp (cinfo->pc, i);
            break;
          }
        case INS_JMPF:
          {
            vm->aluhook++;
            top--;
            if (ovtype ((top)) == LosuTypeDefine_null
                || (ovtype (top) == LosuTypeDefine_bool && ovbool (top) == 0))
              dojmp (cinfo->pc, i);
            break;
          }
        case INS_JMPONT:
          {
            vm->aluhook++;
            if (ovtype ((top - 1)) == LosuTypeDefine_null
                || (ovtype (top) == LosuTypeDefine_bool && ovbool (top) == 0))
              top--;
            else
              dojmp (cinfo->pc, i);
            break;
          }
        case INS_JMPONF:
          {
            vm->aluhook++;
            if (ovtype ((top - 1)) == LosuTypeDefine_null
                || (ovtype (top) == LosuTypeDefine_bool && ovbool (top) == 0))
              {
                dojmp (cinfo->pc, i);
              }
            else
              top--;
            break;
          }
        case INS_JMP:
          {

            dojmp (cinfo->pc, i);
            break;
          }
        case INS_PUSHNULLJMP:
          {
            vm->aluhook++;
            ovtype ((top++)) = LosuTypeDefine_bool;
            ovbool (top - 1) = 0;
            cinfo->pc++;
            break;
          }
        // case INS_FORPREP:
        //   {
        //     if (tonumber ((top - 1)))
        //       vm_error (vm, __config_losucore_errmsg_msgInvalidForstep);
        //     if (tonumber ((top - 2)))
        //       vm_error (vm, __config_losucore_errmsg_msgInvalidFormax);
        //     if (tonumber ((top - 3)))
        //       vm_error (vm, __config_losucore_errmsg_msgInvalidForinit);
        //     if (ovnumber ((top - 1)) > 0
        //             ? ovnumber ((top - 3)) > ovnumber ((top - 2))
        //             : ovnumber ((top - 3)) < ovnumber ((top - 2)))
        //       {
        //         top -= 3;
        //         dojmp (cinfo->pc, i);
        //       }
        //     break;
        //   }
        // case INS_FORLOOP:
        //   {
        //     if (ovtype ((top - 3)) != LosuTypeDefine_number)
        //       vm_error (vm, __config_losucore_errmsg_msgInvalidForstep);
        //     ovnumber ((top - 3)) += ovnumber ((top - 1));
        //     if (ovnumber ((top - 1)) > 0
        //             ? ovnumber ((top - 3)) > ovnumber ((top - 2))
        //             : ovnumber ((top - 3)) < ovnumber ((top - 2)))
        //       top -= 3;
        //     else
        //       dojmp (cinfo->pc, i);
        //     break;
        //   }
        // case INS_LFORPREP:
        //   {
        //     LosuNode *node;
        //     if (ovtype ((top - 1)) != LosuTypeDefine_unit)
        //       vm_error (vm, __config_losucore_errmsg_msgInvalidForobj);
        //     node = (LosuNode *)__losu_objUnit_getnext (
        //         ovhash ((top - 1)), (LosuObj *)&_inlineNullObj);
        //     if (node == NULL)
        //       {
        //         top--;
        //         dojmp (cinfo->pc, i);
        //       }
        //     else
        //       {
        //         top += 2;
        //         *(top - 2) = node->key;
        //         *(top - 1) = node->value;
        //       }
        //     break;
        //   }
        // case INS_LFORLOOP:
        //   {
        //     LosuNode *node;
        //     node = (LosuNode *)__losu_objUnit_getnext (ovhash ((top - 3)),
        //                                                top - 2);
        //     if (node == NULL)
        //       top -= 3;
        //     else
        //       {
        //         *(top - 2) = node->key;
        //         *(top - 1) = node->value;
        //         dojmp (cinfo->pc, i);
        //       }
        //     break;
        //   }
        case INS_PUSHFUNCTION:
          {
            vm->top = top;
            {
              _inlineFunction *func
                  = __losuVmcoreFunction (vm, cgIns_GetB (i));
              func->func.sdef = (_inlineScode *)lcfcode[cgIns_GetA (i)];
              func->isC = 0;
              if (func->func.sdef->isMain)
                {
                  func->isMain = 1;
                  if (vm->main.main)
                    vm_error (vm, "重复定义 '算始' ");
                  vm->main.main
                      = (LosuObj *)__losu_mem_malloc (vm, sizeof (LosuObj));
                  memset (vm->main.main, 0, sizeof (LosuObj));
                  ovtype (vm->main.main) = LosuTypeDefine_function;
                  ovfunc (vm->main.main) = func;
                }
            }
            top = vm->top;
            __losu_gc_checkClt (vm);
            break;
          }
        // case INS_YIELD:
        //   {
        //     vm->top = top;
        //     if (vm->mstack != vm->stack)
        //       __losu_sigThrow (vm, VMSIGYIELD);
        //     break;
        //   }
        // ec2_extIns
        case EC2INS_PUSHSPACE:
          {
            ovtype (top++) = LosuTypeDefine_space;
            break;
          }
        case EC2INS_PUSHUNICODE:
          {
            _l_unicode u = cgIns_GetU (i);
            ovtype (top) = LosuTypeDefine_unicode;
            ovunicode (top) = u;
            top++;
            break;
          }
        case EC2INS_PUSHINT:
          {
            ovtype (top) = LosuTypeDefine_int;
            ovint (top) = cgIns_GetS (i);
            top++;
            break;
          }
        case EC2INS_PUSHBOOL:
          {
            _l_bool b = cgIns_GetU (i);
            ovtype (top) = LosuTypeDefine_bool;
            ovbool (top) = b;
            top++;
            break;
          }

        case EC2INS_BITAND:
          {
            vm->aluhook++;
            if (ovtype (top - 2) != ovtype (top - 1))
              vm_error (vm, " '%s' 与 '%s' 无法进行 '&' 位运算",
                        obj_typeStr (vm, top - 2), obj_typeStr (vm, top - 1));
            else
              switch (ovtype (top - 2))
                {
                case LosuTypeDefine_int:
                  {
                    ovint (top - 2) = ovint (top - 2) & ovint (top - 1);
                    break;
                  }
                case LosuTypeDefine_unicode:
                  {
                    ovunicode (top - 2)
                        = ovunicode (top - 2) & ovunicode (top - 1);
                    break;
                  }
                default:
                  {
                    vm_error (vm, " '%s' 与 '%s' 无法进行 '&' 位运算",
                              obj_typeStr (vm, top - 2),
                              obj_typeStr (vm, top - 1));
                    break;
                  }
                };
            top--;
            break;
          }
        case EC2INS_BITOR:
          {
            vm->aluhook++;
            if (ovtype (top - 2) != ovtype (top - 1))
              vm_error (vm, " '%s' 与 '%s' 无法进行 '|' 位运算",
                        obj_typeStr (vm, top - 2), obj_typeStr (vm, top - 1));
            else
              switch (ovtype (top - 2))
                {
                case LosuTypeDefine_int:
                  {
                    ovint (top - 2) = ovint (top - 2) | ovint (top - 1);
                    break;
                  }
                case LosuTypeDefine_unicode:
                  {
                    ovunicode (top - 2)
                        = ovunicode (top - 2) | ovunicode (top - 1);
                    break;
                  }
                default:
                  {
                    vm_error (vm, " '%s' 与 '%s' 无法进行 '|' 位运算",
                              obj_typeStr (vm, top - 2),
                              obj_typeStr (vm, top - 1));
                    break;
                  }
                };
            top--;
            break;
          }
        case EC2INS_BITNOT:
          {
            vm->aluhook++;
            switch (ovtype (top - 1))
              {
              case LosuTypeDefine_bool:
                {
                  ovbool (top - 1) = !ovbool (top - 1);
                  break;
                }
              case LosuTypeDefine_int:
                {
                  ovint (top - 1) = ~ovint (top - 1);
                  break;
                }
              case LosuTypeDefine_unicode:
                {
                  ovunicode (top - 1) = ~ovunicode (top - 1);
                  break;
                }
              default:
                {
                  ovtype (top - 1) = LosuTypeDefine_null;
                }
              }
            break;
          }
        case EC2INS_BITXOR:
          {
            vm->aluhook++;
            if (ovtype (top - 2) != ovtype (top - 1))
              vm_error (vm, " '%s' 与 '%s' 无法进行 '^' 位运算",
                        obj_typeStr (vm, top - 2), obj_typeStr (vm, top - 1));
            else
              switch (ovtype (top - 2))
                {
                case LosuTypeDefine_int:
                  {
                    ovint (top - 2) = ovint (top - 2) ^ ovint (top - 1);
                    break;
                  }
                case LosuTypeDefine_unicode:
                  {
                    ovunicode (top - 2)
                        = ovunicode (top - 2) ^ ovunicode (top - 1);
                    break;
                  }
                default:
                  {
                    vm_error (vm, " '%s' 与 '%s' 无法进行 '^' 位运算",
                              obj_typeStr (vm, top - 2),
                              obj_typeStr (vm, top - 1));
                    break;
                  }
                };
            top--;
            break;
          }
        case EC2INS_BITLSH:
          {
            vm->aluhook++;
            if (ovtype (top - 2) != ovtype (top - 1))
              {
                switch (ovtype (top - 2))
                  {
                  case LosuTypeDefine_int:
                    {
                      ovint (top - 2) = ovint (top - 2)
                                        << obj_toint (vm, top - 1);
                      break;
                    }
                  case LosuTypeDefine_unicode:
                    {
                      ovunicode (top - 2) = ovunicode (top - 2)
                                            << obj_toint (vm, top - 1);
                      break;
                    }
                  default:
                    {
                      vm_error (vm, " '%s' 与 '%s' 无法进行 '<<' 位运算",
                                obj_typeStr (vm, top - 2),
                                obj_typeStr (vm, top - 1));
                    }
                  }
              }
            else
              switch (ovtype (top - 2))
                {
                case LosuTypeDefine_int:
                  {
                    ovint (top - 2) = ovint (top - 2) << ovint (top - 1);
                    break;
                  }
                case LosuTypeDefine_unicode:
                  {
                    ovunicode (top - 2) = ovunicode (top - 2)
                                          << ovunicode (top - 1);
                    break;
                  }
                default:
                  {
                    vm_error (vm, " '%s' 与 '%s' 无法进行 '<<' 位运算",
                              obj_typeStr (vm, top - 2),
                              obj_typeStr (vm, top - 1));
                    break;
                  }
                };
            top--;
            break;
          }
        case EC2INS_BITRSH:
          {
            vm->aluhook++;
            if (ovtype (top - 2) != ovtype (top - 1))
              {
                switch (ovtype (top - 2))
                  {
                  case LosuTypeDefine_int:
                    {
                      ovint (top - 2)
                          = ovint (top - 2) >> obj_toint (vm, top - 1);
                      break;
                    }
                  case LosuTypeDefine_unicode:
                    {
                      ovunicode (top - 2)
                          = ovunicode (top - 2) >> obj_toint (vm, top - 1);
                      break;
                    }
                  default:
                    {
                      vm_error (vm, " '%s' 与 '%s' 无法进行 '>>' 位运算",
                                obj_typeStr (vm, top - 2),
                                obj_typeStr (vm, top - 1));
                    }
                  }
              }
            else
              switch (ovtype (top - 2))
                {
                case LosuTypeDefine_int:
                  {
                    ovint (top - 2) = ovint (top - 2) >> ovint (top - 1);
                    break;
                  }
                case LosuTypeDefine_unicode:
                  {
                    ovunicode (top - 2)
                        = ovunicode (top - 2) >> ovunicode (top - 1);
                    break;
                  }
                default:
                  {
                    vm_error (vm, " '%s' 与 '%s' 无法进行 '>>' 位运算",
                              obj_typeStr (vm, top - 2),
                              obj_typeStr (vm, top - 1));
                    break;
                  }
                };
            top--;
            break;
          }
        case EC2INS_PUSHSTA:
          {
            __losuvmSTA *newsta = __losu_mem_malloc (vm, sizeof (*newsta));
            newsta->top = top;
            newsta->pre = vm->stacksta;
            vm->stacksta = newsta;
            break;
          }
        case EC2INS_POPSTA:
          {
            __losuvmSTA *sta = vm->stacksta;
            vm->stacksta = sta->pre;
            __losu_mem_free (vm, sta);
            break;
          }
        case EC2INS_SETSTA:
          {
            if (vm->callhook > 65535 || vm->loophook > 65535)
              {
                vm_error (vm, "运行资源超出最大限制:\n\t调用[%s]\t循环[%s]\n",
                          vm->callhook > 65535 ? "❗️" : "",
                          vm->loophook > 65535 ? "❗️" : "");
              }
            vm->loophook++;
            top = vm->stacksta->top;
            break;
          }
        default:
          {
            vm_error (vm,
                      __config_losucore_errmsg_msgInvalidBytecode ":\t%d\n",
                      cgIns_GetOp (i));
          }
        }
    }

#undef tonumber
#undef tostring
#undef dojump
}

#endif

#endif
