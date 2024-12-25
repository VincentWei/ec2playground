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
#ifndef DEFINE_SOURCE_LOSU_OBJECT_C
#define DEFINE_SOURCE_LOSU_OBJECT_C

#include "losu_object.h"
#include "losu.h"
#include "losu_errmsg.h"
#include "losu_malloc.h"
#include <ctype.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef DEFINE_SOURCE_LOSU_OBJECT_C_OBJECT
#define DEFINE_SOURCE_LOSU_OBJECT_C_OBJECT

LosuWeak const LosuObj _inlineNullObj = {
  .type = LosuTypeDefine_null,
  .value = { NULL },
};

LosuWeak _l_bool
__losu_object_isObjEqual (const LosuObj *t1, const LosuObj *t2)
{
  if (ovtype (t1) != ovtype (t2))
    return 0;
  switch (ovtype (t1))
    {
    case LosuTypeDefine_string:
      {
        if (ovIstr (t1)->hash != ovIstr (t2)->hash)
          return 0;
        if (ovIstr (t1)->len != ovIstr (t2)->len)
          return 0;
        return !memcmp (ovSstr (t1), ovSstr (t2), ovIstr (t1)->len);
        break;
      }
    case LosuTypeDefine_number:
      {
        return ovnumber (t1) == ovnumber (t2);
        break;
      }
    case LosuTypeDefine_unit:
      {
        return ovhash (t1) == ovhash (t2);
        break;
      }
    case LosuTypeDefine_function:
      {
        return ovfunc (t1) == ovfunc (t2);
        break;
      }
    case LosuTypeDefine_coroutine:
      {
        return ovcoro (t1) == ovcoro (t2);
        break;
      }
    default:
      return 0;
    }
}

LosuWeak _l_bool
__losu_object_isObjLess (const LosuObj *t1, const LosuObj *t2)
{
  if (ovtype (t1) != ovtype (t2))
    return 0;
  switch (ovtype (t1))
    {
    case LosuTypeDefine_number:
      return ovnumber (t1) < ovnumber (t2);
    case LosuTypeDefine_string:
      {
        const void *l = (const void *)(ovSstr (t1));
        const void *r = (const void *)(ovSstr (t2));
        _l_size_t ll = ovIstr (t1)->len;
        _l_size_t lr = ovIstr (t2)->len;
        _l_size_t ml = ll < lr ? ll : lr;
        int32_t i = memcmp (l, r, ml);
        return i < 0 ? 1 : ((i == 0) ? (ll < lr) : 0);
      }
    default:
      return 0;
      break;
    }
  return 0;
}

_l_bool
__losu_object_str2num (const char *s, _l_number *n)
{
  char *endp;
  _l_number res = ostr2num (s, &endp);
  if (endp == s)
    return 0;
  while (isspace ((uint8_t)*endp))
    endp++;
  if (*endp != '\0')
    return 0;
  *n = res;
  return 1;
}

#endif

#ifndef DEFINE_SOURCE_LOSU_OBJECT_C_STRING
#define DEFINE_SOURCE_LOSU_OBJECT_C_STRING

/* declare segment */
#define sizestring(l) ((sizeof (_inlineString) + (l + 1 - 4) * sizeof (char)))
static _l_hash losuhash (const char *s, _l_size_t l);

static _l_hash
losuhash (const char *s, _l_size_t len)
{
  _l_size_t h = len;
  _l_size_t step = (len >> 5) | 1;
  for (; len >= step; len -= step)
    h = h ^ ((h << 5) + (h >> 2) + (unsigned char)*(s++));
  return (_l_hash)h;
}

void
__losu_objStringPool_init (LosuVm *vm)
{
  vm->strpool.strobj = __losu_mem_newvector (vm, 1, _inlineString *);
  vm->nblocks += sizeof (_inlineString *);
  vm->strpool.size = 1;
  vm->strpool.nsize = 0;
  vm->strpool.strobj[0] = NULL;
}

