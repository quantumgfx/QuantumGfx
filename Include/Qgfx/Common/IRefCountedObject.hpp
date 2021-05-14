#pragma once

#include "SpinLock.hpp"
#include "MemoryAllocator.hpp"
#include "TypeCompatibleBytes.hpp"
#include "ValidatedCast.hpp"

#include "../Platform/Atomics.hpp"

#include <utility>
#include <memory>
#include <type_traits>

namespace Qgfx
{
    class IWeakRefCounter;

    /**
     * @brief Represents an intrusively refcounted object.
    */
    class IRefCountedObject
    {
    public:

        /**
         * @brief This function increments the ref count of the object by one.
        */
        void AddRef();

        /**
         * @brief This function decrements the ref count of the object by one. 
         * And destroys it once the ref count equals zero.
        */
        void Release();

        /**
         * @brief This retrieves the current value of the ref counter.
         * @return Current value of the ref counter.
        */
        Long GetRefCount();

        IWeakRefCounter* AddWeakRef();

    protected:

        IRefCountedObject();

        virtual ~IRefCountedObject() = default;

        // Override in extended class to provide custom deleter.
        virtual void DeleteThis()
        {
            delete this;
        }

    private:

        AtomicLong m_RefCount;

        // Gates access to m_pWeakRefCounter
        SpinLockFlag m_WeakRefCounterSpinFlag;
        IWeakRefCounter* m_pWeakRefCounter; // nullptr if AddWeakRef() is never called
    };

    class IWeakRefCounter
    {
    public:

        void AddWeakRef()
        {
            Atomics::Increment(m_WeakRefCount);
        }

        void ReleaseWeakRef()
        {
            SpinLock Lock{ m_RefCountedObjectSpinFlag };

            auto WeakRefCount = Atomics::Decrement(m_WeakRefCount);
            QGFX_VERIFY(WeakRefCount >= 0, "Inconsistent call to ReleaseWeakRef()");
            // RefCountedObject holds a weak reference until destroyed, so checking this also checks whether or not the managed object is still alive.
            if (WeakRefCount == 0 && m_pRefCountedObject == nullptr)
            {
                Lock.Unlock();

                DeleteThis();
            }
        }

        void Lock(IRefCountedObject** ppObject)
        {
            SpinLock Lock{ m_RefCountedObjectSpinFlag };

            if (m_pRefCountedObject == nullptr)
            {
                *ppObject = nullptr;
                return;
            }

            // Ensures that Lock is not called when ref count is zero

            m_pRefCountedObject->AddRef();

            if (m_pRefCountedObject->GetRefCount() > 1)
            {
                *ppObject = m_pRefCountedObject;
                return;
            }
            
            m_pRefCountedObject->Release();
        }
        

    private:

        friend IRefCountedObject;

        IWeakRefCounter(IRefCountedObject* pRefCountedObject)
            : m_pRefCountedObject(pRefCountedObject), m_WeakRefCount(1)
        {
        }

        ~IWeakRefCounter()
        {
        }

        void IndicateObjectDestroyed()
        {
            SpinLock Lock{ m_RefCountedObjectSpinFlag };

            m_pRefCountedObject = nullptr;

            if (Atomics::Load(m_WeakRefCount) == 0)
            {
                Lock.Unlock();

                DeleteThis();
            }
        }

        void DeleteThis()
        {
            delete this;
        }

        Atomics::AtomicLong m_WeakRefCount;

        // Gates access to m_pRefCountedObject
        SpinLockFlag m_RefCountedObjectSpinFlag;
        IRefCountedObject* m_pRefCountedObject;

    };

    inline void IRefCountedObject::AddRef()
    {
        Atomics::Increment(m_RefCount);
    }

    inline void IRefCountedObject::Release()
    {
        auto RefCount = Atomics::Decrement(m_RefCount);
        QGFX_VERIFY(RefCount >= 0, "Inconsistent call to Release()");
        if (RefCount == 0)
        {
            // This block can only ever be called once, so no spin lock is necessary

            if (m_pWeakRefCounter)
            {
                m_pWeakRefCounter->IndicateObjectDestroyed();
            }

            DeleteThis();
        }
    }

    inline Long IRefCountedObject::GetRefCount()
    {
        return Atomics::Load(m_RefCount);
    }

    inline IWeakRefCounter* IRefCountedObject::AddWeakRef()
    {
        SpinLock Lock{ m_WeakRefCounterSpinFlag };

        if (m_pWeakRefCounter == nullptr)
        {
            // m_pWeakRefCounter starts with one weak reference
            m_pWeakRefCounter = new IWeakRefCounter(this);
        }
        else
        {
            m_pWeakRefCounter->AddWeakRef();
        }

        Lock.Unlock();

        return m_pWeakRefCounter;
    }

