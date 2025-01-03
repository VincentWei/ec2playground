#include "ec2_wasm.h"
#include "losu_malloc.h"
#include "losu_object.h"
#include <emscripten.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
LosuExtern const char *unit2str (LosuVm *vm, LosuObj *obj);

// IO: 输入输出
// 输出(s) 输出 s 的字符串格式，非字符串类型输出地址
int32_t
wasm_libcore_io_print (LosuVm *vm)
{
  int32_t n = arg_num (vm);
  for (int32_t i = 0; i < n; i++)
    {
      LosuObj *o = arg_get (vm, i + 1);
      printf ("%s", obj_tostr (vm, o));
    }
  printf ("\n");
  return 0;
}
// 输出(提示)
int32_t
wasm_libcore_io_input (LosuVm *vm)
{
  char text[1024] = { 0 };
  snprintf (text, 1024, "wasmIOinput('%s')", obj_tostr (vm, arg_get (vm, 1)));
  arg_return (vm, obj_newstr (vm, emscripten_run_script_string (text)));
  return 1;
}
// 终止() 终止算法执行, throw yield
int32_t
wasm_libcore_io_exit (LosuVm *vm)
{
  __losu_sigThrow (vm, VMSIGYIELD);
  return 0;
}

// 数据类型
// 整数
int32_t
wasm_libcore_type_int (LosuVm *vm)
{
  arg_return (vm, obj_newint (vm, obj_toint (vm, arg_get (vm, 1))));
  return 1;
}
// 浮点数
int32_t
wasm_libcore_type_number (LosuVm *vm)
{
  arg_return (vm, obj_newnum (vm, obj_tonum (vm, arg_get (vm, 1))));
  return 1;
}
// 字符
int32_t
wasm_libcore_type_unicode (LosuVm *vm)
{
  arg_return (vm, obj_newunicode (vm, obj_tounicode (vm, arg_get (vm, 1))));
  return 1;
}
// 字符串
int32_t
wasm_libcore_type_string (LosuVm *vm)
{
  arg_return (vm, obj_newstr (vm, (char *)obj_tostr (vm, arg_get (vm, 1))));
  return 1;
}
// 字符串：字符序列
int32_t
wasm_libcore_type_string_unilist (LosuVm *vm)
{
  LosuObj newlist = obj_newunit (vm, 0);
  const char *str = obj_tostr (vm, arg_get (vm, 1));
  int32_t n = 0;
  while (str[0] != '\0')
    {
      int32_t len = charset_utf8len ((uint8_t)str[0]);
      _l_unicode u = 0;
      for (int i = 0; i < len; i++)
        u = (u << 8) + (uint8_t)str[i];
      obj_setunitbynum (vm, newlist, n++, obj_newunicode (vm, u));
      str += len;
    }
  arg_return (vm, newlist);
  return 1;
}
// 字符串：分割
int32_t
wasm_libcore_type_string_strtok (LosuVm *vm)
{
  char tmp[5] = { 0 };
  LosuObj newlist = obj_newunit (vm, 0);
  const char *str = obj_tostr (vm, arg_get (vm, 1));
  int32_t n = 0;
  while (str[0] != '\0')
    {
      int32_t len = charset_utf8len ((uint8_t)str[0]);
      _l_unicode u = 0;
      memset (tmp, 0, 5);
      memcpy (tmp, str, len);
      obj_setunitbynum (vm, newlist, n++, obj_newstr (vm, tmp));
      str += len;
    }
  arg_return (vm, newlist);
  return 1;
}
// 映射：键序列
int32_t
wasm_libcore_type_map_keylist (LosuVm *vm)
{
  LosuObj *mapObj = arg_get (vm, 1);
  if (strcmp (obj_typeStr (vm, mapObj), "映射"))
    vm_error (vm, " '%s' 类型没有键序列", obj_typeStr (vm, mapObj));
  LosuObj newlist = obj_newunit (vm, 0);
  LosuNode *n = obj_unit_first (vm, *mapObj);
  for (int i = 0; n; i++)
    {
      obj_setunitbynum (vm, newlist, i, n->key);
      n = obj_unit_next (vm, *mapObj, n);
    }
  arg_return (vm, newlist);
  return 1;
}
// 映射：值序列
int32_t
wasm_libcore_type_map_vallist (LosuVm *vm)
{
  LosuObj *mapObj = arg_get (vm, 1);
  if (strcmp (obj_typeStr (vm, mapObj), "映射"))
    vm_error (vm, " '%s' 类型没有值序列", obj_typeStr (vm, mapObj));
  LosuObj newlist = obj_newunit (vm, 0);
  LosuNode *n = obj_unit_first (vm, *mapObj);
  for (int i = 0; n; i++)
    {
      obj_setunitbynum (vm, newlist, i, n->value);
      n = obj_unit_next (vm, *mapObj, n);
    }
  arg_return (vm, newlist);
  return 1;
}
// 映射/徐磊：转储
int32_t
wasm_libcore_type_unit_tostr (LosuVm *vm)
{
  LosuObj *obj = arg_get (vm, 1);
  if (obj_type (vm, obj) != LosuTypeDefine_unit)
    vm_error (vm, " '%s' 类型没有转储", obj_typeStr (vm, obj));
  arg_return (vm, obj_newstr (vm, (char *)unit2str (vm, obj)));
  return 1;
}
// 映射/序列：解析
int32_t
wasm_libcore_type_unit_tounit (LosuVm *vm)
{
  LosuObj *oldtop = vm->top;
  char tmp[1024] = { 0 };
  sprintf (tmp, "返回 %s\n", obj_tostr (vm, arg_get (vm, 1)));
  int32_t sta = vm_loadstring (vm, tmp, "");
  if (sta) /* error */
    return 0;
  stack_call (vm, 0, 1);
  LosuObj rt = *(vm->top - 1);
  vm->top = oldtop;
  arg_return (vm, rt);
  return 1;
}
// 混合：长度
int32_t
wasm_libcore_type_any_len (LosuVm *vm)
{
  LosuObj *o = arg_get (vm, 1);
  switch (obj_type (vm, o))
    {
    case LosuTypeDefine_null:
      {
        arg_return (vm, obj_newint (vm, 0));
        break;
      }
    case LosuTypeDefine_bool:
      {
        arg_return (vm, obj_newint (vm, 1));
        break;
      }
    case LosuTypeDefine_number:
      {
        arg_return (vm, obj_newint (vm, sizeof (_l_number)));
        break;
      }
    case LosuTypeDefine_int:
      {
        arg_return (vm, obj_newint (vm, sizeof (_l_int)));
        break;
      }
    case LosuTypeDefine_unicode:
      {
        arg_return (vm, obj_newint (vm, sizeof (_l_unicode)));
        break;
      }
    case LosuTypeDefine_string:
      {
        int32_t len = 0;
        const char *str = obj_tostr (vm, o);
        while (*str != '\0')
          {
            int l = charset_utf8len ((uint8_t)str[0]);
            str += l;
            len++;
          }
        arg_return (vm, obj_newint (vm, len));
        break;
      }
    case LosuTypeDefine_unit:
      {
        if (ovhash (o)->isMap) // iaMap, can't length
          vm_error (vm, " '%s' 类型不能判断长度", obj_typeStr (vm, o));
        int len = 0;
        while (obj_type (vm, obj_indexunitbynum (vm, *o, len))
               != LosuTypeDefine_null)
          len++;
        arg_return (vm, obj_newint (vm, len));
        break;
      }
    default:
      {
        vm_error (vm, " '%s' 类型不能判断长度", obj_typeStr (vm, o));
        break;
      }
    }
  return 1;
}
// 类型
int32_t
wasm_libcore_type_type (LosuVm *vm)
{
  arg_return (vm, obj_newstr (vm, (char *)obj_typeStr (vm, arg_get (vm, 1))));
  return 1;
}

