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
This set of source code defines the memory management functions for Lowe's
(based on malloc@libc by default), which can be changed to use a specific
memory manager (e.g. bget)
 */

#ifndef DEFINE_INCLUDE_LOSU_MALLOC_H
#define DEFINE_INCLUDE_LOSU_MALLOC_H
#include "losu.h"

#ifndef __config_losucore_mem_realloc
#define __config_losucore_mem_realloc realloc
#endif

#ifndef __config_losucore_mem_free
#define __config_losucore_mem_free free
#endif

void *__losu_mem_realloc (LosuVm *vm, void *old, _l_size_t size);

void *__losu_mem_grow (LosuVm *vm, void *block, _l_size_t i_num, int inc,
                       _l_size_t size, _l_size_t limit, const char *errormsg);

#define __losu_mem_malloc(vm, s) (__losu_mem_realloc ((vm), NULL, (s)))
#define __losu_mem_free(vm, p) (__losu_mem_realloc ((vm), (p), 0))
#define __losu_mem_new(vm, t) ((t *)__losu_mem_malloc ((vm), sizeof (t)))
#define __losu_mem_newvector(vm, n, t)                                        \
  ((t *)__losu_mem_malloc ((vm), (n) * sizeof (t)))

#define __losu_mem_growvector(vm, v, n, i, t, l, e)                           \
  ((v) = (t *)__losu_mem_grow ((vm), (v), (n), (i), sizeof (t), (l), (e)))

#define __losu_mem_reallocvector(vm, v, s, t)                                 \
  ((v) = (t *)__losu_mem_realloc ((vm), (v), (s) * sizeof (t)))
#endif