void
__losu_objStringPool_resize (LosuVm *vm, __losuvmStrseg *strpool, _l_hash s)
{
  _inlineString **newstrobj = __losu_mem_newvector (vm, s, _inlineString *);
  /* for (_l_hash i = 0; i < s; i++)
    newstrobj[i] = NULL; */
  memset (newstrobj, 0, s * sizeof (_inlineString *));
  for (_l_hash i = 0; i < strpool->size; i++)
    {
      _inlineString *p = strpool->strobj[i];
      while (p)
        {
          _inlineString *next = p->next;
          _l_hash h = p->hash;
          _l_hash h1 = h & (s - 1);
          p->next = newstrobj[h1];
          newstrobj[h1] = p;
          p = next;
        }
    }
  __losu_mem_free (vm, strpool->strobj);
  vm->nblocks += (s - strpool->size) * sizeof (_inlineString *);
  strpool->size = s;
  strpool->strobj = newstrobj;
}

void
__losu_objStringPool_deinit (LosuVm *vm)
{
  vm->nblocks -= vm->strpool.size * sizeof (_inlineString *);
  __losu_mem_free (vm, vm->strpool.strobj);
}

_inlineString *
__losu_objString_new (LosuVm *vm, const char *s)
{
  return __losu_objString_newstr (vm, s, strlen (s));
}
_inlineString *
__losu_objString_newconst (LosuVm *vm, const char *s)
{
  _inlineString *ts = __losu_objString_newstr (vm, s, strlen (s));
  if (!ts->marked)
    ts->marked = CONSTMARK;
  return ts;
}
_inlineString *
__losu_objString_newstr (LosuVm *vm, const char *s, _l_size_t len)
{
  _l_hash h = losuhash (s, len);
  _l_hash h1 = h & (vm->strpool.size - 1);
  _inlineString *ts;
  for (ts = vm->strpool.strobj[h1]; ts; ts = ts->next)
    {
      if (ts->hash == h)
        if (ts->len == len && (memcmp (s, ts->str, len) == 0))
          return ts;
    }
  ts = (_inlineString *)__losu_mem_malloc (vm, sizestring (len));
  ts->marked = 0;
  ts->next = NULL;
  ts->len = len;
  ts->hash = h;
  ts->cstidx = 0;
  memcpy (ts->str, s, len);
  ts->str[len] = '\0';
  vm->nblocks += sizestring (len);

  ts->next = vm->strpool.strobj[h1];
  vm->strpool.strobj[h1] = ts;
  vm->strpool.nsize++;
  if (vm->strpool.nsize > vm->strpool.size
      && vm->strpool.size < __config_losucore_vm_strpoolMax / 2 + 1)
    __losu_objStringPool_resize (vm, &vm->strpool, vm->strpool.size * 2);

  return ts;
}

#undef sizestring
#endif /* DEFINE_SOURCE_LOSU_OBJECT_C_STRING */

#ifndef DEFINE_SOURCE_LOSU_OBJECT_C_FUNCTION
#define DEFINE_SOURCE_LOSU_OBJECT_C_FUNCTION
#define sizeInlineFunction(n)                                                 \
  (sizeof (_inlineFunction) + sizeof (LosuObj) * ((n)))

#define sizeInlineScode(f)                                                    \
  ((sizeof (_inlineScode) + f->nlcnum * sizeof (_l_number)                    \
    + f->nlcstr * sizeof (_inlineString *)                                    \
    + f->nlcscode * sizeof (_inlineScode *)                                   \
    + f->ncode * sizeof (vmInstruction)                                       \
    + f->nlocalvar * sizeof (_inlineLocalvar)                                 \
    + f->nlineinfo * sizeof (int32_t)))

_inlineScode *
__losu_objFunc_scodeNew (LosuVm *vm)
{
  _inlineScode *f = __losu_mem_new (vm, _inlineScode);
  /* printf("new code %p\n", f); */
  memset ((void *)f, 0, sizeof (_inlineScode));
  f->marked = 0;
  f->next = vm->inspool;
  vm->inspool = f;
  f->isMain = 0;
  return f;
}
void
__losu_objFunc_scodeFree (LosuVm *vm, _inlineScode *f)
{
  /* printf("free code %p\n", f); */

  if (f->ncode > 0)
    vm->nblocks -= sizeInlineScode (f);
  __losu_mem_free (vm, f->code);
  __losu_mem_free (vm, f->localvar);
  __losu_mem_free (vm, f->lcstr);
  __losu_mem_free (vm, f->lcnum);
  __losu_mem_free (vm, f->lcscode);
  __losu_mem_free (vm, f->lineinfo);
  __losu_mem_free (vm, f);
}

