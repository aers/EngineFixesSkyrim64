#pragma once

namespace Patches::OverrideScrapHeap
{
    namespace detail
    {
        namespace AutoScrapBuffer
        {
            struct Patch final :
                Xbyak::CodeGenerator
            {
                Patch()
                {
                    xor_(rax, rax);
                    cmp(rbx, rax);
                }
            };

            inline void Ctor()
            {
                REL::Relocation target{ REL::ID(68108), 0x1D };
                constexpr std::size_t size = 0x32 - 0x1D;
                target.write_fill(REL::NOP, size);
            }

            inline void Dtor()
            {
                REL::Relocation baseStart { REL::ID(68109), 0x12 };
                REL::Relocation baseEnd { REL::ID(68109), 0x2F };

                {
                    constexpr std::size_t size = 0x2F - 0x12;
                    baseStart.write_fill(REL::NOP, size);

                    Patch p;
                    p.ready();
                    assert(p.getSize() <= size);
                    baseStart.write(std::span{ p.getCode<const std::byte*>(), p.getSize() });
                }

                {
                    baseEnd.write(std::uint8_t{ 0x74 });  // jnz -> jz
                }
            }

            inline void Install()
            {
                Ctor();
                Dtor();
            }
        }

        namespace ScrapHeap
        {
            inline void* Allocate(RE::ScrapHeap*, std::size_t a_size, std::size_t a_alignment)
            {
                return RE::aligned_alloc(a_alignment, a_size);
            }

            inline RE::ScrapHeap* Ctor(RE::ScrapHeap* a_self)
            {
                std::memset(a_self, 0, sizeof(RE::ScrapHeap));
                REX::EMPLACE_VTABLE(a_self);
                return a_self;
            }

            inline void Deallocate(RE::ScrapHeap*, void* a_mem)
            {
                RE::aligned_free(a_mem);
            }

            inline void WriteStubs()
            {
                using tuple_t = std::tuple<REL::ID, std::size_t>;
                constexpr std::array todo{
                    tuple_t{ REL::ID(68152), 0xBA },   // Clean
                    tuple_t{ REL::ID(68151), 0x8 },    // ClearKeepPages
                    tuple_t{ REL::ID(68155), 0xF6 },   // InsertFreeBlock
                    tuple_t{ REL::ID(68156), 0x185 },  // RemoveFreeBlock
                    tuple_t{ REL::ID(68150), 0x4 },    // SetKeepPages
                    tuple_t{ REL::ID(68143), 0x32 },   // dtor
                };

                for (const auto& [offset, size] : todo)
                {
                    REL::Relocation target{ offset };
                    target.write_fill(REL::INT3, size);
                    target.write(REL::RET);
                }
            }

            inline void WriteHooks()
            {
                using tuple_t = std::tuple<REL::ID, std::size_t, void*>;
                constexpr std::array todo{
                    tuple_t{ REL::ID(68144), 0x5E7, &Allocate },
                    tuple_t{ REL::ID(68146), 0x13E, &Deallocate },
                    tuple_t{ REL::ID(68142), 0x13A, &Ctor },
                };

                for (const auto& [offset, size, func] : todo)
                {
                    REL::Relocation target{ offset };
                    target.replace_func(size, func);
                }
            }
        }

        inline void Install()
        {
            AutoScrapBuffer::Install();
            ScrapHeap::WriteStubs();
            ScrapHeap::WriteHooks();
        }
    }

    inline void Install()
    {
        detail::Install();

        REX::INFO("installed override scrapheap patch"sv);
    }
}