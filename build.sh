#!/bin/zsh

echo "Building project..."

cpp_files=(
    "src/boot/boot.cpp"
    "src/cartridge/cartridge.cpp"
    "src/cpu/cpu.cpp"
    "src/cpu/interrupts.cpp"
    "src/memory/mmu.cpp"
    "src/utils/loader.cpp"
    "src/main.cpp"
)

include_dirs=(
    "-Isrc"
    "-I/usr/local/Cellar/sdl2/2.0.12_1/include"
)

lib_dirs=(
    "-L/usr/local/Cellar/sdl2/2.0.12_1/lib"
)

libs=(
    "-lSDL2"
)

clang++ -O3 -std=c++2a -fcoroutines-ts ${include_dirs[@]} ${lib_dirs[@]} ${libs[@]} ${cpp_files[@]}
echo "Done."
echo

./a.out