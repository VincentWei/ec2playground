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
 * Losu Gc, a garbage collector for Losu Automated Memory Management.
 * Using the 'Mark-Clean'.
 */

#ifndef DEFINE_SOURCE_LOSU_GC_C
#define DEFINE_SOURCE_LOSU_GC_C

#include "losu_gc.h"
#include "losu.h"
#include "losu_errmsg.h"
#include "losu_malloc.h"
#include "losu_object.h"
#include <stdio.h>

#ifndef DEFINE_SOURCE_LOSU_GC_C_STATICDEC
#define DEFINE_SOURCE_LOSU_GC_C_STATICDEC

/* static segment */
static void __losuGcMark (LosuVm *vm);
static void __losuGcmarkObj (_inlineGc *gc, LosuObj *o);
static void __losuGcmarkStack (LosuVm *vm, _inlineGc *gc);
static void __losuGcmarkFunc (_inlineGc *gc, _inlineFunction *f);
static void __losuGcmarkScode (_inlineScode *f);

static void __losuGcCollect (LosuVm *vm);
static void __losuGccollectScode (LosuVm *vm);
static void __losuGccollectFunc (LosuVm *vm);
static void __losuGccollectUnit (LosuVm *vm);
static void __losuGccollectString (LosuVm *vm, _l_bool all);
static void __losuGccollectCallinfo (LosuVm *vm);
static void __losuGccollectCoroutine (LosuVm *vm);

static void __losuGcAdjbuff (LosuVm *vm);

void
__losu_gc_collect (LosuVm *vm, _l_bool all)
{
  __losuGccollectString (vm, all);
  __losuGccollectUnit (vm);
  __losuGccollectScode (vm);
  __losuGccollectFunc (vm);
  __losuGccollectCallinfo (vm);
  __losuGccollectCoroutine (vm);
  __losuGcAdjbuff (vm);
}

_l_bool
__losu_gc_checkClt (LosuVm *vm)
{
  if (vm->nblocks > vm->gcHook)
    vm->gcHook = vm->nblocks;
  if (vm->nblocks >= vm->gcDymax)
    {
      __losuGcCollect (vm);
      if (vm->gcDymax > vm->gcMax)
        vm->gcDymax = vm->gcMax;
      return 1;
    }
  return 0;
}

#endif /* DEFINE_SOURCE_LOSU_GC_C_STATICDEC */

#ifndef DEFINE_SOURCE_LOSU_GC_C_GCMARK
#define DEFINE_SOURCE_LOSU_GC_C_GCMARK
static void
__losuGcMark (LosuVm *vm)
{
  _inlineGc gc;
  gc.func = NULL;
  gc.unit = vm->global;
  vm->global->mark = NULL;
  __losuGcmarkStack (vm, &gc);
  while (1)
    {
      if (gc.func)
        {
          _inlineFunction *f = gc.func;
          gc.func = f->mark;
          for (int32_t i = 0; i < f->nclosure; i++)
            __losuGcmarkObj (&gc, &f->closure[i]);
        }
      else if (gc.unit)
        {
          _inlineHash *h = gc.unit;
          gc.unit = h->mark;
          for (int32_t i = 0; i < h->size; i++)
            {
              LosuNode *n = &h->node[i];
              if (ovtype ((&n->key)) != LosuTypeDefine_null)
                {
                  if (ovtype ((&n->value)) == LosuTypeDefine_null)
                    {
                      /* __losu_objUnit_remove (h, &n->key); */
                      ovtype ((&n->key)) = LosuTypeDefine_number;
                      ovnumber ((&n->key)) = 0;
                    }
                  __losuGcmarkObj (&gc, &n->key);
                  __losuGcmarkObj (&gc, &n->value);
                }
            }
        }
      else
        break;
    }
}

static void
__losuGcmarkObj (_inlineGc *gc, LosuObj *o)
{
  switch (ovtype ((o)))
    {
    case LosuTypeDefine_string:
      /*     case LosuTypeDefine_byte: */
      {
        ovIstr (o)->marked = 1;
        break;
      }
    case LosuTypeDefine_function:
      {
        __losuGcmarkFunc (gc, ovfunc (o));
        break;
      }
    case LosuTypeDefine_unit:
      {
        if (!(ovhash (o)->mark != ovhash (o)))
          {
            ovhash (o)->mark = gc->unit;
            gc->unit = ovhash (o);
          }
        break;
      }
    case LosuTypeDefine_coroutine:
      {
        ovcoro (o)->marked = 1;
        break;
      }
    case LosuTypeDefine_callinfo:
      {
        if (ovcall (o)->isMalloc)
          ovcall (o)->marked = 1;
        __losuGcmarkFunc (gc, ovcall (o)->func);
        break;
      }
    default:
      break;
    }
}

