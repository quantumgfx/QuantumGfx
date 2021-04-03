#pragma once

#include "Object.hpp"
#include "ValidatedCast.hpp"

#include <type_traits>
#include <utility>

namespace Qgfx
{
	template<typename T>
	class RefAutoPtr
	{
        static_assert(std::is_base_of<IObject, T>::value, "T must extend Qgfx::IObject to be used in Qgfx::RefAutoPtr");

	public:

        RefAutoPtr() noexcept {}

        explicit RefAutoPtr(T* pObj) noexcept 
            : m_pObject{ pObj }
        {
            if (m_pObject)
                m_pObject->AddRef();
        }

        RefAutoPtr(const RefAutoPtr& Other) noexcept 
            : m_pObject{ Other.m_pObject }
        {
            if (m_pObject)
                m_pObject->AddRef();
        }

        template <typename DerivedType, typename = typename std::enable_if<std::is_base_of<T, DerivedType>::value>::type>
        RefAutoPtr(const RefAutoPtr<DerivedType>& Other) noexcept 
            : RefAutoPtr<T>{ Other.m_pObject }
        {
        }

        RefAutoPtr(RefAutoPtr&& Other) noexcept :
            m_pObject{ std::move(Other.m_pObject) }
        {
            //Make sure original pointer has no references to the object
            Other.m_pObject = nullptr;
        }

        template <typename DerivedType, typename = typename std::enable_if<std::is_base_of<T, DerivedType>::value>::type>
        RefAutoPtr(RefAutoPtr<DerivedType>&& Other) noexcept
            : m_pObject{ std::move(Other.m_pObject) }
        {
            //Make sure original pointer has no references to the object
            Other.m_pObject = nullptr;
        }

        ~RefAutoPtr()
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

        RefAutoPtr& operator=(T* pObj) noexcept
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

        RefAutoPtr& operator=(const RefAutoPtr& Other) noexcept
        {
            return *this = Other.m_pObject;
        }

        template <typename DerivedType, typename = typename std::enable_if<std::is_base_of<T, DerivedType>::value>::type>
        RefAutoPtr& operator=(const RefAutoPtr<DerivedType>& Other) noexcept
        {
            return *this = static_cast<T*>(Other.m_pObject);
        }

        RefAutoPtr& operator=(RefAutoPtr&& Other) noexcept
        {
            if (m_pObject != Other.m_pObject)
                Attach(Other.Detach());

            return *this;
        }

        template <typename DerivedType, typename = typename std::enable_if<std::is_base_of<T, DerivedType>::value>::type>
        RefAutoPtr& operator=(RefAutoPtr<DerivedType>&& Other) noexcept
        {
            if (m_pObject != Other.m_pObject)
                Attach(Other.Detach());

            return *this;
        }
        
        bool operator!() const noexcept { return m_pObject == nullptr; }
        explicit operator bool() const noexcept { return m_pObject != nullptr; }
        bool operator==(const RefAutoPtr& Other) const noexcept { return m_pObject == Other.m_pObject; }
        bool operator!=(const RefAutoPtr& Other) const noexcept { return m_pObject != Other.m_pObject; }


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

        template<typename DstType>
        DstType* As() noexcept { return ValidatedCast<DstType*>(m_pObject); }

        template<typename DstType>
        const DstType* As() const noexcept { return ValidatedCast<DstType*>(m_pObject); }

	private:

		template <typename OtherType>
		friend class RefAutoPtr;

		T* m_pObject = nullptr;
	};

    template <typename T>
    class RefWeakPtr
    {

        static_assert(std::is_base_of<IObject, T>::value, "T must extend Qgfx::IObject to be used in Qgfx::RefWeakPtr");

    public:
        explicit RefWeakPtr(T* pObj = nullptr) noexcept 
            : m_pRefCounter{ nullptr }
        {
            if (pObj)
            {
                m_pRefCounter = pObj->GetRefCounter();
                m_pRefCounter->AddWeakRef();
            }
        }

        RefWeakPtr(const RefWeakPtr& Other) noexcept
            : m_pRefCounter{ Other.m_pRefCounter }
        {
            if (m_pRefCounter)
                m_pRefCounter->AddWeakRef();
        }

        RefWeakPtr(RefWeakPtr&& Other) noexcept
            : m_pRefCounter{ std::move(Other.m_pRefCounter) }
        {
            Other.m_pRefCounter = nullptr;
        }


        ~RefWeakPtr()
        {
            Reset();
        }

        RefWeakPtr& operator=(T* pObj) noexcept
        {
            Reset();
            if (pObj)
            {
                m_pRefCounter = pObj->GetRefCounter();
                m_pRefCounter->AddWeakRef();
            }
            return *this;
        }

        RefWeakPtr& operator=(const RefWeakPtr& Other) noexcept
        {
            if (*this == Other)
                return *this;

            Reset();
            m_pRefCounter = Other.m_pRefCounter;
            if (m_pRefCounter)
                m_pRefCounter->AddWeakRef();
            return *this;
        }

        RefWeakPtr& operator=(RefWeakPtr&& Other) noexcept
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
        RefAutoPtr<T> Lock()
        {
            RefAutoPtr<T> spObj;
            if (m_pRefCounter)
            {
                m_pRefCounter->GetObject(&spObj);
            }
            return spObj;
        }

        bool operator==(const RefWeakPtr& Ptr) const noexcept { return m_pRefCounter == Ptr.m_pRefCounter; }
        bool operator!=(const RefWeakPtr& Ptr) const noexcept { return m_pRefCounter != Ptr.m_pRefCounter; }

    protected:

        RefCounter* m_pRefCounter;
    };
}