    inline IRefCountedObject::IRefCountedObject()
        : m_RefCount(1), m_pWeakRefCounter(nullptr)
    {
    }

	template<typename T>
	class RefPtr
	{
        // static_assert(std::is_base_of<IRefCountedObject<T>, T>::value, "T must extend Qgfx::IRefCountedObject to be used in Qgfx::RefPtr");

	public:

        RefPtr() noexcept {}

        explicit RefPtr(T* pObj) noexcept
            : m_pObject{ pObj }
        {
            if (m_pObject)
                m_pObject->AddRef();
        }

        RefPtr(const RefPtr& Other) noexcept
            : m_pObject{ Other.m_pObject }
        {
            if (m_pObject)
                m_pObject->AddRef();
        }

        template <typename DerivedType, typename = typename std::enable_if<std::is_base_of<T, DerivedType>::value>::type>
        RefPtr(const RefPtr<DerivedType>& Other) noexcept
            : RefPtr<T>{ Other.m_pObject }
        {
        }

        RefPtr(RefPtr&& Other) noexcept :
            m_pObject{ std::move(Other.m_pObject) }
        {
            //Make sure original pointer has no RefPtrerences to the object
            Other.m_pObject = nullptr;
        }

        template <typename DerivedType, typename = typename std::enable_if<std::is_base_of<T, DerivedType>::value>::type>
        RefPtr(RefPtr<DerivedType>&& Other) noexcept
            : m_pObject{ std::move(Other.m_pObject) }
        {
            //Make sure original pointer has no RefPtrerences to the object
            Other.m_pObject = nullptr;
        }

        ~RefPtr()
        {
            Reset();
        }

        void Attach(T* pObj) noexcept
        {
            Reset();
            m_pObject = pObj;
        }

        T* Detach() noexcept
        {
            T* pObj = m_pObject;
            m_pObject = nullptr;
            return pObj;
        }

        void Reset() noexcept
        {
            if (m_pObject)
            {
                m_pObject->Release();
                m_pObject = nullptr;
            }
        }

        RefPtr& operator=(T* pObj) noexcept
        {
            if (m_pObject != pObj)
            {
                if (m_pObject)
                    m_pObject->Release();
                m_pObject = pObj;
                if (m_pObject)
                    m_pObject->AddRef();
            }
            return *this;
        }

        RefPtr& operator=(const RefPtr& Other) noexcept
        {
            return *this = Other.m_pObject;
        }

        template <typename DerivedType, typename = typename std::enable_if<std::is_base_of<T, DerivedType>::value>::type>
        RefPtr& operator=(const RefPtr<DerivedType>& Other) noexcept
        {
            return *this = static_cast<T*>(Other.m_pObject);
        }

        RefPtr& operator=(RefPtr&& Other) noexcept
        {
            if (m_pObject != Other.m_pObject)
                Attach(Other.Detach());

            return *this;
        }

        template <typename DerivedType, typename = typename std::enable_if<std::is_base_of<T, DerivedType>::value>::type>
        RefPtr& operator=(RefPtr<DerivedType>&& Other) noexcept
        {
            if (m_pObject != Other.m_pObject)
                Attach(Other.Detach());

            return *this;
        }
        
        bool operator!() const noexcept { return m_pObject == nullptr; }
        explicit operator bool() const noexcept { return m_pObject != nullptr; }
        bool operator==(const RefPtr& Other) const noexcept { return m_pObject == Other.m_pObject; }
        bool operator!=(const RefPtr& Other) const noexcept { return m_pObject != Other.m_pObject; }


        T& operator*() noexcept { return *m_pObject; }
        const T& operator*() const noexcept { return *m_pObject; }

        operator T* () noexcept { return m_pObject; }
        operator const T* () const noexcept { return m_pObject; }

        T* operator->() noexcept { return m_pObject; }
        const T* operator->() const noexcept { return m_pObject; }

        T** operator&() { return &m_pObject; }
        const T** operator&() const { return &m_pObject; }

        T* Raw() noexcept { return m_pObject; }
        const T* Raw() const noexcept { return m_pObject; }

        T** RawDblPtr() noexcept { return &m_pObject; }
        const T** RawDblPtr() const noexcept { return &m_pObject; }

        template<typename DstType>
        DstType* Raw() noexcept { return ValidatedCast<DstType*>(m_pObject); }

