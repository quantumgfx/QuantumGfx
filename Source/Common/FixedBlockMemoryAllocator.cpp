#include "Qgfx/Common/FixedBlockMemoryAllocator.hpp"
#include "Qgfx/Common/Align.hpp"

#include <algorithm>

namespace Qgfx
{
    static size_t AdjustBlockSize(size_t BlockSize)
    {
        return AlignUp(std::max(BlockSize, size_t{ 1 }), sizeof(void*));
    }

    FixedBlockMemoryAllocator::FixedBlockMemoryAllocator(IMemoryAllocator& RawMemoryAllocator,
        size_t            BlockSize,
        uint32_t            NumBlocksInPage) :
        // clang-format off
        m_PagePool(STDAllocatorRawMem<MemoryPage>(RawMemoryAllocator)),
        m_AvailablePages(STDAllocatorRawMem<size_t>(RawMemoryAllocator)),
        m_AddrToPageId(STDAllocatorRawMem<AddrToPageIdMapElem>(RawMemoryAllocator)),
        m_RawMemoryAllocator{ RawMemoryAllocator },
        m_BlockSize{ AdjustBlockSize(BlockSize) },
        m_NumBlocksInPage{ NumBlocksInPage }
        // clang-format on
    {
        // Allocate one page
        CreateNewPage();
    }

    FixedBlockMemoryAllocator::~FixedBlockMemoryAllocator()
    {
#ifdef QGFX_DEBUG
        for (size_t p = 0; p < m_PagePool.size(); ++p)
        {
            QGFX_VERIFY(!m_PagePool[p].HasAllocations(), "Memory leak detected: memory page has allocated block");
            QGFX_VERIFY(m_AvailablePages.find(p) != m_AvailablePages.end(), "Memory page is not in the available page pool");
        }
#endif
    }

    void FixedBlockMemoryAllocator::CreateNewPage()
    {
        m_PagePool.emplace_back(*this);
        m_AvailablePages.insert(m_PagePool.size() - 1);
        m_AddrToPageId.reserve(m_PagePool.size() * m_NumBlocksInPage);
    }

    void* FixedBlockMemoryAllocator::Allocate(size_t Size)
    {
        QGFX_VERIFY_EXPR(Size > 0);

        Size = AdjustBlockSize(Size);
        QGFX_VERIFY(m_BlockSize == Size, "Requested size (", Size, ") does not match the block size (", m_BlockSize, ")");

        std::lock_guard<std::mutex> LockGuard(m_Mutex);

        if (m_AvailablePages.empty())
        {
            CreateNewPage();
        }

        auto  PageId = *m_AvailablePages.begin();
        auto& Page = m_PagePool[PageId];
        auto* Ptr = Page.Allocate();
        m_AddrToPageId.insert(std::make_pair(Ptr, PageId));
        if (!Page.HasSpace())
        {
            m_AvailablePages.erase(m_AvailablePages.begin());
        }

        return Ptr;
    }

    void FixedBlockMemoryAllocator::Free(void* Ptr)
    {
        std::lock_guard<std::mutex> LockGuard(m_Mutex);
        auto PageIdIt = m_AddrToPageId.find(Ptr);
        if (PageIdIt != m_AddrToPageId.end())
        {
            auto PageId = PageIdIt->second;
            QGFX_VERIFY_EXPR(PageId >= 0 && PageId < m_PagePool.size());
            m_PagePool[PageId].Deallocate(Ptr);
            m_AvailablePages.insert(PageId);
            m_AddrToPageId.erase(PageIdIt);
            if (m_AvailablePages.size() > 1 && !m_PagePool[PageId].HasAllocations())
            {
                // In current implementation pages are never released!
                // Note that if we delete a page, all indices past it will be invalid

                //m_PagePool.erase(m_PagePool.begin() + PageId);
                //m_AvailablePages.erase(PageId);
            }
        }
        else
        {
            QGFX_UNEXPECTED("Address not found in the allocations list - double freeing memory?");
        }
    }
}