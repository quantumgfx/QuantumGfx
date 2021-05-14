#pragma once

#if !QGFX_PLATFORM_WIN32
#error QGFX_PLATFORM_WIN32 must be defined to include Win32Atomics.hpp
#endif

#include <cstdint>

#include "../Numerics.hpp"

namespace Qgfx
{
    struct WindowsAtomics
    {
        // Use windows-specific atomics. Standard atomic eventually call
        // the same functions, but introduce significant overhead
        using AtomicLong = volatile Numerics::Long;
        using AtomicInt64 = volatile Numerics::Int64;

        // The function returns the resulting INCREMENTED value.
        static Numerics::Long  Increment(WindowsAtomics::AtomicLong& Val);
        static Numerics::Int64 Increment(WindowsAtomics::AtomicInt64& Val);

        // The function returns the resulting DECREMENTED value.
        static Numerics::Long  Decrement(WindowsAtomics::AtomicLong& Val);
        static Numerics::Int64 Decrement(WindowsAtomics::AtomicInt64& Val);

        static Numerics::Long Load(WindowsAtomics::AtomicLong& Val);
        static Numerics::Int64 Load(WindowsAtomics::AtomicInt64& Val);

        // The function compares the Destination value with the Comparand value. If the Destination value is equal
        // to the Comparand value, the Exchange value is stored in the address specified by Destination.
        // Otherwise, no operation is performed.
        // The function returns the initial value of the Destination parameter
        static Numerics::Long CompareExchange(WindowsAtomics::AtomicLong& Destination, Long Exchange, Long Comparand);

        static Numerics::Long  Add(WindowsAtomics::AtomicLong& Destination, Numerics::Long Val);
        static Numerics::Int64 Add(WindowsAtomics::AtomicInt64& Destination, Numerics::Int64 Val);
    };
}