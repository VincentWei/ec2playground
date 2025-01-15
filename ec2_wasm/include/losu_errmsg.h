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

/* This file defines all of Losu's error messages. */

#ifndef DEFINE_INCLUDE_LOSU_ERRMSG_H
#define DEFINE_INCLUDE_LOSU_ERRMSG_H

#define __config_losucore_errmsg_msgVectorOverflow "数组栈溢出"
#define __config_losucore_errmsg_msgReallocFailed                             \
  "内存重新分配内存时发生错误，地址 '%p' , 大小 '%zu' B"

#define __config_losucore_errmsg_msgInvalidNumber "错误的数字格式 '%s'"
#define __config_losucore_errmsg_msgMissStrEnd                                \
  "缺少字符 %c "
#define __config_losucore_errmsg_msgInvalidEscapeHex                          \
  "%% or \\u 后需要十六进制数"

#define __config_losucore_errmsg_msgInvalidEscapeUtf8                         \
  "错误的 UTF-8 字符"
#define __config_losucore_errmsg_msgInvalidBlockComment "错误的块注释"
#define __config_losucore_errmsg_msgInvalidPureString "纯字符串不能包含换行符"
#define __config_losucore_errmsg_msgUnknownSymbol "未知的符号 '%s'"
#define __config_losucore_errmsg_msgMissingEOZ "缺少文件尾"
#define __config_losucore_errmsg_msgCheckMatch                                \
  "'%s' 后 (第 %d 行) 需要 '%s'"
#define __config_losucore_errmsg_msgCheckName                                 \
  "变量名需要 [NAME] 关键词"
#define __config_losucore_errmsg_msgInvalidExp "错误的表达式"
#define __config_losucore_errmsg_msgParserError "缺少 '%s'"
#define __config_losucore_errmsg_msgInvalidForExp "错误的 for 循环表达式"
#define __config_losucore_errmsg_msgCheckLimit "%s 溢出 ( 限制在 %d)"
#define __config_losucore_errmsg_msgCheckLimitTMlocvar "局部变量数量"
#define __config_losucore_errmsg_msgInvalidClosure                            \
  "错误的闭包 `%s` (必须在全局作用域或相邻作用域中)."
#define __config_losucore_errmsg_msgExpectedFarg "缺少函数参数"
#define __config_losucore_errmsg_msgTooComplexExp "表达式太复杂"
#define __config_losucore_errmsg_msgInvalidJump "跳转指令越界"
#define __config_losucore_errmsg_msgInvalidUnitIndex "无效的键值"
#define __config_losucore_errmsg_msgStackOverflow "虚拟机堆栈溢出"
#define __config_losucore_errmsg_msgCalledNotFunc                             \
  "被调用的对象不是函数"
#define __config_losucore_errmsg_msgInvalidOverload                           \
  " Operators '%s' are not defined how to overload for '%s' and '%s'"
#define __config_losucore_errmsg_msgInvalidForstep "Invalid for 'step'"
#define __config_losucore_errmsg_msgInvalidFormax "Invalid for 'max'"
#define __config_losucore_errmsg_msgInvalidForinit "Invalid for 'init'"
#define __config_losucore_errmsg_msgInvalidForobj "Invalid for 'obj'"
#define __config_losucore_errmsg_msgInvalidBytecode "错误的字节码"
#define __config_losucore_errmsg_msgInvalidSetobj                             \
  "试图向非索引对象进行下标赋值"
#define __config_losucore_errmsg_msgStrlenOverflow "字符串长度溢出"
#define __config_losucore_errmsg_msgInvalidNewobj                             \
  "Invalid new object,The first argument to the `new' function must be a "    \
  "unit type."
#define __config_losucore_errmsg_msgInvalidLibrary "Can't find the module '%s'"
#define __config_losucore_errmsg_msgInvalidSubLibrary                         \
  "Can't find the '%s' in module '%s'"
#define __config_losucore_errmsg_msgIllegalBytecodeFormat                     \
  "Illegal bytecode format, further execution is denied"
#define __config_losucore_errmsg_msgInvalidVmBytecodeVersion                  \
  "Invalid bytecode version, need '%d', but '%d' is given"
#define __config_losucore_errmsg_msgInvalidVmInsSize                          \
  "Invalid Vm-Ins size, need %d-bit, but %d-bit is given"
#define __config_losucore_errmsg_msgInvalidIdtMutiple                         \
  "Invalid indentation, need Multiples of '%d',but '%d' is given"
#define __config_losucore_errmsg_msgInvalidElif                               \
  "错误的表达式，缺少 '若又'"
#define __config_losucore_errmsg_msgInvalidElse                               \
  "错误的表达式，缺少 '若否'"
#define __config_losucore_errmsg_msgfileNotFound "File not found '%s'"