static void
__losuGcmarkStack (LosuVm *vm, _inlineGc *gc)
{
  /* LosuObj *o;
  for (o = vm->stack; o < vm->top; o++)
    __losuGcmarkObj (gc, o); */
  LosuObj *o;
  /* main stack */
  if (vm->stack == vm->mstack)
    vm->mtop = vm->top;
  for (o = vm->mstack; o < vm->mtop; o++)
    __losuGcmarkObj (gc, o);
  /* coro */
  _inlineCoroutine *coro;
  for (coro = vm->coropool; coro; coro = coro->next)
    if (coro->sta != LosuCoroStateEND)
      for (o = coro->stack; o < coro->top; o++)
        __losuGcmarkObj (gc, o);
}

static void
__losuGcmarkFunc (_inlineGc *gc, _inlineFunction *f)
{
  if (f->mark == f)
    {
      if (!f->isC)
        __losuGcmarkScode (f->func.sdef);
      f->mark = gc->func;
      gc->func = f;
    }
}

static void
__losuGcmarkScode (_inlineScode *f)
{
  if (!f->marked)
    {
      f->marked = 1;
      f->src->marked = 1;
      for (int32_t i = 0; i < f->nlcstr; i++)
        f->lcstr[i]->marked = 1;
      for (int32_t i = 0; i < f->nlcscode; i++)
        __losuGcmarkScode (f->lcscode[i]);
      for (int32_t i = 0; i < f->nlocalvar; i++)
        f->localvar[i].name->marked = 1;
    }
}

#endif /* DEFINE_SOURCE_LOSU_GC_C_GCMARK */

#ifndef DEFINE_SOURCE_LOSU_GC_C_GCCOLLECT
#define DEFINE_SOURCE_LOSU_GC_C_GCCOLLECT

static void
__losuGcCollect (LosuVm *vm)
{
  __losuGcMark (vm);
  __losu_gc_collect (vm, 0);

  vm->gcDymax *= 2;
}

static void
__losuGccollectScode (LosuVm *vm)
{
  _inlineScode **f = &vm->inspool;
  _inlineScode *next;
  while ((next = *f) != NULL)
    {
      if (next->marked)
        {
          /* printf ("Gc: %p\n",next); */
          next->marked = 0;
          f = &next->next;
        }
      else
        {
          *f = next->next;
          __losu_objFunc_scodeFree (vm, next);
        }
    }
}

static void
__losuGccollectFunc (LosuVm *vm)
{
  _inlineFunction **f = &vm->funcpool;
  _inlineFunction *next;
  while ((next = *f) != NULL)
    {
      if (next->mark != next)
        {
          next->mark = next;
          f = &next->next;
        }
      else
        {
          *f = next->next;
          __losu_objFunc_free (vm, next);
        }
    }
}

static void
__losuGccollectUnit (LosuVm *vm)
{
  _inlineHash **h = &vm->hashpool;
  _inlineHash *next;
  while ((next = *h) != NULL)
    {
      if (next->mark != next)
        {
          next->mark = next;
          h = &next->next;
        }
      else
        {
          *h = next->next;
          __losu_objUnit_free (vm, next);
        }
    }
}

static void
__losuGccollectString (LosuVm *vm, _l_bool all)
{
#define sizestring(l) ((sizeof (_inlineString) + (l + 1 - 4) * sizeof (char)))

  for (int32_t i = 0; i < vm->strpool.size; i++)
    {
      _inlineString **s = &vm->strpool.strobj[i];
      _inlineString *next;
      while ((next = *s) != NULL)
        {
          if (next->marked && !all)
            {
              if (next->marked < CONSTMARK)
                next->marked = 0;
              s = &next->next;
            }
          else
            {
              *s = next->next;
              vm->strpool.nsize--;
              vm->nblocks -= sizestring (next->len);
              __losu_mem_free (vm, next);
            }
        }
    }
  if (vm->strpool.nsize < vm->strpool.size / 4 && vm->strpool.size > 8)
    __losu_objStringPool_resize (vm, &vm->strpool, vm->strpool.size / 2);

#undef sizestring
}

static void
__losuGccollectCallinfo (LosuVm *vm)
{
  _inlineCallinfo *cobj = vm->callpool;
  while (cobj != NULL)
    {
      if (cobj->marked == 1)
        {
          cobj->marked = 0;
          cobj = cobj->next;
        }
      else
        {
          _inlineCallinfo *tmp = cobj;
          cobj = cobj->next;
          __losu_objCallinfo_free (vm, tmp);
        }
    }
}
static void
__losuGccollectCoroutine (LosuVm *vm)
{
  _inlineCoroutine *cobj = vm->coropool;
  while (cobj != NULL)
    {
      if (cobj->marked == 1)
        {
          cobj->marked = 0;
          cobj = cobj->next;
        }
      else
        {
          _inlineCoroutine *tmp = cobj;
          cobj = cobj->next;
          __losu_objCoroutine_free (vm, tmp);
        }
    }
}

static void
__losuGcAdjbuff (LosuVm *vm)
{
  if (vm->bufftmp)
    {
      vm->nblocks -= vm->nbufftmp * sizeof (char);
      vm->nbufftmp = 0;
      __losu_mem_free (vm, vm->bufftmp);
      vm->bufftmp = NULL;
    }
}

#endif /* DEFINE_SOURCE_LOSU_GC_C_GCCOLLECT */

#endif