        template<typename DstType>
        const DstType* Raw() const noexcept { return ValidatedCast<DstType*>(m_pObject); }

        template<typename DstType>
        DstType** RawDblPtr() noexcept { return &ValidatedCast<DstType*>(m_pObject); }

        template<typename DstType>
        const DstType** RawDblPtr() const noexcept { return &ValidatedCast<DstType*>(m_pObject); }

	private:

		template <typename OtherType>
		friend class RefPtrPtr;

		T* m_pObject = nullptr;
	};

    template <typename T>
       class WeakPtr
       {

           static_assert(std::is_base_of<IRefCountedObject, T>::value, "T must extend Qgfx::IRefCountedObject to be used in Qgfx::WeakPtr");

       public:
           explicit WeakPtr(T* pObj = nullptr) noexcept 
               : m_pRefCounter{ nullptr }
           {
               if (pObj)
               {
                   m_pRefCounter = pObj->AddWeakRef();
               }
           }

           WeakPtr(const WeakPtr& Other) noexcept
               : m_pRefCounter{ Other.m_pRefCounter }
           {
               if (m_pRefCounter)
                   m_pRefCounter->AddWeakRef();
           }

           WeakPtr(WeakPtr&& Other) noexcept
               : m_pRefCounter{ std::move(Other.m_pRefCounter) }
           {
               Other.m_pRefCounter = nullptr;
           }


           ~WeakPtr()
           {
               Reset();
           }

           WeakPtr& operator=(T* pObj) noexcept
           {
               Reset();
               if (pObj)
               {
                   m_pRefCounter = pObj->AddWeakRef();
               }
               return *this;
           }

           WeakPtr& operator=(const WeakPtr& Other) noexcept
           {
               if (*this == Other)
                   return *this;

               Reset();
               m_pRefCounter = Other.m_pRefCounter;
               if (m_pRefCounter)
                   m_pRefCounter->AddWeakRef();
               return *this;
           }

           WeakPtr& operator=(WeakPtr&& Other) noexcept
           {
               if (*this == Other)
                   return *this;

               Reset();
               m_pRefCounter = std::move(Other.m_pRefCounter);
               Other.m_pRefCounter = nullptr;
               return *this;
           }

           void Reset() noexcept
           {
               if (m_pRefCounter)
                   m_pRefCounter->ReleaseWeakRef();
               m_pRefCounter = nullptr;
           }

           /// \note This method may not be reliable in a multithreaded environment.
           ///       However, when false is returned, the strong pointer created from
           ///       this weak pointer will reliably be empty.
           bool IsValid() const noexcept
           {
               return m_pRefCounter != nullptr && m_pRefCounter->GetNumStrongRefs() > 0;
           }

           /// Obtains a strong reference to the object
           void Lock(T** ppObject)
           {
               if (m_pRefCounter)
               {
                   m_pRefCounter->Lock(ppObject);
               }
               else
               {
                   *ppObject = nullptr;
               }
           }

           bool operator==(const WeakPtr& Ptr) const noexcept { return m_pRefCounter == Ptr.m_pRefCounter; }
           bool operator!=(const WeakPtr& Ptr) const noexcept { return m_pRefCounter != Ptr.m_pRefCounter; }

       protected:

