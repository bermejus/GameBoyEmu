#!/bin/zsh

echo "Building project..."

cpp_files=(
    "src/cpu/cpu.cpp"
    "src/main.cpp"
)

include_dirs=(
    "-Isrc"
)

clang++ -std=c++2a ${include_dirs[@]} ${cpp_files[@]}
echo "Done."
echo

./a.out