_inlineFunction *
__losu_objFunc_new (LosuVm *vm, int32_t issnum)
{
  _l_size_t size = sizeInlineFunction (issnum);
  _inlineFunction *f = (_inlineFunction *)__losu_mem_malloc (vm, size);
  /* printf("new func %p\n", f); */
  f->next = vm->funcpool;
  vm->funcpool = f;
  f->mark = f;
  f->nclosure = issnum;
  vm->nblocks += size;
  f->isMain = 0;
  return f;
}
void
__losu_objFunc_free (LosuVm *vm, _inlineFunction *f)
{
  vm->nblocks -= sizeInlineFunction (f->nclosure);
  /* printf("free func %p\n", f); */
  __losu_mem_free (vm, f);
}

#undef sizeInlineFunction
#undef sizeInlineScode
#endif /* DEFINE_SOURCE_LOSU_OBJECT_C_FUNCTION */

#ifndef DEFINE_SOURCE_LOSU_OBJECT_C_UNIT
#define DEFINE_SOURCE_LOSU_OBJECT_C_UNIT

static LosuNode *__losuObjectUnitMainpoint (_inlineHash *t, LosuObj *key);
static void __losuObjectUnitSetvector (LosuVm *vm, _inlineHash *t,
                                       int32_t size);
static int32_t __losuObjectUnitGetnsize (_inlineHash *t);
static void __losuObjectUnitRehash (LosuVm *vm, _inlineHash *t);

#define sizeInlineHash(n) (sizeof (_inlineHash) + sizeof (LosuNode) * (n))

const LosuObj *
__losu_objUnit_getglobal (LosuVm *vm, const char *name)
{
  return __losu_objUnit_getstr (vm->global, __losu_objString_new (vm, name));
}
LosuWeak const LosuObj *
__losu_objUnit_get (_inlineHash *t, LosuObj *key)
{
  switch (ovtype (key))
    {
    case LosuTypeDefine_number:
      return __losu_objUnit_getnum (t, ovnumber (key));
    case LosuTypeDefine_string:
      return __losu_objUnit_getstr (t, ovIstr (key));
    default:
      return __losu_objUnit_getany (t, key);
    }
}
const LosuObj *
__losu_objUnit_getany (_inlineHash *t, LosuObj *key)
{
  LosuNode *n = __losuObjectUnitMainpoint (t, key);
  if (n)
    {
      do
        {
          if (__losu_object_isObjEqual ((const LosuObj *)(key),
                                        (const LosuObj *)(&n->key)))
            return (const LosuObj *)&n->value;
          n = n->next;
        }
      while (n);
    }
  return (const LosuObj *)&_inlineNullObj;
}

const LosuObj *
__losu_objUnit_getnum (_inlineHash *t, _l_number key)
{
  LosuNode *n = &t->node[(_l_hash)(key) & (t->size - 1)];
  do
    {
      if (ovtype ((&n->key)) == LosuTypeDefine_number
          && ovnumber ((&n->key)) == key)
        return (const LosuObj *)&n->value;
      n = n->next;
    }
  while (n);
  return (const LosuObj *)&_inlineNullObj;
}
const LosuObj *
__losu_objUnit_getstr (_inlineHash *t, _inlineString *key)
{
  LosuNode *n = &t->node[key->hash & (t->size - 1)];

  do
    {

      if (ovtype ((&n->key)) == LosuTypeDefine_string
          && ovIstr ((&n->key))->hash == key->hash)
        if (!strcmp (ovIstr ((&n->key))->str, key->str))
          return (const LosuObj *)&n->value;

      n = n->next;
    }
  while (n);
  return (const LosuObj *)&_inlineNullObj;
}
const LosuNode *
__losu_objUnit_getnext (_inlineHash *t, LosuObj *key)
{
  int32_t i;
  if (ovtype (key) == LosuTypeDefine_null)
    i = 0;
  else
    {
      const LosuObj *v = __losu_objUnit_get (t, key);
      if (v == &_inlineNullObj)
        return NULL;
      i = (int32_t)(((char *)v - (char *)(&t->node[0].value))
                        / sizeof (LosuNode)
                    + 1);
    }
  for (; i < t->size; i++)
    {
      LosuNode *n = &(t->node[i]);
      if (ovtype ((&n->value)) != LosuTypeDefine_null)
        return n;
    }

  return NULL;
}

