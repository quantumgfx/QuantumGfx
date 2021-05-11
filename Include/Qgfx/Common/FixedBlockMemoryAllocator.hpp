#pragma once

#include <unordered_map>
#include <mutex>
#include <unordered_set>
#include <vector>
#include <cstring>
#include <memory>
#include <cstdint>

#include "Error.hpp"
#include "MemoryAllocator.hpp"
#include "STDAllocator.hpp"

namespace Qgfx
{
    class FixedBlockMemoryAllocator final : public IMemoryAllocator
    {
    public:
        FixedBlockMemoryAllocator(IMemoryAllocator& RawMemoryAllocator, size_t BlockSize, uint32_t NumBlocksInPage);
        ~FixedBlockMemoryAllocator();

        /// Allocates block of memory
        virtual void* Allocate(size_t Size) override final;

        /// Releases memory
        virtual void Free(void* Ptr) override final;

    private:
        // clang-format off
        FixedBlockMemoryAllocator(const FixedBlockMemoryAllocator&) = delete;
        FixedBlockMemoryAllocator(FixedBlockMemoryAllocator&&) = delete;
        FixedBlockMemoryAllocator& operator = (const FixedBlockMemoryAllocator&) = delete;
        FixedBlockMemoryAllocator& operator = (FixedBlockMemoryAllocator&&) = delete;
        // clang-format on

        void CreateNewPage();

#ifdef QGFX_DEBUG
        void dbgVerifyAddress(const void* pBlockAddr) const
        {
            size_t Delta = reinterpret_cast<const uint8_t*>(pBlockAddr) - reinterpret_cast<uint8_t*>(m_pPageStart);
            VERIFY(Delta % m_pOwnerAllocator->m_BlockSize == 0, "Invalid address");
            uint32_t BlockIndex = static_cast<uint32_t>(Delta / m_pOwnerAllocator->m_BlockSize);
            QGFX_VERIFY(BlockIndex >= 0 && BlockIndex < m_pOwnerAllocator->m_NumBlocksInPage, "Invalid block index");
        }

        inline void dbgFillPattern(void* ptr, uint8_t Pattern, size_t NumBytes)
        {
            memset(ptr, Pattern, NumBytes);
        }
#else
#    define dbgFillPattern(...)
#    define dbgVerifyAddress(...)
#endif

        // Memory page class is based on the fixed-size memory pool described in "Fast Efficient Fixed-Size Memory Pool"
        // by Ben Kenwright
        class MemoryPage
        {
        public:
            static constexpr uint8_t NewPageMemPattern = 0xAA;
            static constexpr uint8_t AllocatedBlockMemPattern = 0xAB;
            static constexpr uint8_t DeallocatedBlockMemPattern = 0xDE;
            static constexpr uint8_t InitializedBlockMemPattern = 0xCF;

            MemoryPage(FixedBlockMemoryAllocator& OwnerAllocator) :
                // clang-format off
                m_NumFreeBlocks{ OwnerAllocator.m_NumBlocksInPage },
                m_NumInitializedBlocks{ 0 },
                m_pOwnerAllocator{ &OwnerAllocator }
                // clang-format on
            {
                auto PageSize = OwnerAllocator.m_BlockSize * OwnerAllocator.m_NumBlocksInPage;
                m_pPageStart = reinterpret_cast<uint8_t*>(
                    OwnerAllocator.m_RawMemoryAllocator.Allocate(PageSize));
                m_pNextFreeBlock = m_pPageStart;
                dbgFillPattern(m_pPageStart, NewPageMemPattern, PageSize);
            }

            MemoryPage(MemoryPage&& Page) noexcept :
                // clang-format off
                m_NumFreeBlocks{ Page.m_NumFreeBlocks },
                m_NumInitializedBlocks{ Page.m_NumInitializedBlocks },
                m_pPageStart{ Page.m_pPageStart },
                m_pNextFreeBlock{ Page.m_pNextFreeBlock },
                m_pOwnerAllocator{ Page.m_pOwnerAllocator }
                // clang-format on
            {
                Page.m_NumFreeBlocks = 0;
                Page.m_NumInitializedBlocks = 0;
                Page.m_pPageStart = nullptr;
                Page.m_pNextFreeBlock = nullptr;
                Page.m_pOwnerAllocator = nullptr;
            }

            ~MemoryPage()
            {
                if (m_pOwnerAllocator)
                    m_pOwnerAllocator->m_RawMemoryAllocator.Free(m_pPageStart);
            }

