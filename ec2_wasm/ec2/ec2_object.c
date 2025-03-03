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

#include "losu.h"
#include "losu_errmsg.h"
#include "losu_malloc.h"
#include "losu_object.h"
#include <ctype.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef DEFINE_SOURCE_LOSU_OBJECT_C_OBJECT
#define DEFINE_SOURCE_LOSU_OBJECT_C_OBJECT

const LosuObj _inlineNullObj = {
  .type = LosuTypeDefine_null,
  .value = { NULL },
};
const LosuObj _inlineSpaceObj = {
  .type = LosuTypeDefine_space,
  .value = { NULL },
};
const LosuObj _inlineTrueObj = {
  .type = LosuTypeDefine_bool,
  .value.boolnum = 1,
};
const LosuObj _inlineFalseObj = {
  .type = LosuTypeDefine_bool,
  .value.boolnum = 0,
};

LosuWeak LosuVm *Ec2WasmVM = NULL;

_l_bool
__losu_object_isObjEqual (const LosuObj *t1, const LosuObj *t2)
{
  if (!t1 || !t2)
    return 0;
  if (ovtype (t1) != ovtype (t2))
    {
      _l_bool b1 = 0, b2 = 0;

      if (ovtype (t1) == LosuTypeDefine_bool)
        {
          b1 = ovbool (t1);
          b2 = ovtype (t2) != LosuTypeDefine_null ? 1 : 0;
          return b1 == b2;
        }
      else if (ovtype (t2) == LosuTypeDefine_bool)
        {
          b2 = ovbool (t2);
          b1 = ovtype (t1) != LosuTypeDefine_null ? 1 : 0;
          return b1 == b2;
        }
      return 0;
    }
  else
    {
      switch (ovtype (t1))
        {
        case LosuTypeDefine_null:
          {
            return 1;
            break;
          }
        case LosuTypeDefine_number:
          {
            return ovnumber (t1) == ovnumber (t2);
            break;
          }
        case LosuTypeDefine_string:
          {
            if (ovIstr (t1)->hash != ovIstr (t2)->hash)
              return 0;
            if (ovIstr (t1)->len != ovIstr (t2)->len)
              return 0;
            return !memcmp (ovSstr (t1), ovSstr (t2), ovIstr (t1)->len);
            break;
          }
        case LosuTypeDefine_function:
          {
            return ovfunc (t1) == ovfunc (t2);
            break;
          }
        case LosuTypeDefine_unit:
          {
            return ovhash (t1) == ovhash (t2);
            break;
          }
        case LosuTypeDefine_space:
          {
            return 1;
            break;
          }
        case LosuTypeDefine_bytes:
          {
            return ovIstr (t1) == ovIstr (t2);
            break;
          }
        case LosuTypeDefine_int:
          {
            // printf ("debug(1)%d\t%d\n", ovint (t1), ovint (t2));
            return t1->value.intnum == t2->value.intnum;
            break;
          }
        case LosuTypeDefine_unicode:
          {
            return t1->value.unicode == t2->value.unicode;
            break;
          }
        case LosuTypeDefine_bool:
          {
            return ovbool (t1) == ovbool (t2);
            break;
          }
        default:
          return 0;
        }
    }
}

_l_bool
__losu_object_isObjLess (const LosuObj *t1, const LosuObj *t2)
{
  if (ovtype (t1) != ovtype (t2))
    return obj_tonum (Ec2WasmVM, (LosuObj *)t1)
           < obj_tonum (Ec2WasmVM, (LosuObj *)t2);
  switch (ovtype (t1))
    {
    case LosuTypeDefine_number:
      return ovnumber (t1) < ovnumber (t2);
    case LosuTypeDefine_int:
      return ovint (t1) < ovint (t2);
    case LosuTypeDefine_unicode:
      return ovunicode (t1) < ovunicode (t2);
    case LosuTypeDefine_string:
    case LosuTypeDefine_bytes:
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
      vm_error (Ec2WasmVM, "'%s' 与 '%s' 类型不能进行关系比较",
                obj_typeStr (Ec2WasmVM, (LosuObj *)t1),
                obj_typeStr (Ec2WasmVM, (LosuObj *)t2));
      return 0;
      break;
    }
  return 0;
}
const LosuObj *
__losu_objUnit_get (_inlineHash *t, LosuObj *key)
{
  switch (ovtype (key))
    {
    case LosuTypeDefine_null:
      return &_inlineNullObj;
    case LosuTypeDefine_number:
      return __losu_objUnit_getnum (t, ovnumber (key));
    case LosuTypeDefine_string:
      return __losu_objUnit_getstr (t, ovIstr (key));
    default:
      return __losu_objUnit_getany (t, key);
    }
}

#endif

#endif