#pragma once

#include <array>
#include <tuple>
#include <type_traits>
#include "container_utils.hpp"
#include "meta.hpp"
//#include <byteswap.h>


__attribute__((aligned(64))) constexpr auto _reversed = gen_array<unsigned char, 0x100>([](const int i) constexpr
                                                                                        {
                                                                                            unsigned char res = i >> 4 | i << 4;
                                                                                            res = (res & 0xCC) >> 2 | (res & 0x33) << 2;
                                                                                            return (res & 0xAA) >> 1 | (res & 0x55) << 1;
                                                                                        });

template <typename T>
inline T swap_bytes(T value) noexcept
{
    static_assert(std::is_integral_v<T>);

    if constexpr(std::is_integral_v<T> && sizeof(T) == 8)
    {
        #if defined(__arm__) || defined(__aarch64__)
            asm("rev %0" : "=r"(value) : "r"(value));
        #elif defined(__x86__) || defined(__x86_64__)
            asm("bswap %0" : "=r"(value) : "r"(value));
        #else
            value = (value >> 32) | (value << 32);
            value = (value & 0xFFFFFFFF00000000) >> 16 | (value & 0x00000000FFFFFFFF) << 16;
            value = (value & 0xFF00FF00FF00FF00) >> 8 | (value & 0x00FF00FF00FF00FF) << 8;
        #endif
    }
    else if constexpr(std::is_integral_v<T> && sizeof(T) == 4)
    {
        #if defined(__arm__) || defined(__aarch64__)
            asm("rev %0" : "=r"(value) : "r"(value));
        #elif defined(__x86__) || defined(__x86_64__)
            asm("bswap %0" : "=r"(value) : "r"(value));
        #else
            value = (value >> 16) | (value << 16);
            value = (value & 0xFF00FF00) >> 8 | (value & 0x00FF00FF) << 8;
        #endif
    }
    else if constexpr(std::is_integral_v<T> && sizeof(T) == 2)
    {
        #if defined(__arm__) || defined(__aarch64__)
            asm("rev16 %0" : "=r"(value) : "r"(value));
        #elif defined(__x86__) || defined(__x86_64__)
            unsigned int res = value << 16;
            asm("bswap %0" : "=r"(res) : "r"(res));
            return res;
        #else
            value = (value >> 8) | (value << 8);
        #endif
    }

    return value;
}

template <typename T>
inline T swap_bits(T value) noexcept
{
    static_assert(std::is_integral_v<T>);

    if constexpr(std::is_integral_v<T> && sizeof(T) == 8)
    {
        #if defined(__arm__) || defined(__aarch64__)
            asm("rbit %0" : "=r"(value) : "r"(value));
        #else
            return (T)_reversed[value & 0xFF] << 56 | (T)_reversed[(value >> 8) & 0xFF] << 48 | (T)_reversed[(value >> 16) & 0xFF] << 40 | (T)_reversed[(value >> 24) & 0xFF] << 32 |
                   (T)_reversed[(value >> 32) & 0xFF] << 24 | (T)_reversed[(value >> 40) & 0xFF] << 16 | (T)_reversed[(value >> 48) & 0xFF] << 8 | (T)_reversed[(value >> 56)];
        #endif
    }
    else if constexpr(std::is_integral_v<T> && sizeof(T) == 4)
    {
        #if defined(__arm__) || defined(__aarch64__)
            asm("rbit %0" : "=r"(value) : "r"(value));
        #else
            return _reversed[value & 0xFF] << 24 | _reversed[(value >> 8) & 0xFF] << 16 | _reversed[(value >> 16) & 0xFF] << 8 | _reversed[(value >> 24)];
        #endif
    }
    else if constexpr(std::is_integral_v<T> && sizeof(T) == 2)
    {
        #if defined(__arm__) || defined(__aarch64__)
            unsigned int res = value << 16;
            asm("rbit %0, %1" : "=r"(value) : "r"(res));
        #else
            return _reversed[value & 0xFF] << 8 | _reversed[(value >> 8) & 0xFF];
        #endif
    }
    else if constexpr(std::is_integral_v<T> && sizeof(T) == 1)
    {
        #if defined(__arm__) || defined(__aarch64__)
            unsigned int res = value << 24;
            asm("rbit %0, %1" : "=r"(value) : "r"(res));
        #else
            return _reversed[value];
        #endif
    }

    return value;
};