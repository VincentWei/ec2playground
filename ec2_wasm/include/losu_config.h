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
-------------- How to configure the Losu Core ----------------
  1. The configuration file is named 'losu_config.h'.
  2. You can use predefined OS macros to build ELF for officially supported
    platforms
  3. You can also define your own macros to build ELF for other platforms
---- You can refer to the following configuration tree ----

__config_losucore_
  platform_
    name: "xxxx"            // the name of the platform [os]-[generic]
    template:
      linux=        1       // the template for linux
      windows=      2       // the template for windows
      embedded=     3       // the template for embedded
      ......
  vm_
    bitsize:        32/64   // the bitsize of the virtual machine
    iobuff:         32      // io size of the virtual machine, default is 32
    vmbuff:         256     // static buff size of the Vm default is 256
    strpoolMax:     1024    // max size of the string pool
    corepkg:        1/0     // enable/disable the full compiler
    stacksize:      1024    // max stack-size of the stack
    corosize:       1024    // max coro-size of the coro
  mem_
    realloc:        realloc // enable/disable the realloc function
    free:           free    // enable/disable the free function
  gc_
    defaultsize:    0       // default size of the GC max
  feature_
    chKeyword:      1/0     // enable/disable Chinese keywords
    cStyle:         1/0     // enable/disable C style tokens
  encode_           1/0     // only one encode can be enabled
    winGBK:         1/0     // enable/disable GBK encoding(need Win32 API)
    nativeUTF8:     1/0     // enable/disable native UTF-8 encoding
  pkgs_[name]
 */

#ifndef DEFINE_INCLUDE_LOSUCONFIG_H
#define DEFINE_INCLUDE_LOSUCONFIG_H

/* Notice! - define this Marco before compiler */
// #define __config_losucore_platform_name ""
// define config 
#define __config_losucore_platform_template 0

#ifndef __config_losucore_platform_name
#define __config_losucore_platform_name "wasm32-unknown-emscripten"
#endif

#define __config_losucore_vm_bitsize 64
#define __config_losucore_vm_iobuff 1024
#define __config_losucore_vm_vmbuff 256
#define __config_losucore_vm_strpoolMax 1024
// #define __config_losucore_vm_corepkg 1 // enable core package
#define __config_losucore_vm_stacksize 1024
#define __config_losucore_vm_corosize __config_losucore_vm_stacksize

#define __config_mem_realloc realloc
#define __config_mem_free free

#define __config_losucore_gc_defaultsize 1024 * 1024 * 128 /* 128M */

#define __config_losucore_feature_chKeyword 1
#define __config_losucore_feature_cStyle 1

#define __config_losucore_encode_nativeUTF8 1




/* default is linux*/
#ifndef __config_losucore_platform_template
#define __config_losucore_platform_template 1
#endif

#if __config_losucore_platform_template < 0
#error "the platform template must be greater than 0"
#endif

#if __config_losucore_platform_template == 1
#ifndef __config_losucore_platform_name
#define __config_losucore_platform_name "unknown-linux-unknown"
#endif

#define __config_losucore_vm_bitsize 64
#define __config_losucore_vm_iobuff 1024
#define __config_losucore_vm_vmbuff 256
#define __config_losucore_vm_strpoolMax 1024
#define __config_losucore_vm_corepkg 1
#define __config_losucore_vm_stacksize 1024
#define __config_losucore_vm_corosize __config_losucore_vm_stacksize

#define __config_mem_realloc realloc
#define __config_mem_free free

#define __config_losucore_gc_defaultsize 1024 * 1024 * 128 /* 128M */

#define __config_losucore_feature_chKeyword 1
#define __config_losucore_feature_cStyle 1

#define __config_losucore_encode_nativeUTF8 1
/* #define __config_losucore_encode_winGBK 1 */

#endif

#if __config_losucore_platform_template == 2

#ifndef __config_losucore_platform_name
#define __config_losucore_platform_name "unknown-windows-unknown"
#endif

#define __config_losucore_vm_bitsize 64
#define __config_losucore_vm_iobuff 1024
#define __config_losucore_vm_vmbuff 256
#define __config_losucore_vm_strpoolMax 1024
#define __config_losucore_vm_fullcompiler 1
#define __config_losucore_vm_stacksize 1024
#define __config_losucore_vm_corosize __config_losucore_vm_stacksize

#define __config_mem_realloc realloc
#define __config_mem_free free

#define __config_losucore_gc_defaultsize 1024 * 1024 * 128 /* 128M */

#define __config_losucore_feature_chKeyword 1
#define __config_losucore_feature_cStyle 1

#define __config_losucore_encode_winGBK 1

#endif

#endif
