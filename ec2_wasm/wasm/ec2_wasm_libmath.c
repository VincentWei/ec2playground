#include "ec2_wasm.h"
#include "losu.h"
#include "losu_malloc.h"
#include "losu_object.h"
#include <emscripten.h>

// #include <emscripten.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const double PI = 3.141592653589793238462643383279502;
const double E = 2.718281828459045235360287471352662;

static inline double
__arg_getnum (LosuVm *vm, int idx)
{
  return obj_tonum (vm, arg_get (vm, idx));
}

static inline void
__arg_returnnum (LosuVm *vm, double n)
{
  arg_return (vm, obj_newnum (vm, n));
}

static int32_t
simgleNumber (LosuVm *vm, double (*func) (double))
{
  double n = __arg_getnum (vm, 2);
  __arg_returnnum (vm, func (n));
  return 1;
}

int32_t
wasm_libmath_abs (LosuVm *vm)
{
  return simgleNumber (vm, fabs);
}

int32_t
wasm_libmath_sqrt (LosuVm *vm)
{
  return simgleNumber (vm, sqrt);
}

int32_t
wasm_libmath_cbrt (LosuVm *vm)
{
  return simgleNumber (vm, cbrt);
}

int32_t
wasm_libmath_pow (LosuVm *vm)
{
  double x, y;

  x = __arg_getnum (vm, 2);
  y = __arg_getnum (vm, 3);
  __arg_returnnum (vm, pow (x, y));

  return 1;
}

int32_t
wasm_libmath_log (LosuVm *vm)
{
  return simgleNumber (vm, log);
}

int32_t
wasm_libmath_log10 (LosuVm *vm)
{
  return simgleNumber (vm, log10);
}

int32_t
wasm_libmath_log2 (LosuVm *vm)
{
  return simgleNumber (vm, log2);
}

EM_JS (double, jsfloor, (double x), { return Math.floor (x); });
EM_JS (double, jsceil, (double x), { return Math.ceil (x); });
int32_t
wasm_libmath_floor (LosuVm *vm)
{
  return simgleNumber (vm, jsfloor);
}

int32_t
wasm_libmath_ceil (LosuVm *vm)
{
  return simgleNumber (vm, jsceil);
}

#define make(s) wasm_libmath_##s

static struct
{
  const char *name;
  LosuApi func;
} libfunc[] = {
  { "abs", make (abs) },     { "sqrt", make (sqrt) },
  { "cbrt", make (cbrt) },   { "pow", make (pow) },
  { "log", make (log) },     { "log2", make (log2) },
  { "log10", make (log10) }, { "floor", make (floor) },
  { "ceil", make (ceil) },
};

static void
addFuncToUnit (LosuVm *vm, LosuObj *unit, const char *name,
               int32_t (*func) (LosuVm *))
{
  LosuObj o = obj_newfunction (vm, func);
  obj_setunitbystr (vm, *unit, (char *)name, o);
}

static void
addNumToUnit (LosuVm *vm, LosuObj *unit, const char *name, double n)
{
  LosuObj o = obj_newnum (vm, n);
  obj_setunitbystr (vm, *unit, (char *)name, o);
}

static void
makeMathObj (LosuVm *vm, LosuObj *o)
{
  *o = obj_newunit (vm, 1);

  for (int i = 0; i < (sizeof (libfunc) / sizeof (libfunc[0])); i++)
    {
      const char *name = libfunc[i].name;
      LosuApi func = libfunc[i].func;
      addFuncToUnit (vm, o, name, func);
    }

  addNumToUnit (vm, o, "PI", PI);
  addNumToUnit (vm, o, "E", E);
  addNumToUnit (vm, o, "SQRT2", sqrt (2));
}

void
wasm_libmath__init__ (LosuVm *vm)
{
  LosuObj o;
  makeMathObj (vm, &o);
  vm_setval (vm, "math", o);
}
