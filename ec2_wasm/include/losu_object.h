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
Object for Losu DataStruct
*/

#ifndef DEFINE_INCLUDE_LOSU_OBJECT_H
#define DEFINE_INCLUDE_LOSU_OBJECT_H

#include "losu.h"

#ifndef DEFINE_INCLUDE_LOSU_OBJECT_H_LOSU_OBJECT
#define DEFINE_INCLUDE_LOSU_OBJECT_H_LOSU_OBJECT

LosuExtern const LosuObj _inlineNullObj;
_l_bool __losu_object_isObjEqual (const LosuObj *t1, const LosuObj *t2);
_l_bool __losu_object_isObjLess (const LosuObj *t1, const LosuObj *t2);

_l_bool __losu_object_str2num (const char *s, _l_number *n);
#define ovtype(o) ((o)->type)
#define ovnumber(o) ((o)->value.num)
#define ovint(o) ((o)->value.intnum)
#define ovunicode(o) ((o)->value.unicode)
#define ovbool(o) ((o)->value.boolnum)
#define ovIstr(o) ((o)->value.str)
#define ovSstr(o) ((o)->value.str->str)
#define ovfunc(o) ((o)->value.func)
#define ovhash(o) ((o)->value.hash)
#define ovcall(o) ((o)->value.call)
#define ovcoro(o) ((o)->value.coro)
#define ovchar(o) ((o)->value._char)
#define ostr2num(s, p) (strtod ((s), (p)))

#endif /* DEFINE_INCLUDE_LOSU_OBJECT_H_LOSU_OBJECT */

#ifndef DEFINE_INCLUDE_LOSU_OBJECT_H_LOSU_STRING
#define DEFINE_INCLUDE_LOSU_OBJECT_H_LOSU_STRING

#define CONSTMARK 2
void __losu_objStringPool_init (LosuVm *vm);
void __losu_objStringPool_resize (LosuVm *vm, __losuvmStrseg *strpool,
                                  _l_hash s);
void __losu_objStringPool_deinit (LosuVm *vm);

_inlineString *__losu_objString_new (LosuVm *vm, const char *s);
_inlineString *__losu_objString_newstr (LosuVm *vm, const char *s,
                                        _l_size_t l);
_inlineString *__losu_objString_newconst (LosuVm *vm, const char *s);

#endif /* DEFINE_INCLUDE_LOSU_OBJECT_H_LOSU_STRING */

#ifndef DEFINE_INCLUDE_LOSU_OBJECT_H_LOSU_FUNCTION
#define DEFINE_INCLUDE_LOSU_OBJECT_H_LOSU_FUNCTION

_inlineScode *__losu_objFunc_scodeNew (LosuVm *vm);
void __losu_objFunc_scodeFree (LosuVm *vm, _inlineScode *f);

_inlineFunction *__losu_objFunc_new (LosuVm *vm, int32_t issnum);
void __losu_objFunc_free (LosuVm *vm, _inlineFunction *f);

#endif /* DEFINE_INCLUDE_LOSU_OBJECT_H_LOSU_FUNCTION */

#ifndef DEFINE_INCLUDE_LOSU_OBJECT_H_LOSU_UNIT
#define DEFINE_INCLUDE_LOSU_OBJECT_H_LOSU_UNIT

const LosuObj *__losu_objUnit_getglobal (LosuVm *vm, const char *name);
const LosuObj *__losu_objUnit_get (_inlineHash *t, LosuObj *key);
const LosuObj *__losu_objUnit_getany (_inlineHash *t, LosuObj *key);
const LosuObj *__losu_objUnit_getnum (_inlineHash *t, _l_number key);
const LosuObj *__losu_objUnit_getstr (_inlineHash *t, _inlineString *key);
const LosuNode *__losu_objUnit_getnext (_inlineHash *t, LosuObj *key);

_inlineHash *__losu_objUnit_new (LosuVm *vm, int32_t size);
void __losu_objUnit_remove (_inlineHash *t, LosuObj *key);
void __losu_objUnit_free (LosuVm *vm, _inlineHash *t);

LosuObj *__losu_objUnit_set (LosuVm *vm, _inlineHash *t, LosuObj *key);
LosuObj *__losu_objUnit_setnum (LosuVm *vm, _inlineHash *t, _l_number key);
LosuObj *__losu_objUnit_setstr (LosuVm *vm, _inlineHash *t,
                                _inlineString *key);

#endif /* DEFINE_INCLUDE_LOSU_OBJECT_H_LOSU_UNIT */

#ifndef DEFINE_INCLUDE_LOSU_OBJECT_H_LOSU_CALLINFO
#define DEFINE_INCLUDE_LOSU_OBJECT_H_LOSU_CALLINFO
_inlineCallinfo *__losu_objCallinfo_new (LosuVm *vm);
void __losu_objCallinfo_free (LosuVm *vm, _inlineCallinfo *ci);
#endif /* DEFINE_INCLUDE_LOSU_OBJECT_H_LOSU_CALLINFO */

#ifndef DEFINE_INCLUDE_LOSU_OBJECT_H_LOSU_CORO
#define DEFINE_INCLUDE_LOSU_OBJECT_H_LOSU_CORO

#define LosuCoroStateREADY 0
#define LosuCoroStateRUN 1
#define LosuCoroStateYIELD 2
#define LosuCoroStateEND 3

_inlineCoroutine *__losu_objCoroutine_new (LosuVm *vm);
void __losu_objCoroutine_free (LosuVm *vm, _inlineCoroutine *co);

#endif

#endif /* define_included_losu_object_h */