           IWeakRefCounter* m_pRefCounter;
       };

//	/**
//	 * @brief Forward declare RefCountedObject.
//	*/
//	class RefCountedObject;
//
//	/**
//	 * @brief Manages the references counts of an object, and allows for weak references (contrary to standard intrusive ref implementations).
//	*/
//	class IRefCounter
//	{
//	public:
//
//		inline Atomics::Long AddStrongRef()
//		{
//			Atomics::AtomicIncrement(m_lNumStrongReferences);
//		}
//
//		template <class TPreObjectDestroy>
//		inline Atomics::Long ReleaseStrongRef(TPreObjectDestroy PreObjectDestroy)
//		{
//			QGFX_VERIFY(m_ObjectState == ObjectState::Alive, "Attempting to decrement strong reference counter for an object that is not alive");
//
//			// Decrement strong reference counter without acquiring the lock.
//			auto RefCount = Atomics::AtomicDecrement(m_lNumStrongReferences);
//			QGFX_VERIFY(RefCount >= 0, "Inconsistent call to ReleaseStrongRef()");
//			if (RefCount == 0)
//			{
//				PreObjectDestroy();
//				TryDestroyObject();
//			}
//
//			return RefCount;
//		}
//
//		inline Atomics::Long ReleaseStrongRef()
//		{
//			return ReleaseStrongRef([]() {});
//		}
//
//		inline Atomics::Long AddWeakRef()
//		{
//			return Atomics::AtomicIncrement(m_lNumWeakReferences);
//		}
//
//		inline Atomics::Long ReleaseWeakRef()
//		{
//			// The method must be serialized!
//			SpinLock Lock(m_LockFlag);
//			// It is essentially important to check the number of weak references
//			// while holding the lock. Otherwise reference counters object
//			// may be destroyed twice if ReleaseStrongRef() is executed by other
//			// thread.
//			auto NumWeakReferences = Atomics::AtomicDecrement(m_lNumWeakReferences);
//			QGFX_VERIFY(NumWeakReferences >= 0, "Inconsistent call to ReleaseWeakRef()");
//
//			
//			if (NumWeakReferences == 0 && m_ObjectState == ObjectState::Destroyed)
//			{
//				QGFX_VERIFY_EXPR(m_lNumStrongReferences == 0);
//				// m_ObjectState is set to ObjectState::Destroyed under the lock. If the state is not Destroyed,
//				// ReleaseStrongRef() will take care of it.
//				// Access to Object wrapper and decrementing m_lNumWeakReferences is atomic. Since we acquired the lock,
//				// no other thread can access either of them.
//				// Access to m_lNumStrongReferences is NOT PROTECTED by lock.
//
//				// There are no more references to the ref counters object and the object itself
//				// is already destroyed.
//				// We can safely unlock it and destroy.
//				// If we do not unlock it, this->m_LockFlag will expire,
//				// which will cause Lock.~LockHelper() to crash.
//				Lock.Unlock();
//				DeleteThis();
//			}
//			return NumWeakReferences;
//		}
//
//		inline void GetObject(RefCountedObject** ppObject)
//		{
//			if (m_ObjectState != ObjectState::Alive)
//				return; // Early exit
//
//			// It is essential to INCREMENT REF COUNTER while HOLDING THE LOCK to make sure that
//			// StrongRefCnt > 1 guarantees that the object is alive.
//
//			// If other thread started deleting the object in ReleaseStrongRef(), then m_lNumStrongReferences==0
//			// We must make sure only one thread is allowed to increment the counter to guarantee that if StrongRefCnt > 1,
//			// there is at least one real strong reference left. Otherwise the following scenario may occur:
//			//
//			//                                      m_lNumStrongReferences == 1
//			//
//			//    Thread 1 - ReleaseStrongRef()    |     Thread 2 - GetObject()        |     Thread 3 - GetObject()
//			//                                     |                                   |
//			//  - Decrement m_lNumStrongReferences | -Increment m_lNumStrongReferences | -Increment m_lNumStrongReferences
//			//  - Read RefCount == 0               | -Read StrongRefCnt==1             | -Read StrongRefCnt==2
//			//    Destroy the object               |                                   | -Return reference to the soon
//			//                                     |                                   |  to expire object
//			//
//
//			SpinLock Lock(m_LockFlag);
//
//			auto StrongRefCnt = Atomics::AtomicIncrement(m_lNumStrongReferences);
//
//			// Checking if m_ObjectState == ObjectState::Alive only is not reliable:
//			//
//			//           This thread                    |          Another thread
//			//                                          |
//			//   1. Acquire the lock                    |
//			//                                          |    1. Decrement m_lNumStrongReferences
//			//   2. Increment m_lNumStrongReferences    |    2. Test RefCount==0
//			//   3. Read StrongRefCnt == 1              |    3. Start destroying the object
//			//      m_ObjectState == ObjectState::Alive |
//			//   4. DO NOT return the reference to      |    4. Wait for the lock, m_ObjectState == ObjectState::Alive
//			//      the object                          |
//			//   5. Decrement m_lNumStrongReferences    |
//			//                                          |    5. Destroy the object
//
//			if (m_ObjectState == ObjectState::Alive && StrongRefCnt > 1)
//			{
//				*ppObject = GetRefCountedObject();
//			}
//
//			if (*ppObject == nullptr)
//			{
//				Atomics::AtomicDecrement(m_lNumStrongReferences);
//			}
//		}
//
//		inline Atomics::Long GetNumStrongRefs() const
//		{
//			return m_lNumStrongReferences;
//		}
//
//		inline Atomics::Long GetNumWeakRefs() const
//		{
//			return m_lNumWeakReferences;
//		}
//
//	private:
//
//		virtual RefCountedObject* GetRefCountedObject() const = 0;
//
//		virtual void DestroyObject() = 0;
//
//		virtual void DeleteThis() = 0;
//
//		IRefCounter() noexcept
//		{
//			m_lNumStrongReferences = 0;
//			m_lNumWeakReferences = 0;
//		}
//
//		~IRefCounter() = default;
//
//		void TryDestroyObject()
//		{
//#ifdef QGFX_DEBUG
//			{
//				Atomics::Long NumStrongRefs = m_lNumStrongReferences;
//				QGFX_VERIFY(NumStrongRefs == 0 || NumStrongRefs == 1, "Num strong references (", NumStrongRefs, ") is expected to be 0 or 1");
//			}
//#endif
//
//			SpinLock Lock(m_LockFlag);
//
//			// GetObject() first acquires the lock, and only then increments and
//			// decrements the ref counter. If it reads 1 after incremeting the counter,
//			// it does not return the reference to the object and decrements the counter.
//			// If we acquired the lock, GetObject() will not start until we are done
//			QGFX_VERIFY_EXPR(m_lNumStrongReferences == 0 && m_ObjectState == ObjectState::Alive);
//
//			// Extra caution
//			if (m_lNumStrongReferences == 0 && m_ObjectState == ObjectState::Alive)
//			{
//				// We cannot destroy the object while reference counters are locked as this will
//				// cause a deadlock in cases like this:
//				//
//				//    A ==sp==> B ---wp---> A
//				//
//				//    RefCounters_A.Lock();
//				//    delete A{
//				//      A.~dtor(){
//				//          B.~dtor(){
//				//              wpA.ReleaseWeakRef(){
//				//                  RefCounters_A.Lock(); // Deadlock
//				//
//
//				// So we destroy the object after unlocking the reference counters
//
//				// In a multithreaded environment, reference counters object may
//				// be destroyed at any time while m_pObject->~dtor() is running.
//
//				// Note that this is the only place where m_ObjectState is
//				// modified after the ref counters object has been created
//				m_ObjectState = ObjectState::Destroyed;
//				// The object is now detached from the reference counters and it is if
//				// it was destroyed since no one can obtain access to it.
//
//
//				// It is essentially important to check the number of weak references
//				// while the object is locked. Otherwise reference counters object
//				// may be destroyed twice if ReleaseWeakRef() is executed by other thread:
//				//
//				//             This thread             |    Another thread - ReleaseWeakRef()
//				//                                     |
//				// 1. Decrement m_lNumStrongReferences,|
//				//    m_lNumStrongReferences==0,       |
//				//    acquire the lock, destroy        |
//				//    the obj, release the lock        |
//				//    m_lNumWeakReferences == 1        |
//				//                                     |   1. Aacquire the lock,
//				//                                     |      decrement m_lNumWeakReferences,
//				//                                     |      m_lNumWeakReferences == 0,
//				//                                     |      m_ObjectState == ObjectState::Destroyed
//				//                                     |
//				// 2. Read m_lNumWeakReferences == 0   |
//				// 3. Destroy the ref counters obj     |   2. Destroy the ref counters obj
//				//
//				bool bDeleteThiss = m_lNumWeakReferences == 0;
//				// ReleaseWeakRef() decrements m_lNumWeakReferences, and checks it for
//				// zero only after acquiring the lock. So if m_lNumWeakReferences==0, no
//				// weak reference-related code may be running
//
//
//				// We must explicitly unlock the object now to avoid deadlocks. Also,
//				// if this is deleted, this->m_LockFlag will expire, which will cause
//				// Lock.~LockHelper() to crash
//				Lock.Unlock();
//
//				// Call the attached object's destructor
//				DestroyObject();
//
//				// Note that <this> may be destroyed here already,
//				// see comments in ~ControlledObjectType()
//				if (bDeleteThiss) // Free the object, its memory, and this reference counter.
//					DeleteThis();
//			}
//		}
//
//		// IRefCountedAllocation* m_pAllocation;
//
//		Atomics::AtomicLong m_lNumStrongReferences;
//		Atomics::AtomicLong m_lNumWeakReferences;
//		SpinLockFlag m_LockFlag;
//
//		enum class ObjectState : uint32_t
//		{
//			NotInitialized,
//			Alive,
//			Destroyed
//		};
//		volatile ObjectState m_ObjectState = ObjectState::NotInitialized;
//	};
//
//	template<typename ObjectType, typename DeleterType>
//	class RefCounterWithDeleter : private DeleterType, public IRefCounter
//	{
//	public:
//
//		virtual RefCountedObject* GetRefCountedObject() const
//		{
//			return m_pObject;
//		}
//
//		virtual void DestroyObject()
//		{
//			(*static_cast<DeleterType*>(this))(m_pObject);
//		}
//
//		virtual void DeleteThis()
//		{
//			delete this;
//		}
//
//	private:
//
//		ObjectType* m_pObject;
//
//		RefCounterWithDeleter
//
//	};
//
//	template<typename ObjectType>
//	class IntrusiveRefCounter :  public IRefCounter
//	{
//	public:
//
//		virtual RefCountedObject* GetRefCountedObject() const
//		{
//			return m_pObject;
//		}
//
//		virtual void DestroyObject()
//		{
//			(*static_cast<DeleterType*>(this))(m_pObject);
//		}
//
//		virtual void DeleteThis()
//		{
//			delete this;
//		}
//
//	private:
//
//		TypeCompatibleBytes<ObjectType> m_ObjectStorage;
//
//	};
//
//	/**
//	 * @brief Base class from which all RefCounted classes derive.
//	*/
//	class RefCountedObject
//	{
//	public:
//
//		RefCountedObject(IRefCounter* pRefCounter)
//			: m_pRefCounter(pRefCounter)
//		{
//		}
//
//		/**
//		 * @brief Increases strong references of RefCountedObject by 1.
//		 * @return The number of strong references after the variable has been incremented.
//		*/
//		inline Atomics::Long AddRef()
//		{
//			m_pRefCounter->AddStrongRef();
//		}
//
//		virtual Atomics::Long Release()
//		{
//			m_pRefCounter->ReleaseStrongRef();
//		}
//
//		inline IRefCounter* GetRefCounter()
//		{
//			return m_pRefCounter;
//		}
//
//	private:
//
//		IRefCounter* m_pRefCounter;
//	};
//
//	template<typename ObjectType, typename AllocatorType>
//	class RefCounterImpl final : public IRefCounter
//	{
//	public:
//
//		virtual RefCountedObject* GetRefCountedObject() const override final
//		{
//			return ObjectPtr();
//		}
//
//	private:
//
//		virtual void DestructObject() override final
//		{
//			ObjectPtr()->ObjectType::~ObjectType();
//		}
//
//		virtual void FreeSelf() override final
//		{
//			// Call RefCounter's destructor
//			this->IRefCounter::~IRefCounter();
//
//			if (m_pAllocator)
//				m_pAllocator->Free(this));
//			else
//				delete[] reinterpret_cast<uint8_t*>(this);
//		}
//
//		ObjectType* ObjectPtr() const
//		{
//			return reinterpret_cast<ObjectType*>(m_ObjectStorage);
//		}
//
//		alignas(ObjectType) uint8_t m_ObjectStorage[sizeof(ObjectType)];
//		AllocatorType* m_pAllocator;
//
//	private:
//
//		template<typename ObjectType, typename AllocatorType>
//		friend class MakeRefCountedObj;
//
//		RefCounterImpl(AllocatorType* pAllocator)
//			: m_pAllocator{ pAllocator }
//		{
//		}
//
//		static RefCounterImpl* Create(AllocatorType* pAllocator)
//		{
//			RefCounterImpl* pRefCounter;
//			if (pAllocator)
//				pRefCounter = reinterpret_cast<RefCounterImpl*>(pAllocator->Allocate(sizeof(RefCounterImpl)));
//			else
//				pRefCounter = reinterpret_cast<RefCounterImpl*>(new uint8_t[sizeof(RefCounterImpl)]);
//
//			new(pRefCounter) RefCounterImpl(pAllocator);
//
//			return pRefCounter;
//		}
//
//	};
//
//	/**
//	 * @brief Interface for creating new ref counted objects.
//	 * @tparam ObjectType
//	 * @tparam AllocatorType 
//	*/
//	template<typename ObjectType, typename AllocatorType = IMemoryAllocator>
//	class MakeRefCountedObj
//	{
//	public:
//
//		explicit MakeRefCountedObj()
//			: m_pAllocator(nullptr)
//		{
//		}
//
//		explicit MakeRefCountedObj(AllocatorType& Allocator)
//			: m_pAllocator(&Allocator)
//		{
//		}
//
//		template<typename ... CtorArgTypes>
//		ObjectType* operator() (CtorArgTypes&& ... CtorArgs)
//		{
//			RefCounterImpl<ObjectType, AllocatorType>* pRefCounter = RefCounterImpl<ObjectType, AllocatorType>::Create(m_pAllocator);
//
//			new (pRefCounter->ObjectPtr()) ObjectType(pRefCounter, std::forward<CtorArgTypes>(CtorArgs)...);
//
//			pRefCounter->AddStrongRef();
//
//			return pRefCounter->ObjectPtr();
//		}
//
//	private:
//
//		AllocatorType* m_pAllocator;
//	};

//template<typename T>
    //class RefPtr
    //{
 //       static_assert(std::is_base_of<IObject, T>::value, "T must extend Qgfx::IObject to be used in Qgfx::RefPtr");

