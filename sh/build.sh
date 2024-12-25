#!/bin/sh
emcc\
    ec2_wasm/losu/*.c\
    ec2_wasm/ec2/*.c\
    ec2_wasm/wasm/*.c\
    -I ec2_wasm/include/\
    -s WASM=1\
    -s ALLOW_MEMORY_GROWTH=1\
    -s EXPORTED_FUNCTIONS='["_wasmIOrunCode"]'\
    -s ASYNCIFY=1\
    -o js/ec2_wasm.js\
    -O3\
    -s STACK_SIZE=16777216\


# emrun index.html
