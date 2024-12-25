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
Losu Gc, a garbage collector for Losu Automated Memory Management
*/

#ifndef DEFINE_INCLUDE_LOSU_GC_H
#define DEFINE_INCLUDE_LOSU_GC_H
#include "losu.h"

/**
 * @brief Losu Gc, a garbage collector for Losu Automated Memory Management
 * @param vm LosuVm
 * @param all If true, all objects will be collected, otherwise only objects
 * that are not referenced will be collected
 */
void __losu_gc_collect (LosuVm *vm, _l_bool all);

/**
 * @brief Check if the client is ready to be collected
 * @param vm LosuVm
 * @return _l_bool 1 if collection operation performed, 0 if collection
 * operation not performed
 */
_l_bool __losu_gc_checkClt (LosuVm *vm);

/* data struct */
typedef struct _inlineGc
{
  _inlineHash *unit;     /* tmark */
  _inlineFunction *func; /* cmark */
} _inlineGc;

#endif
