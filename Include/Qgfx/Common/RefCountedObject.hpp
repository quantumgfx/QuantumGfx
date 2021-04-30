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
	 * @brief Manages the references counts of an object, and allows for weak references (contrary to standard intrusive ref implementations).
	*/
	class IRefCounter
	{
	public:

		virtual RefCountedObject* GetManagedObject() const = 0;
		virtual RefCountedObject* GetOwningObject() const = 0;

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
				FreeSelf();
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
				*ppObject = GetManagedObject();
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

		virtual void DestructObject() = 0;
		virtual void FreeSelf() = 0;

		template<typename ObjectType, typename AllocatorType>
		friend class RefCountedAllocationImpl;

		IRefCounter() noexcept
		{
			m_lNumStrongReferences = 0;
			m_lNumWeakReferences = 0;
		}

		~IRefCounter() = default;

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
				DestructObject();

				// Note that <this> may be destroyed here already,
				// see comments in ~ControlledObjectType()
				if (bDestroyThis) // Free the object, its memory, and this reference counter.
					FreeSelf();
			}
		}

		// IRefCountedAllocation* m_pAllocation;

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

		RefCountedObject(IRefCounter* pRefCounter)
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

		inline IRefCounter* GetRefCounter()
		{
			return m_pRefCounter;
		}

	private:

		IRefCounter* m_pRefCounter;
	};

	template<typename ObjectType, typename AllocatorType>
	class RefCounterImpl final : public IRefCounter
	{
	public:

		virtual RefCountedObject* GetManagedObject() const override final
		{
			return ObjectPtr();
		}

		virtual RefCountedObject* GetOwnerObject() const override final
		{
			return m_pOwner;
		}

	private:

		virtual void DestructObject() override final
		{
			ObjectPtr()->ObjectType::~ObjectType();
		}

		virtual void FreeSelf() override final
		{
			// Call RefCounter's destructor
			this->IRefCounter::~IRefCounter();

			if (m_pAllocator)
				m_pAllocator->Deallocate(this, sizeof(RefCounterImpl));
			else
				delete[] reinterpret_cast<uint8_t*>(this);
		}

		ObjectType* ObjectPtr() const
		{
			return reinterpret_cast<ObjectType*>(m_ObjectStorage);
		}

		alignas(ObjectType) uint8_t m_ObjectStorage[sizeof(ObjectType)];
		AllocatorType* m_pAllocator;
		RefCountedObject* m_pOwner;

	private:

		template<typename ObjectType, typename AllocatorType>
		friend class MakeRefCountedObj;

		RefCounterImpl(AllocatorType* pAllocator, RefCountedObject* pOwner)
			: m_pAllocator{ pAllocator }, m_pOwner{ pOwner }
		{
		}

		static RefCounterImpl* Create(AllocatorType* pAllocator, RefCountedObject* pOwner)
		{
			RefCounterImpl* pRefCounter;
			if (pAllocator)
				pRefCounter = reinterpret_cast<RefCounterImpl*>(pAllocator->Allocate(sizeof(RefCounterImpl)));
			else
				pRefCounter = reinterpret_cast<RefCounterImpl*>(new uint8_t[sizeof(RefCounterImpl)]);

			new(pRefCounter) RefCounterImpl(pAllocator, pOwner);

			return pRefCounter;
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
			RefCounterImpl<ObjectType, AllocatorType>* pRefCounter = RefCounterImpl<ObjectType, AllocatorType>::Create(m_pAllocator, m_pOwner);

			new (pRefCounter->ObjectPtr()) ObjectType(pRefCounter, std::forward<CtorArgTypes>(CtorArgs)...);

			pRefCounter->AddStrongRef();

			return pRefCounter->ObjectPtr();
		}

	private:

		AllocatorType* m_pAllocator;
		RefCountedObject* m_pOwner;
	};
}