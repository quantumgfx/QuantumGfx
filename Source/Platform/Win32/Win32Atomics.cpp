#include "Qgfx/Platform/Win32/Win32Atomics.hpp"

#include <Windows.h>

namespace Qgfx
{
    // The function returns the resulting INCREMENTED value.
    WindowsAtomics::Long WindowsAtomics::AtomicIncrement(AtomicLong& Val)
    {
        return InterlockedIncrement(&Val);
    }

    WindowsAtomics::Int64 WindowsAtomics::AtomicIncrement(AtomicInt64& Val)
    {
        return InterlockedIncrement64(&Val);
    }

    // The function returns the resulting DECREMENTED value.
    WindowsAtomics::Long WindowsAtomics::AtomicDecrement(AtomicLong& Val)
    {
        return InterlockedDecrement(&Val);
    }

    WindowsAtomics::Int64 WindowsAtomics::AtomicDecrement(AtomicInt64& Val)
    {
        return InterlockedDecrement64(&Val);
    }

    WindowsAtomics::Long WindowsAtomics::AtomicCompareExchange(AtomicLong& Destination, Long Exchange, Long Comparand)
    {
        return InterlockedCompareExchange(&Destination, Exchange, Comparand);
    }

    WindowsAtomics::Long WindowsAtomics::AtomicAdd(AtomicLong& Destination, Long Val)
    {
        return InterlockedAdd(&Destination, Val);
    }

    WindowsAtomics::Int64 WindowsAtomics::AtomicAdd(AtomicInt64& Destination, Int64 Val)
    {
        return InterlockedAdd64(&Destination, Val);
    }
}