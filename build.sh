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
)

clang++ -O3 -std=c++2a -fcoroutines-ts ${include_dirs[@]} ${cpp_files[@]}
echo "Done."
echo

./a.out