#pragma once

#include <array>

template <typename T, size_t S, typename P>
constexpr auto gen_array(P&& pred) noexcept
{
    std::array<T, S> res{};
    for (int i = 0; i < S; i++)
        res[i] = pred(i);
    return res;
}

template <typename T, size_t S, typename P>
constexpr void fill_array(std::array<T, S>& arr, P&& pred) noexcept
{
    for (int i = 0; i < S; i++)
        arr[i] = pred(i);
}