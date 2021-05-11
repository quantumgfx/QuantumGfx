#pragma once

#include <unordered_map>
#include <utility>

#include "IObject.hpp"

#include "../Common/Error.hpp"
#include "../Common/MemoryAllocator.hpp"
#include "../Common/RefPtr.hpp"
#include "../Common/SpinLock.hpp"
#include "../Common/STDAllocator.hpp"

namespace Qgfx
{
    /// Template class implementing state object registry
    template <typename ResourceDescType, int DeletedObjectsToPurge = 32>
    class StateObjectsRegistry
    {
    public:

        using HashMapElem = std::pair<const ResourceDescType, WeakPtr<IObject>>;

        StateObjectsRegistry(IMemoryAllocator& RawAllocator) :
            m_NumDeletedObjects{ 0 },
            m_DescToObjHashMap(STDAllocatorRawMem<HashMapElem>(RawAllocator)),
        {}

        ~StateObjectsRegistry()
        {
            // Object registry is part of the device, and every device
            // object holds a strong reference to the device. So device
            // is destroyed after all device objects are destroyed, and there
            // may only be expired references in the registry. After we
            // purge it, the registry must be empty.
            Purge();
            QGFX_VERIFY(m_DescToObjHashMap.empty(), "DescToObjHashMap is not empty");
        }

        /// Adds a new object to the registry

        /// \param [in] ObjectDesc - object description.
        /// \param [in] pObject - pointer to the object.
        ///
        /// Besides adding a new object, the function also checks the number of
        /// outstanding deleted objects and calls Purge() if the number has reached
        /// the threshold value DeletedObjectsToPurge. Creating a state object is
        /// assumed to be an expensive operation and should be performed during
        /// the initialization. Occasional purge operations should not add significant
        /// cost to it.
        void Add(const ResourceDescType& ObjectDesc, IObject* pObject)
        {
            SpinLock Lock(m_LockFlag);

            // If the number of outstanding deleted objects reached the threshold value,
            // purge the registry. Since we have exclusive access now, it is safe
            // to do.
            if (m_NumDeletedObjects >= DeletedObjectsToPurge)
            {
                Purge();
                m_NumDeletedObjects = 0;
            }

            // Try to construct the new element in place
            auto Elems = m_DescToObjHashMap.emplace(std::make_pair(ObjectDesc, WeakPtr<IObject>(pObject)));
            // It is theorertically possible that the same object can be found
            // in the registry. This might happen if two threads try to create
            // the same object at the same time. They both will not find the
            // object and then will create and try to add it.
            //
            // If the object already exists, we replace the existing reference.
            // This is safer as there might be scenarios where existing reference
            // might be expired. For instance, two threads try to create the same
            // object which is not in the registry. The first thread creates
            // the object, adds it to the registry and then releases it. After that
            // the second thread creates the same object and tries to add it to
            // the registry. It will find an existing expired reference to the
            // object.
            if (!Elems.second)
            {
                QGFX_VERIFY(Elems.first->first == ObjectDesc, "Incorrect object description");
                QGFX_LOG_WARNING_MESSAGE("Object named '", Elems.first->first.Name,
                    "' with the same description already exists in the registry."
                    "Replacing with the new object named '",
                    ObjectDesc.Name ? ObjectDesc.Name : "", "'.");
                Elems.first->second = pObject;
            }
        }

        /// Finds the object in the registry
        void Find(const ResourceDescType& Desc, IObject** ppObject)
        {
            QGFX_VERIFY(*ppObject == nullptr, "Overwriting reference to existing object may cause memory leaks");
            *ppObject = nullptr;

            SpinLock Lock(m_LockFlag);

            auto It = m_DescToObjHashMap.find(Desc);
            if (It != m_DescToObjHashMap.end())
            {
                // Try to obtain strong reference to the object.
                // This is an atomic operation and we either get
                // a new strong reference or object has been destroyed
                // and we get null.
                auto pObject = It->second.Lock();
                if (pObject)
                {
                    *ppObject = pObject.Detach();
                    //LOG_INFO_MESSAGE( "Equivalent of the requested state object named \"", Desc.Name ? Desc.Name : "", "\" found in the ", m_RegistryName, " registry. Reusing existing object.");
                }
                else
                {
                    // Expired object found: remove it from the map
                    m_DescToObjHashMap.erase(It);
                    Atomics::AtomicDecrement(m_NumDeletedObjects);
                }
            }
        }

        /// Purges outstanding deleted objects from the registry
        void Purge()
        {
            uint32_t NumPurgedObjects = 0;
            auto It = m_DescToObjHashMap.begin();
            while (It != m_DescToObjHashMap.end())
            {
                auto NextIt = It;
                ++NextIt;
                // Note that IsValid() is not a thread-safe function in the sense that it
                // can give false positive results. The only thread-safe way to check if the
                // object is alive is to lock the weak pointer, but that requires thread
                // synchronization. We will immediately unlock the pointer anyway, so we
                // want to detect 100% expired pointers. IsValid() does provide that information
                // because once a weak pointer becomes invalid, it will be invalid
                // until it is destroyed. It is not a problem if we miss an expired weak
                // pointer as it will definitiely be removed next time.
                if (!It->second.IsValid())
                {
                    m_DescToObjHashMap.erase(It);
                    ++NumPurgedObjects;
                }

                It = NextIt;
            }
            QGFX_LOG_INFO_MESSAGE("Purged ", NumPurgedObjects, " deleted objects from registry");
        }

        /// Increments the number of outstanding deleted objects.
        /// When this number reaches DeletedObjectsToPurge, Purge() will
        /// be called.
        void ReportDeletedObject()
        {
            Atomics::AtomicIncrement(m_NumDeletedObjects);
        }

    private:
        /// Lock flag to protect the m_DescToObjHashMap
        SpinLockFlag m_LockFlag;

        /// Nmber of outstanding deleted objects that have not been purged
        Atomics::AtomicLong m_NumDeletedObjects;

        /// Hash map that stores weak pointers to the referenced objects
        
        std::unordered_map<ResourceDescType, WeakPtr<IObject>, std::hash<ResourceDescType>, std::equal_to<ResourceDescType>, STDAllocatorRawMem<HashMapElem>> m_DescToObjHashMap;
    };
}