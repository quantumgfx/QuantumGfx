#pragma once

#include <functional>
#include <memory>
#include <cstring>

#include "Error.hpp"

namespace Qgfx
{

    // http://www.boost.org/doc/libs/1_35_0/doc/html/hash/combine.html
    template <typename T>
    void HashCombine(std::size_t& Seed, const T& Val)
    {
        Seed ^= std::hash<T>{}(Val)+0x9e3779b9 + (Seed << 6) + (Seed >> 2);
    }

    template <typename FirstArgType, typename... RestArgsType>
    void HashCombine(std::size_t& Seed, const FirstArgType& FirstArg, const RestArgsType&... RestArgs)
    {
        HashCombine(Seed, FirstArg);

        if constexpr (sizeof(RestArgs)... > 0)
        {
            HashCombine(Seed, RestArgs...); // recursive call using pack expansion syntax
        }
    }

    template <typename... ArgsType>
    std::size_t ComputeHash(const ArgsType&... Args)
    {
        std::size_t Seed = 0;
        HashCombine(Seed, Args...);
        return Seed;
    }

}