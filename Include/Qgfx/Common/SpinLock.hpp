#pragma once

#include "Error.hpp"

#include "../Platform/Atomics.hpp"

namespace Qgfx
{
    class SpinLockFlag
    {
    public:
        enum State
        {
            Unlocked = 0,
            Locked = 1
        };

        SpinLockFlag(Atomics::Long InitFlag = static_cast<Atomics::Long>(SpinLockFlag::State::Unlocked)) noexcept
        {
            //m_Flag.store(InitFlag);
            m_Flag = InitFlag;
        }

        operator Atomics::Long() const { return m_Flag; }

    private:
        friend class SpinLock;
        Atomics::AtomicLong m_Flag;
    };

    // Spinlock implementation. This kind of lock should be used in scenarios
    // where simultaneous access is uncommon but possible.
    class SpinLock
    {
    public:
        SpinLock() noexcept {}

        SpinLock(SpinLockFlag& LockFlag) noexcept
        {
            Lock(LockFlag);
        }

        SpinLock(SpinLock&& Lock) noexcept :
            m_pLockFlag{ std::move(Lock.m_pLockFlag) }
        {
            Lock.m_pLockFlag = nullptr;
        }

        SpinLock(const SpinLock& Lock) = delete;

        SpinLock& operator=(const SpinLock& Lock) = delete;

        const SpinLock& operator=(SpinLock&& Lock) noexcept
        {
            m_pLockFlag = std::move(Lock.m_pLockFlag);
            Lock.m_pLockFlag = nullptr;
            return *this;
        }

        ~SpinLock()
        {
            Unlock();
        }

        static bool UnsafeTryLock(SpinLockFlag& LockFlag) noexcept
        {
            return Atomics::AtomicCompareExchange(LockFlag.m_Flag, static_cast<Atomics::Long>(SpinLockFlag::State::Locked), 
                static_cast<Atomics::Long>(SpinLockFlag::State::Unlocked)) == SpinLockFlag::State::Unlocked;
        }

        bool TryLock(SpinLockFlag& LockFlag) noexcept
        {
            if (UnsafeTryLock(LockFlag))
            {
                m_pLockFlag = &LockFlag;
                return true;
            }
            else
                return false;
        }

        static constexpr const int DefaultSpinCountToYield = 256;

        static void UnsafeLock(SpinLockFlag& LockFlag, int SpinCountToYield = DefaultSpinCountToYield) noexcept
        {
            int SpinCount = 0;
            while (!UnsafeTryLock(LockFlag))
            {
                ++SpinCount;
                if (SpinCount == SpinCountToYield)
                {
                    SpinCount = 0;
                    YieldThread();
                }
            }
        }

        void Lock(SpinLockFlag& LockFlag, int SpinCountToYield = DefaultSpinCountToYield) noexcept
        {
            QGFX_VERIFY(m_pLockFlag == nullptr, "Object already locked");
            // Wait for the flag to become unlocked and lock it
            int SpinCount = 0;
            while (!TryLock(LockFlag))
            {
                ++SpinCount;
                if (SpinCount == SpinCountToYield)
                {
                    SpinCount = 0;
                    YieldThread();
                }
            }
        }

        static void UnsafeUnlock(SpinLockFlag& LockFlag) noexcept
        {
            LockFlag.m_Flag = SpinLockFlag::State::Unlocked;
        }

        void Unlock() noexcept
        {
            if (m_pLockFlag)
                UnsafeUnlock(*m_pLockFlag);
            m_pLockFlag = nullptr;
        }

    private:
        static void YieldThread() noexcept;

        SpinLockFlag* m_pLockFlag = nullptr;
    };

}