    //public:

 //       RefPtr() noexcept {}

 //       explicit RefPtr(T* pObj) noexcept 
 //           : m_pObject{ pObj }
 //       {
 //           if (m_pObject)
 //               m_pObject->AddRef();
 //       }

 //       RefPtr(const RefPtr& Other) noexcept 
 //           : m_pObject{ Other.m_pObject }
 //       {
 //           if (m_pObject)
 //               m_pObject->AddRef();
 //       }

 //       template <typename DerivedType, typename = typename std::enable_if<std::is_base_of<T, DerivedType>::value>::type>
 //       RefPtr(const RefPtr<DerivedType>& Other) noexcept 
 //           : RefPtr<T>{ Other.m_pObject }
 //       {
 //       }

 //       RefPtr(RefPtr&& Other) noexcept :
 //           m_pObject{ std::move(Other.m_pObject) }
 //       {
 //           //Make sure original pointer has no references to the object
 //           Other.m_pObject = nullptr;
 //       }

 //       template <typename DerivedType, typename = typename std::enable_if<std::is_base_of<T, DerivedType>::value>::type>
 //       RefPtr(RefPtr<DerivedType>&& Other) noexcept
 //           : m_pObject{ std::move(Other.m_pObject) }
 //       {
 //           //Make sure original pointer has no references to the object
 //           Other.m_pObject = nullptr;
 //       }

