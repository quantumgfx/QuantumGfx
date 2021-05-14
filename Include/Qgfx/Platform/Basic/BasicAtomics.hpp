#pragma once

#include <atomic>
#include <cstdint>

#include "../Numerics.hpp"

namespace Qgfx
{
    struct BasicAtomics
    {
        using AtomicLong = std::atomic<Long>;
        using AtomicInt64 = std::atomic<Int64>;

        // The function returns the resulting INCREMENTED value.
        template <typename Type>
        static inline Type Increment(std::atomic<Type> &Val)
        {
            return ++Val;
        }

        // The function returns the resulting DECREMENTED value.
        template <typename Type>
        static inline Type Decrement(std::atomic<Type> &Val)
        {
            return --Val;
        }

        template<typename Type>
        static inline Type Load(std::atomic<Type>& Val)
        {
            return Val.load();
        }

        // The function compares the Destination value with the Comparand value. If the Destination value is equal
        // to the Comparand value, the Exchange value is stored in the address specified by Destination.
        // Otherwise, no operation is performed.
        // The function returns the initial value of the Destination parameter
        template <typename Type>
        static inline Type CompareExchange(std::atomic<Type> &Destination, Type Exchange, Type Comparand)
        {
            Destination.compare_exchange_strong(Comparand, Exchange);
            return Comparand;
        }

        template <typename Type>
        static inline Type Add(std::atomic<Type> &Destination, Type Val)
        {
            return std::atomic_fetch_add(&Destination, Val);
        }
    };
}
