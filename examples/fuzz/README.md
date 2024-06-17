# What's this

A [Libfuzzer] target, which instantiates a wasm module with toywasm.

[Libfuzzer]: https://llvm.org/docs/LibFuzzer.html

# How to

## Build

```shell
./build.sh
```

## Run

```shell
./run.sh
```

## Run with seeding with wasm-tools

```shell
cargo install wasm-tools
./seed.sh
./run.sh seed
```

# Corpus

Maybe it's a good idea to seed your corpus from ones for
other fuzzer targets which take raw wasm binaries.

For examples,

* https://github.com/yamt/toywasm-fuzzer-corpus

* https://github.com/bytecodealliance/wasmtime-libfuzzer-corpus

* https://github.com/wasmx/wasm-fuzzing-corpus
