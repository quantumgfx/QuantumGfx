#pragma once

#include <atomic>
#include <cstdint>

namespace Qgfx
{
    struct BasicAtomics
    {
        using Long = long;
        using AtomicLong = std::atomic<Long>;
        using Int64 = int64_t;
        using AtomicInt64 = std::atomic<Int64>;

        // The function returns the resulting INCREMENTED value.
        template <typename Type>
        static inline Type AtomicIncrement(std::atomic<Type> &Val)
        {
            return ++Val;
        }

        // The function returns the resulting DECREMENTED value.
        template <typename Type>
        static inline Type AtomicDecrement(std::atomic<Type> &Val)
        {
            return --Val;
        }

        // The function compares the Destination value with the Comparand value. If the Destination value is equal
        // to the Comparand value, the Exchange value is stored in the address specified by Destination.
        // Otherwise, no operation is performed.
        // The function returns the initial value of the Destination parameter
        template <typename Type>
        static inline Type AtomicCompareExchange(std::atomic<Type> &Destination, Type Exchange, Type Comparand)
        {
            Destination.compare_exchange_strong(Comparand, Exchange);
            return Comparand;
        }

        template <typename Type>
        static inline Type AtomicAdd(std::atomic<Type> &Destination, Type Val)
        {
            return std::atomic_fetch_add(&Destination, Val);
        }
    };
}
