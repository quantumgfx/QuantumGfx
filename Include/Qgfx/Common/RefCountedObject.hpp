#pragma once

#include "SpinLock.hpp"
#include "MemoryAllocator.hpp"

#include "../Platform/Atomics.hpp"

#include <utility>

namespace Qgfx
{
	/**
	 * @brief Forward declare RefCountedObject.
	*/
	class RefCountedObject;

	/**
	 * @brief Represents a RefCountedAllocation block. In order to minimize fragmentation and memory allocation, both the RefCounter
	 * and the managed object are allocated together. This represents that allocation
	*/
	class IRefCountedAllocation
	{
	public:

		virtual RefCountedObject* GetManagedObject() = 0;
		virtual RefCountedObject* GetOwnerObject() = 0;
		virtual void DestructObject() = 0;
		virtual void FreeSelf() = 0;
	};

	/**
	 * @brief Manages the references counts of an object, and allows for weak references (contrary to standard intrusive ref implementations).
	*/
	class RefCounter
	{
	public:

		inline Atomics::Long AddStrongRef()
		{
			Atomics::AtomicIncrement(m_lNumStrongReferences);
		}

		template <class TPreObjectDestroy>
		inline Atomics::Long ReleaseStrongRef(TPreObjectDestroy PreObjectDestroy)
		{
			QGFX_VERIFY(m_ObjectState == ObjectState::Alive, "Attempting to decrement strong reference counter for an object that is not alive");

			// Decrement strong reference counter without acquiring the lock.
			auto RefCount = Atomics::AtomicDecrement(m_lNumStrongReferences);
			QGFX_VERIFY(RefCount >= 0, "Inconsistent call to ReleaseStrongRef()");
			if (RefCount == 0)
			{
				PreObjectDestroy();
				TryDestroyObject();
			}

			return RefCount;
		}

		inline Atomics::Long ReleaseStrongRef()
		{
			return ReleaseStrongRef([]() {});
		}

		inline Atomics::Long AddWeakRef()
		{
			return Atomics::AtomicIncrement(m_lNumWeakReferences);
		}

		inline Atomics::Long ReleaseWeakRef()
		{
			// The method must be serialized!
			SpinLock Lock(m_LockFlag);
			// It is essentially important to check the number of weak references
			// while holding the lock. Otherwise reference counters object
			// may be destroyed twice if ReleaseStrongRef() is executed by other
			// thread.
			auto NumWeakReferences = Atomics::AtomicDecrement(m_lNumWeakReferences);
			QGFX_VERIFY(NumWeakReferences >= 0, "Inconsistent call to ReleaseWeakRef()");

			
			if (NumWeakReferences == 0 && m_ObjectState == ObjectState::Destroyed)
			{
				QGFX_VERIFY_EXPR(m_lNumStrongReferences == 0);
				// m_ObjectState is set to ObjectState::Destroyed under the lock. If the state is not Destroyed,
				// ReleaseStrongRef() will take care of it.
				// Access to Object wrapper and decrementing m_lNumWeakReferences is atomic. Since we acquired the lock,
				// no other thread can access either of them.
				// Access to m_lNumStrongReferences is NOT PROTECTED by lock.

				// There are no more references to the ref counters object and the object itself
				// is already destroyed.
				// We can safely unlock it and destroy.
				// If we do not unlock it, this->m_LockFlag will expire,
				// which will cause Lock.~LockHelper() to crash.
				Lock.Unlock();
				m_pAllocation->FreeSelf();
			}
			return NumWeakReferences;
		}

		inline void GetObject(RefCountedObject** ppObject)
		{
			if (m_ObjectState != ObjectState::Alive)
				return; // Early exit

			// It is essential to INCREMENT REF COUNTER while HOLDING THE LOCK to make sure that
			// StrongRefCnt > 1 guarantees that the object is alive.

			// If other thread started deleting the object in ReleaseStrongRef(), then m_lNumStrongReferences==0
			// We must make sure only one thread is allowed to increment the counter to guarantee that if StrongRefCnt > 1,
			// there is at least one real strong reference left. Otherwise the following scenario may occur:
			//
			//                                      m_lNumStrongReferences == 1
			//
			//    Thread 1 - ReleaseStrongRef()    |     Thread 2 - GetObject()        |     Thread 3 - GetObject()
			//                                     |                                   |
			//  - Decrement m_lNumStrongReferences | -Increment m_lNumStrongReferences | -Increment m_lNumStrongReferences
			//  - Read RefCount == 0               | -Read StrongRefCnt==1             | -Read StrongRefCnt==2
			//    Destroy the object               |                                   | -Return reference to the soon
			//                                     |                                   |  to expire object
			//

			SpinLock Lock(m_LockFlag);

			auto StrongRefCnt = Atomics::AtomicIncrement(m_lNumStrongReferences);

			// Checking if m_ObjectState == ObjectState::Alive only is not reliable:
			//
			//           This thread                    |          Another thread
			//                                          |
			//   1. Acquire the lock                    |
			//                                          |    1. Decrement m_lNumStrongReferences
			//   2. Increment m_lNumStrongReferences    |    2. Test RefCount==0
			//   3. Read StrongRefCnt == 1              |    3. Start destroying the object
			//      m_ObjectState == ObjectState::Alive |
			//   4. DO NOT return the reference to      |    4. Wait for the lock, m_ObjectState == ObjectState::Alive
			//      the object                          |
			//   5. Decrement m_lNumStrongReferences    |
			//                                          |    5. Destroy the object

			if (m_ObjectState == ObjectState::Alive && StrongRefCnt > 1)
			{
				*ppObject = m_pAllocation->GetManagedObject();
			}

			if (*ppObject == nullptr)
			{
				Atomics::AtomicDecrement(m_lNumStrongReferences);
			}
		}