_inlineHash *
__losu_objUnit_new (LosuVm *vm, int32_t size)
{

  _inlineHash *t = __losu_mem_new (vm, _inlineHash);
  t->isMap = 0;
  t->next = vm->hashpool;
  vm->hashpool = t;
  t->mark = t;
  t->size = 0;

  vm->nblocks += sizeInlineHash (0);
  t->node = NULL;
  {
    int32_t p = 4;
    while (p <= size)
      p <<= 1;
    size = p;
  }
  __losuObjectUnitSetvector (vm, t, size);
  return t;
}
void
__losu_objUnit_remove (_inlineHash *t, LosuObj *key)
{
  ovtype (key) = LosuTypeDefine_number;
  ovnumber (key) = 0;
}
void
__losu_objUnit_free (LosuVm *vm, _inlineHash *t)
{
  vm->nblocks -= sizeInlineHash (t->size);
  __losu_mem_free (vm, t->node);
  __losu_mem_free (vm, t);
}

LosuObj *
__losu_objUnit_set (LosuVm *vm, _inlineHash *t, LosuObj *key)
{
  LosuNode *mp = __losuObjectUnitMainpoint (t, key);
  LosuNode *n = mp;
  if (!n)
    vm_error (vm, __config_losucore_errmsg_msgInvalidUnitIndex);
  do
    {
      if (__losu_object_isObjEqual (key, &n->key))
        return &n->value;
      n = n->next;
    }
  while (n);
  if (ovtype ((&mp->key)) != LosuTypeDefine_null)
    {
      LosuNode *oth;
      n = t->free;
      if (mp > n && (oth = __losuObjectUnitMainpoint (t, &mp->key)) != mp)
        {
          while (oth->next != mp)
            oth = oth->next;
          oth->next = n;
          *n = *mp;
          mp->next = NULL;
        }
      else
        {
          n->next = mp->next;
          mp->next = n;
          mp = n;
        }
    }
  mp->key = *key;
  while (1)
    {
      if (ovtype ((&t->free->key)) == LosuTypeDefine_null)
        return &mp->value;
      else if (t->free == t->node)
        break;
      else
        (t->free)--;
    }
  __losuObjectUnitRehash (vm, t);
  return __losu_objUnit_set (vm, t, key);
}

LosuObj *
__losu_objUnit_setnum (LosuVm *vm, _inlineHash *t, _l_number key)
{
  LosuObj idx = (LosuObj){
    .type = LosuTypeDefine_number,
    .value.num = key,
  };
  return __losu_objUnit_set (vm, t, &idx);
}
LosuObj *
__losu_objUnit_setstr (LosuVm *vm, _inlineHash *t, _inlineString *key)
{
  LosuObj idx = (LosuObj){
    .type = LosuTypeDefine_string,
    .value.str = key,
  };
  return __losu_objUnit_set (vm, t, &idx);
}

