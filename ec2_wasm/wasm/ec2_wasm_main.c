#include "ec2_wasm.h"
#include "emscripten/emscripten.h"
#include <stdio.h>
#include <string.h>
#include <time.h>

wasmLibApi wasmLibList[] = { wasm_libcore__init__, wasm_libmath__init__ };

static int32_t
test (LosuVm *vm)
{
  fprintf (stdout, "%s\n", obj_tostr (vm, arg_get (vm, 1)));
  return 0;
}
LosuExtern const char *unit2str (LosuVm *vm, LosuObj *obj);
void
wasmIOrunCode ()
{
  emscripten_run_script ("WasmMutex.runLock = 1;");
  char str[128] = { 0 };
  LosuVm *vm = Ec2WasmVM = vm_create (1024);
  gc_setmax (vm, 1024 * 1024); // 1 M
  // init lib
  for (int32_t i = 0; i < sizeof (wasmLibList) / sizeof (wasmLibApi); i++)
    wasmLibList[i](vm);
  // vm_setval (vm, "输出", obj_newfunction (vm, test));
  // do script

  // find 'main'
  if (vm_dostring (vm, emscripten_run_script_string ("wasmIOgetCode()"),
                   "Ec2-Playground")
          == 0
      && vm->main.main)
    {
      clock_t start, end;
      double cpu_time_used;
      int narg = vm->main.main->value.func->func.sdef->narg;
      __losuvmJmp *oldjmp = vm->errjmp;
      __losuvmJmp newjmp;
      vm->errjmp = &newjmp;
      stack_push (vm, *vm->main.main);
      for (int i = 0; i < narg; i++)
        {
          sprintf (str, "wasmIOinput('算法: %s 请输入第 %d 个参数: %s')",
                   vm->main.funcname, i + 1, vm->main.funcargs[i]);
          if (emscripten_run_script_int ("WasmMutex.runLock") == 0)
            break;
          stack_push (vm, obj_newstr (vm, emscripten_run_script_string (str)));
        }
      start = clock ();
      int sta = setjmp (newjmp.jmpflag);
      switch (sta)
        {
        case 0:
          {
            if (emscripten_run_script_int ("WasmMutex.runLock") == 0)
              __losu_sigThrow (vm, VMSIGYIELD);
            stack_call (vm, narg, 1);
            end = clock ();
            cpu_time_used = ((double)(end - start)) / CLOCKS_PER_SEC;
            // output log
            fprintf (stdout, "--------------------------------\n");
            if (obj_type (vm, vm->top - 1) == LosuTypeDefine_unit)
              fprintf (stdout, "返回值:\t%s\n",
                       (char *)unit2str (vm, vm->top - 1));
            else
              fprintf (stdout, "返回值:\t%s\n", obj_tostr (vm, vm->top - 1));
            fprintf (
                stdout, "统计\n\t运算:\t%lld\n\t调用:\t%lld\n\t循环:\t%lld\n",
                vm->aluhook - 1 > 0 ? vm->aluhook - 1 : 0,
                vm->callhook - 2 > 0 ? vm->callhook - 2 : 0, vm->loophook);
            fprintf (stdout, "CPU时间:\t%.5fs\n", cpu_time_used);
            if (gc_getmemMax (vm) > 1024 * 1024 * 1024)
              fprintf (stdout, "最大内存占用:\t %.5f GB\n",
                       (double)gc_getmemMax (vm) / 1024 / 1024 / 1024);
            else if (gc_getmemMax (vm) > 1024 * 1024) // output xxM
              fprintf (stdout, "最大内存占用:\t %.5f MB\n",
                       (double)gc_getmemMax (vm) / 1024 / 1024);
            else
              fprintf (stdout, "最大内存占用:\t %.5f KB\n",
                       (double)gc_getmemMax (vm) / 1024);
            fprintf (stdout, "--------------------------------\n");
            break;
          }
        case VMSIGYIELD:
          {
            end = clock ();
            cpu_time_used = ((double)(end - start)) / (double)CLOCKS_PER_SEC;
            // output log
            fprintf (stdout, "--------------------------------\n");
            fprintf (stdout, "程序被终止\n");
            fprintf (
                stdout, "统计\n\t运算:\t%lld\n\t调用:\t%lld\n\t循环:\t%lld\n",
                vm->aluhook - 1 > 0 ? vm->aluhook - 1 : 0,
                vm->callhook - 2 > 0 ? vm->callhook - 2 : 0, vm->loophook);
            fprintf (stdout, "CPU时间:\t%.5fs\n", cpu_time_used);
            if (gc_getmemMax (vm) > 1024 * 1024 * 1024)
              fprintf (stdout, "最大内存占用:\t %.5f GB\n",
                       (double)gc_getmemMax (vm) / 1024 / 1024 / 1024);
            else if (gc_getmemMax (vm) > 1024 * 1024) // output xxM
              fprintf (stdout, "最大内存占用:\t %.5f MB\n",
                       (double)gc_getmemMax (vm) / 1024 / 1024);
            else
              fprintf (stdout, "最大内存占用:\t %.5f KB\n",
                       (double)gc_getmemMax (vm) / 1024);
            fprintf (stdout, "--------------------------------\n");
            break;
          }
        default:
          end = clock ();
          cpu_time_used = ((double)(end - start)) / (double)CLOCKS_PER_SEC;
          // output log
          fprintf (stdout, "--------------------------------\n");
          fprintf (stdout, "程序出错\n");
          fprintf (stdout,
                   "统计\n\t运算:\t%lld\n\t调用:\t%lld\n\t循环:\t%lld\n",
                   vm->aluhook - 1 > 0 ? vm->aluhook - 1 : 0,
                   vm->callhook - 2 > 0 ? vm->callhook - 2 : 0, vm->loophook);
          fprintf (stdout, "CPU时间:\t%.5fs\n", cpu_time_used);
          if (gc_getmemMax (vm) > 1024 * 1024 * 1024)
            fprintf (stdout, "最大内存占用:\t %.5f GB\n",
                     (double)gc_getmemMax (vm) / 1024 / 1024 / 1024);
          else if (gc_getmemMax (vm) > 1024 * 1024) // output xxM
            fprintf (stdout, "最大内存占用:\t %.5f MB\n",
                     (double)gc_getmemMax (vm) / 1024 / 1024);
          else
            fprintf (stdout, "最大内存占用:\t %.5f KB\n",
                     (double)gc_getmemMax (vm) / 1024);
          fprintf (stdout, "--------------------------------\n");
          break;
          break;
        }
    }
endrun:
  vm_close (vm);
  Ec2WasmVM = NULL;
  emscripten_run_script ("WasmMutex.runLock = 0;");
}
void
wasmIObreakCode ()
{
  emscripten_run_script ("WasmMutex.runLock = 0;"); // 释放运行锁
}