		inline Atomics::Long GetNumStrongRefs() const
		{
			return m_lNumStrongReferences;
		}

		inline Atomics::Long GetNumWeakRefs() const
		{
			return m_lNumWeakReferences;
		}

	private:

		template<typename ObjectType, typename AllocatorType>
		friend class RefCountedAllocationImpl;

		RefCounter(IRefCountedAllocation* pOwningAllocation) noexcept
			: m_pAllocation(pOwningAllocation)
		{
			m_lNumStrongReferences = 0;
			m_lNumWeakReferences = 0;
		}

		~RefCounter() = default;

		void TryDestroyObject()
		{
#ifdef QGFX_DEBUG
			{
				Atomics::Long NumStrongRefs = m_lNumStrongReferences;
				QGFX_VERIFY(NumStrongRefs == 0 || NumStrongRefs == 1, "Num strong references (", NumStrongRefs, ") is expected to be 0 or 1");
			}
#endif

			SpinLock Lock(m_LockFlag);

			// GetObject() first acquires the lock, and only then increments and
			// decrements the ref counter. If it reads 1 after incremeting the counter,
			// it does not return the reference to the object and decrements the counter.
			// If we acquired the lock, GetObject() will not start until we are done
			QGFX_VERIFY_EXPR(m_lNumStrongReferences == 0 && m_ObjectState == ObjectState::Alive);

			// Extra caution
			if (m_lNumStrongReferences == 0 && m_ObjectState == ObjectState::Alive)
			{
				// We cannot destroy the object while reference counters are locked as this will
				// cause a deadlock in cases like this:
				//
				//    A ==sp==> B ---wp---> A
				//
				//    RefCounters_A.Lock();
				//    delete A{
				//      A.~dtor(){
				//          B.~dtor(){
				//              wpA.ReleaseWeakRef(){
				//                  RefCounters_A.Lock(); // Deadlock
				//

				// So we destroy the object after unlocking the reference counters

				// In a multithreaded environment, reference counters object may
				// be destroyed at any time while m_pObject->~dtor() is running.

				// Note that this is the only place where m_ObjectState is
				// modified after the ref counters object has been created
				m_ObjectState = ObjectState::Destroyed;
				// The object is now detached from the reference counters and it is if
				// it was destroyed since no one can obtain access to it.


				// It is essentially important to check the number of weak references
				// while the object is locked. Otherwise reference counters object
				// may be destroyed twice if ReleaseWeakRef() is executed by other thread:
				//
				//             This thread             |    Another thread - ReleaseWeakRef()
				//                                     |
				// 1. Decrement m_lNumStrongReferences,|
				//    m_lNumStrongReferences==0,       |
				//    acquire the lock, destroy        |
				//    the obj, release the lock        |
				//    m_lNumWeakReferences == 1        |
				//                                     |   1. Aacquire the lock,
				//                                     |      decrement m_lNumWeakReferences,
				//                                     |      m_lNumWeakReferences == 0,
				//                                     |      m_ObjectState == ObjectState::Destroyed
				//                                     |
				// 2. Read m_lNumWeakReferences == 0   |
				// 3. Destroy the ref counters obj     |   2. Destroy the ref counters obj
				//
				bool bDestroyThis = m_lNumWeakReferences == 0;
				// ReleaseWeakRef() decrements m_lNumWeakReferences, and checks it for
				// zero only after acquiring the lock. So if m_lNumWeakReferences==0, no
				// weak reference-related code may be running


				// We must explicitly unlock the object now to avoid deadlocks. Also,
				// if this is deleted, this->m_LockFlag will expire, which will cause
				// Lock.~LockHelper() to crash
				Lock.Unlock();

				// Call the attached object's destructor
				m_pAllocation->DestructObject();

				// Note that <this> may be destroyed here already,
				// see comments in ~ControlledObjectType()
				if (bDestroyThis) // Free the object, its memory, and this reference counter.
					m_pAllocation->FreeSelf();
			}
		}

