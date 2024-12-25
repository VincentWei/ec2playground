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

#ifndef DEFINE_SOURCE_LOSU_MALLOC_C
#define DEFINE_SOURCE_LOSU_MALLOC_C
#include "losu_malloc.h"
#include "losu.h"
#include "losu_errmsg.h"
#include <stdio.h>
#include <stdlib.h>

/**
 * @brief realloc memory, if old is NULL, it is equivalent to malloc, if size
 * is 0, it is equivalent to free
 * @param vm LosuVm
 * @param old old pointer
 * @param size new size
 * @return new pointer, if failed, return NULL
 */
void *
__losu_mem_realloc (LosuVm *vm, void *old, _l_size_t size)
{
  void *new = NULL;
  if (size == 0)
    {
      if (old != NULL)
        __config_losucore_mem_free (old);
      return NULL;
    }

  new = (void *)(__config_losucore_mem_realloc (old, size));
  if (new == NULL)
    {
      if (vm)
        vm_error (vm, __config_losucore_errmsg_msgReallocFailed, old, size);
      return NULL;
    }
  return new;
}

void *
__losu_mem_grow (LosuVm *vm, void *block, _l_size_t i_num, int inc,
                 _l_size_t size, _l_size_t limit, const char *errormsg)
{
  /*
  size_t newn = issues_num + inc;
  if (issues_num >= limit - inc)
    vm_error (vm, errormsg, ELS_ERRORBACK_RUN);
  if ((newn ^ issues_num) <= issues_num || (issues_num > 0 && newn < 4))
    return block;
  else
    return els_Mem_realloc (vm, block, els_Object_power2 (newn) * size);
  */

  _l_size_t newn = i_num + inc;
  if (newn >= limit)
    vm_error (vm, errormsg);
  return __losu_mem_realloc (vm, block, newn * size);
}
#endif