#endif /* DEFINE_INCLUDE_LOSU_ERRMSG_H */

#ifndef DEFINE_INCLUDE_LOSU_ERRMSG_H
#define DEFINE_INCLUDE_LOSU_ERRMSG_H

#define __config_losucore_errmsg_msgVectorOverflow "Vector overflow"
#define __config_losucore_errmsg_msgReallocFailed                             \
  "Fails to reallocate memory successfully when reallocating 0x%p  to %zu "   \
  "size(B) "

#define __config_losucore_errmsg_msgInvalidNumber "Invalid number '%s'"
#define __config_losucore_errmsg_msgMissStrEnd                                \
  "Missing terminating  character %c "
#define __config_losucore_errmsg_msgInvalidEscapeHex                          \
  "%% or \\u used with no following hex digits"

#define __config_losucore_errmsg_msgInvalidEscapeUtf8                         \
  "Invalid UTF-8 escape sequence"
#define __config_losucore_errmsg_msgInvalidBlockComment "Invalid block comment"
#define __config_losucore_errmsg_msgInvalidPureString "Invalid string after $ "
#define __config_losucore_errmsg_msgUnknownSymbol "Unknown symbol '%s'"
#define __config_losucore_errmsg_msgMissingEOZ "Missing EOZ"
#define __config_losucore_errmsg_msgCheckMatch                                \
  "After '%s'(at line %d) expected '%s'"
#define __config_losucore_errmsg_msgCheckName                                 \
  "Variable name  required as [TOKEN_NAME]"
#define __config_losucore_errmsg_msgInvalidExp "Invalid expression"
#define __config_losucore_errmsg_msgParserError "Expected '%s'"
#define __config_losucore_errmsg_msgInvalidForExp "Invalid 'for' expression"
#define __config_losucore_errmsg_msgCheckLimit "Too mant %s (limit %d)"
#define __config_losucore_errmsg_msgCheckLimitTMlocvar "local variables"
#define __config_losucore_errmsg_msgInvalidClosure                            \
  "Invalid closure `%s` (must be global or in neighboring scope)."
#define __config_losucore_errmsg_msgExpectedFarg "Expected function argument"
#define __config_losucore_errmsg_msgTooComplexExp "Overly complex expressions"
#define __config_losucore_errmsg_msgInvalidJump "Invalid jump, overflow"
#define __config_losucore_errmsg_msgInvalidUnitIndex "Invalid unit index"
#define __config_losucore_errmsg_msgStackOverflow "Stack overflow"
#define __config_losucore_errmsg_msgCalledNotFunc                             \
  "The object being called is not a function type"
#define __config_losucore_errmsg_msgInvalidOverload                           \
  " Operators '%s' are not defined how to overload for '%s' and '%s'"
#define __config_losucore_errmsg_msgInvalidForstep "Invalid for 'step'"
#define __config_losucore_errmsg_msgInvalidFormax "Invalid for 'max'"
#define __config_losucore_errmsg_msgInvalidForinit "Invalid for 'init'"
#define __config_losucore_errmsg_msgInvalidForobj "Invalid for 'obj'"
#define __config_losucore_errmsg_msgInvalidBytecode "Invalid bytecode"
#define __config_losucore_errmsg_msgInvalidSetobj                             \
  "Attempting to assign a value to a non-indexed unit"
#define __config_losucore_errmsg_msgStrlenOverflow "String length overflow"
#define __config_losucore_errmsg_msgInvalidNewobj                             \
  "Invalid new object,The first argument to the `new' function must be a "    \
  "unit type."
#define __config_losucore_errmsg_msgInvalidLibrary "Can't find the module '%s'"
#define __config_losucore_errmsg_msgInvalidSubLibrary                         \
  "Can't find the '%s' in module '%s'"
#define __config_losucore_errmsg_msgIllegalBytecodeFormat                     \
  "Illegal bytecode format, further execution is denied"
#define __config_losucore_errmsg_msgInvalidVmBytecodeVersion                  \
  "Invalid bytecode version, need '%d', but '%d' is given"
#define __config_losucore_errmsg_msgInvalidVmInsSize                          \
  "Invalid Vm-Ins size, need %d-bit, but %d-bit is given"
#define __config_losucore_errmsg_msgInvalidIdtMutiple                         \
  "Invalid indentation, need Multiples of '%d',but '%d' is given"
#define __config_losucore_errmsg_msgInvalidElif                               \
  "Invalid if statement, unexpected 'elif'"
#define __config_losucore_errmsg_msgInvalidElse                               \
  "Invalid if statement, unexpected 'else'"
#define __config_losucore_errmsg_msgfileNotFound "File not found '%s'"

#endif /* DEFINE_INCLUDE_LOSU_ERRMSG_H */