 //       ~RefPtr()
 //       {
 //           Reset();
 //       }

 //       void Attach(T* pObj) noexcept
 //       {
 //           Reset();
 //           m_pObject = pObj;
 //       }

 //       T* Detach() noexcept
 //       {
 //           T* pObj = m_pObject;
 //           m_pObject = nullptr;
 //           return pObj;
 //       }

 //       void Reset() noexcept
 //       {
 //           if (m_pObject)
 //           {
 //               m_pObject->Release();
 //               m_pObject = nullptr;
 //           }
 //       }

 //       RefPtr& operator=(T* pObj) noexcept
 //       {
 //           if (m_pObject != pObj)
 //           {
 //               if (m_pObject)
 //                   m_pObject->Release();
 //               m_pObject = pObj;
 //               if (m_pObject)
 //                   m_pObject->AddRef();
 //           }
 //           return *this;
 //       }

 //       RefPtr& operator=(const RefPtr& Other) noexcept
 //       {
 //           return *this = Other.m_pObject;
 //       }

 //       template <typename DerivedType, typename = typename std::enable_if<std::is_base_of<T, DerivedType>::value>::type>
 //       RefPtr& operator=(const RefPtr<DerivedType>& Other) noexcept
 //       {
 //           return *this = static_cast<T*>(Other.m_pObject);
 //       }