// 功能调用
// 执行
int32_t
wasm_libcore_exec (LosuVm *vm)
{
  char tmp[1024] = { 0 };
  const char *cmd = obj_tostr (vm, arg_get (vm, 1));
  if (!strcmp (cmd, "拍手"))
    {
      emscripten_run_script ("wasmIOclapshow();");
      emscripten_sleep (1000);
      emscripten_run_script ("wasmIOclapclose();");
    }
  else if (!strcmp (cmd, "说出"))
    {
      sprintf (tmp, "wasmIOspeek('%s')", obj_tostr (vm, arg_get (vm, 2)));
      emscripten_run_script (tmp);
    }
  return 0;
}
// 拷贝
int32_t
wasm_libcore_copy (LosuVm *vm)
{
  LosuObj *obj = arg_get (vm, 1);
  if (ovtype (obj) != LosuTypeDefine_unit)
    vm_error (vm, " '%s' 类型不允许拷贝操作", obj_typeStr (vm, obj));
  LosuObj new = obj_newunit (vm, 0);
  if (ovhash (obj)->isMap) // map
    {
      ovhash (&new)->isMap = 1;
      LosuNode *n = obj_unit_first (vm, *obj);
      while (n)
        {
          obj_setunit (vm, new, n->key, n->value);
          n = obj_unit_next (vm, *obj, n);
        }
    }
  else // list
    {
      ovhash (&new)->isMap = 0;
      int i = 0;
      while (1)
        {
          LosuObj *v = obj_indexunitbynum (vm, *obj, i);
          if (ovtype (v) == LosuTypeDefine_null)
            break;
          obj_setunitbynum (vm, new, i, *v);
          i++;
        }
    }
  arg_return (vm, new);
  return 1;
}

int32_t
wasm_libcore_assert (LosuVm *vm)
{
  _l_bool b = obj_tobool (vm, arg_get (vm, 1));
  const char *s = obj_tostr (vm, arg_get (vm, 2));
  if (!b)
    vm_error (vm, "%s", s);
  return 0;
}

static struct
{
  const char *name;
  LosuApi func;
} libfunc[] = {
  { "输出", wasm_libcore_io_print },
  { "输入", wasm_libcore_io_input },
  { "执行", wasm_libcore_exec },
  { "终止", wasm_libcore_io_exit },
  { "整数", wasm_libcore_type_int },
  { "浮点数", wasm_libcore_type_number },
  { "字符", wasm_libcore_type_unicode },
  { "字符串", wasm_libcore_type_string },
  { "字符序列", wasm_libcore_type_string_unilist },
  { "分割", wasm_libcore_type_string_strtok },
  { "键序列", wasm_libcore_type_map_keylist },
  { "值序列", wasm_libcore_type_map_vallist },
  { "转储", wasm_libcore_type_unit_tostr },
  { "解析", wasm_libcore_type_unit_tounit },
  { "长度", wasm_libcore_type_any_len },
  { "类型", wasm_libcore_type_type },
  // { "拷贝", wasm_libcore_copy },
  { "断言", wasm_libcore_assert },

};

void
wasm_libcore__init__ (LosuVm *vm)
{
  for (int i = 0; i < sizeof (libfunc) / sizeof (libfunc[0]); i++)
    vm_setval (vm, libfunc[i].name, obj_newfunction (vm, libfunc[i].func));
}