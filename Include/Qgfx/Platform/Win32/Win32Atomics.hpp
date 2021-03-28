#pragma once

#if !QGFX_PLATFORM_WIN32
#error QGFX_PLATFORM_WIN32 must be defined to include Win32Atomics.hpp
#endif

#include <Windows.h>

#include <cstdint>

namespace Qgfx
{
    struct WindowsAtomics
    {
        // Use windows-specific atomics. Standard atomic eventually call
        // the same functions, but introduce significant overhead
        using Long = long;
        using AtomicLong = volatile Long;
        using Int64 = signed long long;
        using AtomicInt64 = volatile Int64;

        // The function returns the resulting INCREMENTED value.
        static Long  AtomicIncrement(AtomicLong& Val);
        static Int64 AtomicIncrement(AtomicInt64& Val);

        // The function returns the resulting DECREMENTED value.
        static Long  AtomicDecrement(AtomicLong& Val);
        static Int64 AtomicDecrement(AtomicInt64& Val);

        // The function compares the Destination value with the Comparand value. If the Destination value is equal
        // to the Comparand value, the Exchange value is stored in the address specified by Destination.
        // Otherwise, no operation is performed.
        // The function returns the initial value of the Destination parameter
        static Long AtomicCompareExchange(AtomicLong& Destination, Long Exchange, Long Comparand);

        static Long  AtomicAdd(AtomicLong& Destination, Long Val);
        static Int64 AtomicAdd(AtomicInt64& Destination, Int64 Val);
    };
}