static LosuNode *
__losuObjectUnitMainpoint (_inlineHash *t, LosuObj *key)
{
  _l_size_t h;
  switch (key->type)
    {
    case LosuTypeDefine_int:
      h = (_l_size_t)(ovint (key));
    case LosuTypeDefine_unicode:
      h = (_l_size_t)(ovunicode (key));
    case LosuTypeDefine_number:
      h = (_l_size_t)(ovnumber (key));
      break;
    case LosuTypeDefine_string:
      /* case LosuTypeDefine_byte: */
      h = (_l_size_t)(ovIstr (key)->hash);
      break;
    case LosuTypeDefine_unit:
      h = (_l_size_t)((size_t)(ovhash (key)));
      break;
    case LosuTypeDefine_function:
      h = (_l_size_t)((size_t)(ovfunc (key)));
      break;
    default:
      return NULL;
    }
  return &t->node[h & (t->size - 1)];
}
static void
__losuObjectUnitSetvector (LosuVm *vm, _inlineHash *t, int32_t size)
{
  int32_t i;
  if (size > INT32_MAX)
    vm_error (vm, __config_losucore_errmsg_msgVectorOverflow);
  t->node = __losu_mem_newvector (vm, size, LosuNode);
  for (i = 0; i < size; i++)
    {
      ovtype ((&t->node[i].key)) = ovtype ((&t->node[i].value))
          = LosuTypeDefine_null;
      t->node[i].next = NULL;
    }
  vm->nblocks += (sizeInlineHash (size)) - (sizeInlineHash (t->size));
  t->size = size;
  t->free = &t->node[size - 1];
}
static int32_t
__losuObjectUnitGetnsize (_inlineHash *t)
{
  LosuNode *v = t->node;
  int32_t s = t->size;
  int32_t use = 0;
  int32_t i;
  for (i = 0; i < s; i++)
    if (ovtype ((&v[i].value)) != LosuTypeDefine_null)
      use++;
  return use;
}
static void
__losuObjectUnitRehash (LosuVm *vm, _inlineHash *t)
{
  int32_t osize = t->size;
  LosuNode *onode = t->node;
  int32_t uszie = __losuObjectUnitGetnsize (t);
  int32_t i;
  if (uszie >= osize - osize / 4)
    __losuObjectUnitSetvector (vm, t, osize * 2);
  else if (uszie <= osize / 4 && osize > 4)
    __losuObjectUnitSetvector (vm, t, osize / 2);
  else
    __losuObjectUnitSetvector (vm, t, osize);

  for (i = 0; i < osize; i++)
    if (ovtype ((&((onode + i)->value))) != LosuTypeDefine_null)
      *__losu_objUnit_set (vm, t, &(onode + i)->key)
          = ((LosuNode *)(onode + i))->value;
  __losu_mem_free (vm, onode);
}

#undef sizeInlineHash
#endif

#ifndef DEFINE_SOURCE_LOSU_OBJECT_C_CALLINFO
#define DEFINE_SOURCE_LOSU_OBJECT_C_CALLINFO
_inlineCallinfo *
__losu_objCallinfo_new (LosuVm *vm)
{
  _inlineCallinfo *nc = __losu_mem_new (vm, _inlineCallinfo);
  /* printf ("malloc:%p\n", nc); */
  vm->nblocks += sizeof (_inlineCallinfo);
  memset (nc, 0, sizeof (_inlineCallinfo));
  nc->pre = NULL;
  nc->next = vm->callpool;
  if (vm->callpool)
    vm->callpool->pre = nc;
  vm->callpool = nc;
  nc->isMalloc = 1;
  return nc;
}

void
__losu_objCallinfo_free (LosuVm *vm, _inlineCallinfo *c)
{
  if (!c->isMalloc)
    return;
  /* printf ("free:%p\tpre:%p\tnext:%p\tcallpool:%p\n", c, c->pre, c->next,
          vm->callpool); */

  if (c->pre)
    c->pre->next = c->next;
  if (c->next)
    c->next->pre = c->pre;
  if (c == vm->callpool)
    vm->callpool = c->next;
  __losu_mem_free (vm, (_inlineCallinfo *)c);

  vm->nblocks -= sizeof (_inlineCallinfo);
}

#endif /* DEFINE_SOURCE_LOSU_OBJECT_C_CALLINFO */

#ifndef DEFINE_SOURCE_LOSU_OBJECT_C_COROUTINE
#define DEFINE_SOURCE_LOSU_OBJECT_C_COROUTINE

_inlineCoroutine *
__losu_objCoroutine_new (LosuVm *vm)
{
  _inlineCoroutine *nc = __losu_mem_new (vm, _inlineCoroutine);
  vm->nblocks += sizeof (_inlineCoroutine);
  memset (nc, 0, sizeof (_inlineCoroutine));
  nc->pre = NULL;
  nc->next = vm->coropool;
  if (vm->coropool)
    vm->coropool->pre = nc;
  vm->coropool = nc;
  return nc;
}

void
__losu_objCoroutine_free (LosuVm *vm, _inlineCoroutine *co)
{
  if (co->pre)
    co->pre->next = co->next;
  if (co->next)
    co->next->pre = co->pre;
  if (co == vm->coropool)
    vm->coropool = co->next;
  __losu_mem_free (vm, co);
  vm->nblocks -= sizeof (_inlineCoroutine);
}

#endif /* DEFINE_SOURCE_LOSU_OBJECT_C_COROUTINE */

#endif