		IRefCountedAllocation* m_pAllocation;

		Atomics::AtomicLong m_lNumStrongReferences;
		Atomics::AtomicLong m_lNumWeakReferences;
		SpinLockFlag m_LockFlag;
		enum class ObjectState : uint32_t
		{
			NotInitialized,
			Alive,
			Destroyed
		};
		volatile ObjectState m_ObjectState = ObjectState::NotInitialized;
	};

	/**
	 * @brief Base class from which all RefCounted classes derive.
	*/
	class RefCountedObject
	{
	public:

		RefCountedObject(RefCounter* pRefCounter)
			: m_pRefCounter(pRefCounter)
		{
		}

		/**
		 * @brief Increases strong references of RefCountedObject by 1.
		 * @return The number of strong references after the variable has been incremented.
		*/
		inline Atomics::Long AddRef()
		{
			m_pRefCounter->AddStrongRef();
		}

		virtual Atomics::Long Release()
		{
			m_pRefCounter->ReleaseStrongRef();
		}

		inline RefCounter* GetRefCounter()
		{
			return m_pRefCounter;
		}

	private:

		RefCounter* m_pRefCounter;
	};

	template<typename ObjectType, typename AllocatorType>
	class RefCountedAllocation final : public IRefCountedAllocation
	{
	public:

		virtual RefCountedObject* GetManagedObject() override final
		{
			return static_cast<RefCountedObject*>(ObjectPtr());
		}

		virtual RefCountedObject* GetOwnerObject() override final
		{
			return m_pOwner;
		}

		virtual void DestructObject() override final
		{
			ObjectPtr()->~ObjectType();
		}

		virtual void FreeSelf() override final
		{
			// Call RefCounter's destructor
			RefCounterPtr()->~RefCounter();

			if (m_pAllocator)
				m_pAllocator->Free(this);
			else
				delete[] reinterpret_cast<uint8_t*>(this);
		}

	private:

		RefCountedAllocation(AllocatorType* pAllocator, RefCountedObject* pOwner)
			: m_pAllocator{ pAllocator }, m_pOwner{pOwner}
		{
		}

		inline ObjectType* ObjectPtr() const { return reinterpret_cast<ObjectType*>(m_ObjectStorage); }
		inline RefCounter* RefCounterPtr() const { return reinterpret_cast<RefCounter*>(m_RefCounterStorage); }

		alignas(ObjectType) uint8_t m_ObjectStorage[sizeof(ObjectType)];
		alignas(RefCounter) uint8_t m_RefCounterStorage[sizeof(RefCounter)];
		AllocatorType* m_pAllocator;
		RefCountedObject* m_pOwner;

	private:

		template<typename ObjectType, typename AllocatorType>
		friend class MakeRefCountedObj;

		template<typename ... CtorArgTypes>
		static RefCountedAllocation* Create(AllocatorType* pAllocator, RefCountedObject* pOwner, CtorArgTypes&& ... CtorArgs)
		{
			RefCountedAllocation* Alloc;
			if (pAllocator)
				Alloc = reinterpret_cast<RefCountedAllocation*>(pAllocator->Allocate(sizeof(RefCountedAllocation)));
			else
				Alloc = reinterpret_cast<RefCountedAllocation*>(new uint8_t[sizeof(RefCountedAllocation)]);

			new(Alloc) RefCountedAllocation(pAllocator, pOwner);
			new (Alloc->RefCounterPtr()) RefCounter(Alloc);
			new (Alloc->ObjectPtr()) ObjectType(RefCounterPtr(), std::forward<CtorArgTypes>(CtorArgs)...);

			Alloc->ObjectPtr()->AddRef();
		}
	};

	/**
	 * @brief Interface for creating new ref counted objects.
	 * @tparam ObjectType
	 * @tparam AllocatorType 
	*/
	template<typename ObjectType, typename AllocatorType = IMemoryAllocator>
	class MakeRefCountedObj
	{
	public:

		using RefCountedAllocationType = RefCountedAllocation<ObjectType, AllocatorType>;

		explicit MakeRefCountedObj()
			: m_pAllocator(nullptr)
		{
		}

		explicit MakeRefCountedObj(AllocatorType& Allocator, RefCountedObject* pOwner)
			: m_pAllocator(&Allocator), m_pOwner(pOwner)
		{
		}

		template<typename ... CtorArgTypes>
		ObjectType* operator() (CtorArgTypes&& ... CtorArgs)
		{
			RefCountedAllocationType* Alloc = RefCountedAllocationType::Create(m_pAllocator, m_pOwner, std::forward<CtorArgTypes>(CtorArgs));

			return Alloc->ObjectPtr();
		}

	private:

		AllocatorType* m_pAllocator;
		RefCountedObject* m_pOwner;
	};
}