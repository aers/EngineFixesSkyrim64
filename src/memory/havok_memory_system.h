#pragma once

namespace Memory::HavokMemorySystem
{
    namespace detail
    {
        class hkMemoryAllocator final :
            public RE::hkMemoryAllocator
        {
        public:
            void* BlockAlloc(std::int32_t a_numBytesIn) override
            {
                return a_numBytesIn > 0 ?
                           Allocator::GetAllocator()->AllocateAligned(a_numBytesIn, 0x10) :
                           nullptr;
            }

            void BlockFree(void* a_ptr, std::int32_t) override
            {
                Allocator::GetAllocator()->DeallocateAligned(a_ptr);
            }

            void* BufAlloc(std::int32_t& a_reqNumBytesInOut) override
            {
                return a_reqNumBytesInOut > 0 ?
                           Allocator::GetAllocator()->AllocateAligned(a_reqNumBytesInOut, 0x10) :
                           nullptr;
            }

            void BufFree(void* a_ptr, std::int32_t) override
            {
                Allocator::GetAllocator()->DeallocateAligned(a_ptr);
            }

            void* BufRealloc(void* a_old, std::int32_t, std::int32_t& a_reqNumBytesInOut) override
            {
                return Allocator::GetAllocator()->ReallocateAligned(a_old, a_reqNumBytesInOut, 0x10);
            }

            void BlockAllocBatch(void** a_ptrsOut, std::int32_t a_numPtrs, std::int32_t a_blockSize) override
            {
                std::span range{ a_ptrsOut, static_cast<std::size_t>(a_numPtrs) };
                std::ranges::for_each(range,
                    [&](void*& a_elem) {
                        a_elem =
                            a_blockSize > 0 ?
                                Allocator::GetAllocator()->AllocateAligned(a_blockSize, 0x10) :
                                nullptr;
                    });
            }

            void BlockFreeBatch(void** a_ptrsIn, std::int32_t a_numPtrs, std::int32_t) override
            {
                std::span range{ a_ptrsIn, static_cast<std::size_t>(a_numPtrs) };
                std::ranges::for_each(range,
                    [&](void* a_elem) {
                        Allocator::GetAllocator()->DeallocateAligned(a_elem);
                    });
            }

            void         GetMemoryStatistics([[maybe_unused]] MemoryStatistics& a_usage ) override {}
            std::int32_t GetAllocatedSize([[maybe_unused]] const void* a_obj, std::int32_t a_numBytes) override { return a_numBytes; };
            void         ResetPeakMemoryStatistics() override {}
        };

        class hkMemorySystem final :
            public RE::hkMemorySystem
        {
        public:
            [[nodiscard]] static hkMemorySystem* GetSingleton()
            {
                static hkMemorySystem singleton;
                return std::addressof(singleton);
            }

            RE::hkMemoryRouter* MainInit(const FrameInfo&, Flags a_flags) override
            {
                ThreadInit(_router, "main", a_flags);
                return std::addressof(_router);
            }

            RE::hkResult MainQuit(Flags a_flags) override
            {
                ThreadQuit(_router, a_flags);
                return RE::hkResult::kSuccess;
            }

            void ThreadInit(RE::hkMemoryRouter& a_router, [[maybe_unused]] const char* a_name, Flags a_flags) override
            {
                const auto allocator = std::addressof(_allocator);

                if (a_flags & FlagBits::kPersistent) {
                    a_router.SetHeap(allocator);
                    a_router.SetDebug(allocator);
                    a_router.SetTemp(nullptr);
                    a_router.SetSolver(nullptr);
                }

                if (a_flags & FlagBits::kTemporary) {
                    a_router.GetStack().Init(allocator, allocator, allocator);
                    a_router.SetTemp(allocator);
                    a_router.SetSolver(allocator);
                }
            }

            void ThreadQuit(RE::hkMemoryRouter& a_router, Flags a_flags) override
            {
                if (a_flags & FlagBits::kTemporary) {
                    a_router.GetStack().Quit();
                }

                if (a_flags & FlagBits::kPersistent) {
                    std::memset(
                        std::addressof(a_router),
                        0,
                        sizeof(a_router));
                }

                GarbageCollectThread(a_router);
            }

            RE::hkMemoryAllocator* GetUncachedLockedHeapAllocator() override { return std::addressof(_allocator); }
            void                   PrintStatistics([[maybe_unused]] RE::hkOstream& a_ostream) override {}
            void                   GetHeapStatistics([[maybe_unused]] RE::hkMemoryAllocator::MemoryStatistics&  a_stats) override {}
            RE::hkResult           WalkMemory([[maybe_unused]] MemoryWalkCallback a_callback, [[maybe_unused]] void* a_param) override { return RE::hkResult::kFailure; }

        private:
            hkMemorySystem() = default;
            hkMemorySystem(const hkMemorySystem&) = delete;
            hkMemorySystem(hkMemorySystem&&) = delete;

            ~hkMemorySystem() override = default;

            hkMemorySystem& operator=(const hkMemorySystem&) = delete;
            hkMemorySystem& operator=(hkMemorySystem&&) = delete;

            // padding forces 10 alignment
            std::uint64_t pad8;
            hkMemoryAllocator _allocator;
            std::uint64_t pad18;
            RE::hkMemoryRouter _router;
        };

        inline void Install()
        {
            REL::Relocation target { RELOCATION_ID(76023, 77856), VAR_NUM(0x6C, 0x61) };
            target.write_call<5>(hkMemorySystem::GetSingleton);
        }
    }

    inline void Install()
    {
        detail::Install();
        logger::info("installed havok memory system patch"sv);
    }
}