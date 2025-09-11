#pragma once

namespace Memory::MemoryManager
{
    namespace detail
    {
        // when the global memory manager has a zero-size allocation it returns scratch memory
        // replicate this behavior
        inline std::byte* g_ZeroAddress{ nullptr };

        // MemoryManager::AutoScrapBuffer impl
        // ScrapHeap RAII wrapper
        struct AutoScrapBuffer
        {
            static void Ctor(AutoScrapBuffer* a_self, const std::size_t a_size, const std::size_t a_alignment)
            {
                if (a_size == 0) {
                    a_self->p_Memory = g_ZeroAddress;
                } else {
                    a_self->p_Memory = Allocator::GetAllocator()->AllocateAligned(a_size, a_alignment);
                }
            }

            static void Dtor(AutoScrapBuffer* a_self)
            {
                if (a_self->p_Memory != g_ZeroAddress) {
                    Allocator::GetAllocator()->DeallocateAligned(a_self->p_Memory);
                }
                a_self->p_Memory = nullptr;
            }

            static void Install()
            {
                REL::Relocation ctor{ RELOCATION_ID(66853, 68108) };
                REL::Relocation dtor{ RELOCATION_ID(66854, 68109) };

                ctor.replace_func(0x7B, Ctor);
                dtor.replace_func(VAR_NUM(0x58, 0x54), Dtor);
            }

            void* p_Memory;
        };

        inline void* Allocate(RE::MemoryManager*, const std::size_t a_size, const std::uint32_t a_alignment, const bool a_alignmentRequired)
        {
            if (a_size > 0) {
                return a_alignmentRequired ? Allocator::GetAllocator()->AllocateAligned(a_size, a_alignment) : Allocator::GetAllocator()->Allocate(a_size);
            }

            return g_ZeroAddress;
        }

        inline void* Reallocate(RE::MemoryManager* a_self, void* a_oldMem, const std::size_t a_newSize, const std::uint32_t a_alignment, const bool a_alignmentRequired)
        {
            if (a_oldMem == g_ZeroAddress)
                return Allocate(a_self, a_newSize, a_alignment, a_alignmentRequired);

            return a_alignmentRequired ? Allocator::GetAllocator()->ReallocateAligned(a_oldMem, a_newSize, a_alignment) : Allocator::GetAllocator()->Reallocate(a_oldMem, a_newSize);
        }

        inline void Deallocate(RE::MemoryManager*, void* a_mem, const bool a_alignmentRequired)
        {
            if (a_mem != g_ZeroAddress) {
                if (a_alignmentRequired)
                    Allocator::GetAllocator()->DeallocateAligned(a_mem);
                else
                    Allocator::GetAllocator()->Deallocate(a_mem);
            }
        }

        inline std::size_t Size(RE::MemoryManager*, void* a_mem)
        {
            if (a_mem == g_ZeroAddress)
                return 0;

            return Allocator::GetAllocator()->Size(a_mem);
        }

        inline void ReplaceAllocRoutines()
        {
            REL::Relocation allocate{ RELOCATION_ID(66859, 68115) };
            REL::Relocation reallocate{ RELOCATION_ID(66860, 68116) };
            REL::Relocation deallocate{ RELOCATION_ID(66861, 68117) };
            REL::Relocation size{ RELOCATION_ID(66849, 68100) };

            allocate.replace_func(0x248, Allocate);
            reallocate.replace_func(VAR_NUM(0xA7, 0x1F6), Reallocate);
            deallocate.replace_func(0x114, Deallocate);
            size.replace_func(VAR_NUM(0x12A, 0x156), Size);
        }

        inline void StubInit()
        {
            REL::Relocation target{ RELOCATION_ID(66862, 68121) };

            target.write_fill(REL::INT3, VAR_NUM(0x9F, 0x1A7));
            target.write(REL::RET);

            REL::Relocation<std::uint32_t*> initFence{ RELOCATION_ID(514112, 400190) };
            *initFence = 2;
        }

        inline void Install()
        {
            g_ZeroAddress = new std::byte[1u << 10]{ static_cast<std::byte>(0) };

            StubInit();
            ReplaceAllocRoutines();
            RE::MemoryManager::GetSingleton()->RegisterMemoryManager();
            RE::BSThreadEvent::InitSDM();
        }
    }

    inline void Install()
    {
        detail::Install();
        detail::AutoScrapBuffer::Install();
        logger::info("installed global memory manager patch"sv);
    }
}