 //       RefPtr& operator=(RefPtr&& Other) noexcept
 //       {
 //           if (m_pObject != Other.m_pObject)
 //               Attach(Other.Detach());

 //           return *this;
 //       }

 //       template <typename DerivedType, typename = typename std::enable_if<std::is_base_of<T, DerivedType>::value>::type>
 //       RefPtr& operator=(RefPtr<DerivedType>&& Other) noexcept
 //       {
 //           if (m_pObject != Other.m_pObject)
 //               Attach(Other.Detach());

 //           return *this;
 //       }
 //       
 //       bool operator!() const noexcept { return m_pObject == nullptr; }
 //       explicit operator bool() const noexcept { return m_pObject != nullptr; }
 //       bool operator==(const RefPtr& Other) const noexcept { return m_pObject == Other.m_pObject; }
 //       bool operator!=(const RefPtr& Other) const noexcept { return m_pObject != Other.m_pObject; }


 //       T& operator*() noexcept { return *m_pObject; }
 //       const T& operator*() const noexcept { return *m_pObject; }

 //       operator T* () noexcept { return m_pObject; }
 //       operator const T* () const noexcept { return m_pObject; }

 //       T* operator->() noexcept { return m_pObject; }
 //       const T* operator->() const noexcept { return m_pObject; }

