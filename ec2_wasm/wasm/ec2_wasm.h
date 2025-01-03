#include "losu.h"

// wasm_main
extern void wasmIOrunCode ();

// wasm lib list
typedef void (*wasmLibApi) (LosuVm *vm);
extern wasmLibApi wasmLibList[];

// libs

void wasm_libcore__init__ (LosuVm *vm);
void wasm_libmath__init__ (LosuVm *vm);