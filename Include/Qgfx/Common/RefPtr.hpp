#pragma once

#include "RefCountedObject.hpp"
#include "ValidatedCast.hpp"

#include <type_traits>
#include <utility>

namespace Qgfx
{
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