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
 * consisting of vmHeap, vmHost,vmCore
 */

#ifndef DEFINE_INCLUDE_LOSU_VM_H
#define DEFINE_INCLUDE_LOSU_VM_H

#include "losu.h"

#ifndef DEFINE_INCLUDE_LOSU_VM_H_VMHEAP
#define DEFINE_INCLUDE_LOSU_VM_H_VMHEAP

void __losu_vmHeap_init (LosuVm *vm, int32_t size);
void __losu_vmHeap_check (LosuVm *vm, int32_t size);
/* void __losu_vmHeap_break (LosuVm *vm, int16_t ecode); */
void __losu_vmHeap_adjTop (LosuVm *vm, LosuObj *base, int32_t size);
int32_t __losu_vmHeap_callS (LosuVm *vm, int32_t arg, int32_t nres);
/* int32_t __losu_vmHeap_callS (LosuVm *vm, int32_t arg, int32_t nres);
void __losu_vmHeap_call (LosuVm *vm, LosuObj *func, int32_t nres); */
void __losu_vmHeap_rawcall (LosuVm *vm, LosuObj *func, int32_t nres);
void __losu_vmHeap_procall (LosuVm *vm, LosuObj *func, int32_t nres);
void __losu_vmHeap_call (LosuVm *vm, _inlineCallinfo *cinfo);

#endif /* DEFINE_INCLUDE_LOSU_VM_H_VMHEAP */

#ifndef DEFINE_INCLUDE_LOSU_VM_H_VMCORE
#define DEFINE_INCLUDE_LOSU_VM_H_VMCORE
LosuObj *__losu_vmCore_exec (LosuVm *vm, _inlineCallinfo *cinfo,
                             LosuObj *recall);
_l_bool __losu_vmCore_Tonum (LosuVm *vm, LosuObj *obj, _l_bool isCore);
_l_bool __losu_vmCore_Tostr (LosuVm *vm, LosuObj *obj, _l_bool isCore);

#endif /* DEFINE_INCLUDE_LOSU_VM_H_VMCORE */



#endif /* DEFINE_INCLUDE_LOSU_VM_H */