            void* GetBlockStartAddress(uint32_t BlockIndex) const
            {
                QGFX_VERIFY_EXPR(m_pOwnerAllocator != nullptr);
                QGFX_VERIFY(BlockIndex >= 0 && BlockIndex < m_pOwnerAllocator->m_NumBlocksInPage, "Invalid block index");
                return reinterpret_cast<uint8_t*>(m_pPageStart) + BlockIndex * m_pOwnerAllocator->m_BlockSize;
            }

            void* Allocate()
            {
                QGFX_VERIFY_EXPR(m_pOwnerAllocator != nullptr);

                if (m_NumFreeBlocks == 0)
                {
                    QGFX_VERIFY_EXPR(m_NumInitializedBlocks == m_pOwnerAllocator->m_NumBlocksInPage);
                    return nullptr;
                }

                // Initialize the next block
                if (m_NumInitializedBlocks < m_pOwnerAllocator->m_NumBlocksInPage)
                {
                    // Link next uninitialized block to the end of the list:

                    //
                    //                            ___________                      ___________
                    //                           |           |                    |           |
                    //                           | 0xcdcdcd  |                 -->| 0xcdcdcd  |   m_NumInitializedBlocks
                    //                           |-----------|                |   |-----------|
                    //                           |           |                |   |           |
                    //  m_NumInitializedBlocks   | 0xcdcdcd  |      ==>        ---|           |
                    //                           |-----------|                    |-----------|
                    //
                    //                           ~           ~                    ~           ~
                    //                           |           |                    |           |
                    //                       0   |           |                    |           |
                    //                            -----------                      -----------
                    //
                    auto* pUninitializedBlock = GetBlockStartAddress(m_NumInitializedBlocks);
                    dbgFillPattern(pUninitializedBlock, InitializedBlockMemPattern, m_pOwnerAllocator->m_BlockSize);
                    void** ppNextBlock = reinterpret_cast<void**>(pUninitializedBlock);
                    ++m_NumInitializedBlocks;
                    if (m_NumInitializedBlocks < m_pOwnerAllocator->m_NumBlocksInPage)
                        *ppNextBlock = GetBlockStartAddress(m_NumInitializedBlocks);
                    else
                        *ppNextBlock = nullptr;
                }

                void* res = m_pNextFreeBlock;
                dbgVerifyAddress(res);
                // Move pointer to the next free block
                m_pNextFreeBlock = *reinterpret_cast<void**>(m_pNextFreeBlock);
                --m_NumFreeBlocks;
                if (m_NumFreeBlocks != 0)
                    dbgVerifyAddress(m_pNextFreeBlock);
                else
                    QGFX_VERIFY_EXPR(m_pNextFreeBlock == nullptr);

                dbgFillPattern(res, AllocatedBlockMemPattern, m_pOwnerAllocator->m_BlockSize);
                return res;
            }

            void Deallocate(void* p)
            {
                QGFX_VERIFY_EXPR(m_pOwnerAllocator != nullptr);

                dbgVerifyAddress(p);
                dbgFillPattern(p, DeallocatedBlockMemPattern, m_pOwnerAllocator->m_BlockSize);
                // Add block to the beginning of the linked list
                *reinterpret_cast<void**>(p) = m_pNextFreeBlock;
                m_pNextFreeBlock = p;
                ++m_NumFreeBlocks;
            }

            bool HasSpace() const { return m_NumFreeBlocks > 0; }
            bool HasAllocations() const { return m_NumFreeBlocks < m_NumInitializedBlocks; }

        private:
            MemoryPage(const MemoryPage&) = delete;
            MemoryPage& operator=(const MemoryPage) = delete;
            MemoryPage& operator=(MemoryPage&&) = delete;

            uint32_t                     m_NumFreeBlocks = 0;       // Num of remaining blocks
            uint32_t                     m_NumInitializedBlocks = 0;       // Num of initialized blocks
            void* m_pPageStart = nullptr; // Beginning of memory pool
            void* m_pNextFreeBlock = nullptr; // Num of next free block
            FixedBlockMemoryAllocator* m_pOwnerAllocator = nullptr;
        };

        std::vector<MemoryPage, STDAllocatorRawMem<MemoryPage>>                                          m_PagePool;
        std::unordered_set<size_t, std::hash<size_t>, std::equal_to<size_t>, STDAllocatorRawMem<size_t>> m_AvailablePages;

        using AddrToPageIdMapElem = std::pair<void* const, size_t>;
        std::unordered_map<void*, size_t, std::hash<void*>, std::equal_to<void*>, STDAllocatorRawMem<AddrToPageIdMapElem>> m_AddrToPageId;

        std::mutex m_Mutex;

        IMemoryAllocator& m_RawMemoryAllocator;
        const size_t      m_BlockSize;
        const uint32_t    m_NumBlocksInPage;
    };


}