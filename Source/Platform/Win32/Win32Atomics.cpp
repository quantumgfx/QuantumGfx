#include "Qgfx/Platform/Win32/Win32Atomics.hpp"

#include <Windows.h>

namespace Qgfx
{
    // The function returns the resulting INCREMENTED value.
    Numerics::Long WindowsAtomics::Increment(AtomicLong& Val)
    {
        return InterlockedIncrement(&Val);
    }

    Numerics::Int64 WindowsAtomics::Increment(AtomicInt64& Val)
    {
        return InterlockedIncrement64(&Val);
    }

    // The function returns the resulting DECREMENTED value.
    Numerics::Long WindowsAtomics::Decrement(AtomicLong& Val)
    {
        return InterlockedDecrement(&Val);
    }

    Numerics::Int64 WindowsAtomics::Decrement(AtomicInt64& Val)
    {
        return InterlockedDecrement64(&Val);
    }

    Numerics::Long WindowsAtomics::Load(WindowsAtomics::AtomicLong& Val)
    {
        return Val;
    }

    Numerics::Int64 WindowsAtomics::Load(WindowsAtomics::AtomicInt64& Val)
    {
        return Val;
    }

    Numerics::Long WindowsAtomics::CompareExchange(AtomicLong& Destination, Long Exchange, Long Comparand)
    {
        return InterlockedCompareExchange(&Destination, Exchange, Comparand);
    }

    Numerics::Long WindowsAtomics::Add(AtomicLong& Destination, Long Val)
    {
        return InterlockedAdd(&Destination, Val);
    }

    Numerics::Int64 WindowsAtomics::Add(AtomicInt64& Destination, Int64 Val)
    {
        return InterlockedAdd64(&Destination, Val);
    }
}