 //       T** operator&() { return &m_pObject; }
 //       const T** operator&() const { return &m_pObject; }

 //       T* Raw() noexcept { return m_pObject; }
 //       const T* Raw() const noexcept { return m_pObject; }

 //       T** RawDblPtr() noexcept { return &m_pObject; }
 //       const T** RawDblPtr() const noexcept { return &m_pObject; }

 //       template<typename DstType>
 //       DstType* Raw() noexcept { return ValidatedCast<DstType*>(m_pObject); }

 //       template<typename DstType>
 //       const DstType* Raw() const noexcept { return ValidatedCast<DstType*>(m_pObject); }

 //       template<typename DstType>
 //       DstType** RawDblPtr() noexcept { return &ValidatedCast<DstType*>(m_pObject); }

 //       template<typename DstType>
 //       const DstType** RawDblPtr() const noexcept { return &ValidatedCast<DstType*>(m_pObject); }

 //       template<typename DstType>
 //       DstType* Raw() noexcept { return ValidatedCast<DstType*>(m_pObject); }

 //       template<typename DstType>
 //       const DstType* Raw() const noexcept { return ValidatedCast<DstType*>(m_pObject); }

    //private:

    //	template <typename OtherType>
    //	friend class RefPtr;

    //	T* m_pObject = nullptr;
    //};

 //   template <typename T>
 //   class WeakPtr
 //   {

 //       static_assert(std::is_base_of<IObject, T>::value, "T must extend Qgfx::IObject to be used in Qgfx::WeakPtr");

 //   public:
 //       explicit WeakPtr(T* pObj = nullptr) noexcept 
 //           : m_pRefCounter{ nullptr }
 //       {
 //           if (pObj)
 //           {
 //               m_pRefCounter = pObj->GetRefCounter();
 //               m_pRefCounter->AddWeakRef();
 //           }
 //       }

 //       WeakPtr(const WeakPtr& Other) noexcept
 //           : m_pRefCounter{ Other.m_pRefCounter }
 //       {
 //           if (m_pRefCounter)
 //               m_pRefCounter->AddWeakRef();
 //       }

 //       WeakPtr(WeakPtr&& Other) noexcept
 //           : m_pRefCounter{ std::move(Other.m_pRefCounter) }
 //       {
 //           Other.m_pRefCounter = nullptr;
 //       }


 //       ~WeakPtr()
 //       {
 //           Reset();
 //       }

 //       WeakPtr& operator=(T* pObj) noexcept
 //       {
 //           Reset();
 //           if (pObj)
 //           {
 //               m_pRefCounter = pObj->GetRefCounter();
 //               m_pRefCounter->AddWeakRef();
 //           }
 //           return *this;
 //       }

 //       WeakPtr& operator=(const WeakPtr& Other) noexcept
 //       {
 //           if (*this == Other)
 //               return *this;

 //           Reset();
 //           m_pRefCounter = Other.m_pRefCounter;
 //           if (m_pRefCounter)
 //               m_pRefCounter->AddWeakRef();
 //           return *this;
 //       }

 //       WeakPtr& operator=(WeakPtr&& Other) noexcept
 //       {
 //           if (*this == Other)
 //               return *this;

 //           Reset();
 //           m_pRefCounter = std::move(Other.m_pRefCounter);
 //           Other.m_pRefCounter = nullptr;
 //           return *this;
 //       }

 //       void Reset() noexcept
 //       {
 //           if (m_pRefCounter)
 //               m_pRefCounter->ReleaseWeakRef();
 //           m_pRefCounter = nullptr;
 //       }

 //       /// \note This method may not be reliable in a multithreaded environment.
 //       ///       However, when false is returned, the strong pointer created from
 //       ///       this weak pointer will reliably be empty.
 //       bool IsValid() const noexcept
 //       {
 //           return m_pRefCounter != nullptr && m_pRefCounter->GetNumStrongRefs() > 0;
 //       }

 //       /// Obtains a strong reference to the object
 //       RefPtr<T> Lock()
 //       {
 //           RefPtr<T> spObj;
 //           if (m_pRefCounter)
 //           {
 //               m_pRefCounter->GetObject(&spObj);
 //           }
 //           return spObj;
 //       }

 //       bool operator==(const WeakPtr& Ptr) const noexcept { return m_pRefCounter == Ptr.m_pRefCounter; }
 //       bool operator!=(const WeakPtr& Ptr) const noexcept { return m_pRefCounter != Ptr.m_pRefCounter; }

 //   protected:

 //       IRefCounter* m_pRefCounter;
 //   };
}