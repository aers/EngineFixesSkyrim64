#pragma once

// Loading a save game crashes when a Papyrus string longer than 4095 bytes was saved.
//
// The Papyrus part of a save game starts with a string table. The loader (BSScript::IVMSaveLoadInterface::LoadGame, "Func2") reads every entry as a 2 byte length and then the
// characters into a "page" of a pool: 0x1000 bytes of data, followed by a write pointer (+0x1000), the end of the data (+0x1008) and the link to the next page (+0x1010). If the
// string does not fit into the current page, the loader asks for a new page, but a new page has also only 0x1000 bytes, so a string of 4096 bytes or more is read behind the end of
// the page. It overwrites the header of the page and the next heap block, and the terminating 0 is written through the overwritten write pointer: access violation at
// "mov byte ptr [rbx+rax], 0" (rbx = the length of the string, rax = the 8 bytes of the string at offset 4096). The saving side (see the notes of the writer below) has no limit, so
// the game writes a save game that it cannot load: any mod that keeps a long text in a script variable can cause it, and the crash comes with the next load, not with the save.
//
// The fix: when the string does not fit into a standard page, the page that the loader gets is a page that is big enough. It is built like the standard one (header at +0x1000,
// linked in the same way, freed by the pool like any page, the heap frees a block without its size), only the data is behind the header. A string can have 65535 bytes at most (the
// length is a 2 byte value), so a page is 64 KiB at most.
//
// The writer (the function that saves the table, 104679) has the same kind of problem for strings longer than 65535 bytes: it writes the lower 16 bits of the length as the length
// and then all bytes (both ways of writing it, the buffered one and the direct one), so everything behind such a string is read at the wrong place and the save game is damaged.
// The fix cuts a string that is longer than 65535 bytes to 65535 bytes when it is saved, and writes a line to the log. A string of that size is not meant to be there (Papyrus
// strings are far shorter in practice), but a damaged save game is worse than a cut string.

namespace Fixes::SaveStringTableOverflow
{
    namespace detail
    {
        constexpr std::size_t kPageDataSize = 0x1000;  // the data of a standard page
        constexpr std::size_t kPageHeaderSize = 0x18;  // write pointer, end, link
        constexpr std::size_t kPageSize = kPageDataSize + kPageHeaderSize;
        constexpr std::size_t kPageCursor = 0x1000;  // where the next string is written
        constexpr std::size_t kPageEnd = 0x1008;     // the end of the data
        constexpr std::size_t kPageNext = 0x1010;    // the link to the next page

        // the pool of the pages (the same container as the writable string table of BSScript has, with pages of 0x1018 bytes)
        struct Pool
        {
            RE::ScrapHeap* heap;      // 00
            void*          sentinel;  // 08  the link in front of the first page
            void**         tail;      // 10  the link that the next page is written to
            void*          current;   // 18  the page that is read into
            void*          free;      // 20  pages that can be used again
            void*          claimed;   // 28  the link behind the current page once it is in use
            void*          first;     // 30
            std::uint32_t  count;     // 38
            std::uint32_t  pad;       // 3C
        };
        static_assert(sizeof(Pool) == 0x40);

        template <class T>
        inline T& At(void* a_base, std::size_t a_offset)
        {
            return *reinterpret_cast<T*>(static_cast<std::byte*>(a_base) + a_offset);
        }

        inline std::uintptr_t g_originalNewPage = 0;  // the function that gives a standard page

        using NewPage_t = void* (*)(Pool*);

        // Builds a page for a string that needs a_bytes bytes (with the terminating 0) in the same way as the standard function appends one.
        inline void* NewBigPage(Pool* a_pool, std::size_t a_bytes)
        {
            void* page = a_pool->heap->Allocate(kPageSize + a_bytes, 8);
            if (!page) {
                return nullptr;
            }

            auto* data = static_cast<std::byte*>(page) + kPageSize;
            At<void*>(page, kPageNext) = nullptr;
            *a_pool->tail = page;
            a_pool->tail = &At<void*>(page, kPageNext);
            if (!a_pool->first) {
                a_pool->first = page;
            }
            a_pool->current = page;
            ++a_pool->count;
            a_pool->claimed = &At<void*>(page, kPageNext);
            At<void*>(page, kPageCursor) = data;
            At<void*>(page, kPageEnd) = data + a_bytes;
            return page;
        }

        // The loader calls this where it wants a new page: a_length is the length of the string that does not fit into the current page.
        inline void* NewPage(Pool* a_pool, std::uint32_t a_length)
        {
            const std::size_t bytes = static_cast<std::size_t>(a_length) + 1;
            if (bytes <= kPageDataSize) {
                return reinterpret_cast<NewPage_t>(g_originalNewPage)(a_pool);
            }

            // The standard function reuses a page that is linked already when the pool is in another state than "the current page is in use". That happens after a reset of the
            // pool, which does not take place while a save game is loaded: leave this case to the game.
            if (a_pool->current && a_pool->claimed != &At<void*>(a_pool->current, kPageNext)) {
                REX::WARN("save string table: a string of {} bytes found while the pool is not in use, left to the game"sv, a_length);
                return reinterpret_cast<NewPage_t>(g_originalNewPage)(a_pool);
            }

            if (void* page = NewBigPage(a_pool, bytes)) {
                REX::INFO("save string table: a string of {} bytes was read into a page of its own"sv, a_length);
                return page;
            }

            REX::ERROR("save string table: no memory for a string of {} bytes"sv, a_length);
            return reinterpret_cast<NewPage_t>(g_originalNewPage)(a_pool);
        }

        // The call that asks for a new page has the length of the string in ebx (the pool is in rcx already): pass it as the second argument.
        struct Patch final : Xbyak::CodeGenerator
        {
            explicit Patch(std::uintptr_t a_hook)
            {
                mov(edx, ebx);
                mov(rax, a_hook);
                jmp(rax);
            }
        };

        // the code of the loader that is replaced, from +0x296 to +0x2B6 (movzx ecx, bx ... call <new page> ... mov rbp, rax)
        constexpr std::array<std::uint8_t, 0x1F> kExpected{ 0x0F, 0xB7, 0xCB, 0x48, 0xFF, 0xC1, 0x48, 0x03, 0x8D, 0x00, 0x10, 0x00, 0x00, 0x48, 0x3B, 0x8D,
                                                            0x08, 0x10, 0x00, 0x00, 0x76, 0x0B, 0x48, 0x8B, 0xCE, 0xE8, 0x00, 0x00, 0x00, 0x00, 0x48 };
        constexpr std::size_t kSite = 0x296;
        constexpr std::size_t kCall = 0x2AF;  // the call of the function that gives a new page

        // ---- the writer: strings longer than 65535 bytes are cut to 65535 bytes

        constexpr std::uint64_t kMaxLength = 0xFFFF;  // the length of a string in the save game is a 2 byte value

        inline std::atomic<int> g_cutCount{ 0 };

        // called from the code of the writer when a string is too long (the registers are saved around the call)
        inline void OnStringTooLong(std::uint64_t a_length)
        {
            if (g_cutCount.fetch_add(1) < 8) {  // the first ones only: a save game with many of them would fill the log
                REX::WARN("save string table: a string of {} bytes is too long for a save game (65535 at most), it is cut to 65535 bytes"sv, a_length);
            }
        }

        // Saves the registers that the code of the game may still need, calls a_function with the length (rbx) and restores them (the stack is aligned inside the writer)
        inline void CallOnStringTooLong(Xbyak::CodeGenerator& a_code, std::uintptr_t a_function)
        {
            using namespace Xbyak::util;
            a_code.sub(rsp, 0x70);
            a_code.mov(ptr[rsp + 0x20], rax);
            a_code.mov(ptr[rsp + 0x28], rcx);
            a_code.mov(ptr[rsp + 0x30], rdx);
            a_code.mov(ptr[rsp + 0x38], r8);
            a_code.mov(ptr[rsp + 0x40], r9);
            a_code.mov(ptr[rsp + 0x48], r10);
            a_code.mov(ptr[rsp + 0x50], r11);
            a_code.mov(rcx, rbx);
            a_code.mov(rax, a_function);
            a_code.call(rax);
            a_code.mov(rax, ptr[rsp + 0x20]);
            a_code.mov(rcx, ptr[rsp + 0x28]);
            a_code.mov(rdx, ptr[rsp + 0x30]);
            a_code.mov(r8, ptr[rsp + 0x38]);
            a_code.mov(r9, ptr[rsp + 0x40]);
            a_code.mov(r10, ptr[rsp + 0x48]);
            a_code.mov(r11, ptr[rsp + 0x50]);
            a_code.add(rsp, 0x70);
        }

        // site 1: right behind the first count of the characters (rbx = the length). It keeps the length at 65535 for the room that is asked for and for the direct way of writing,
        // and reports the string. The instruction that was there (cmp byte ptr [r14 + 0x18], 0) is done here, the "je" behind it follows.
        struct WriterLengthPatch final : Xbyak::CodeGenerator
        {
            WriterLengthPatch(std::uintptr_t a_back, std::uintptr_t a_report)
            {
                using namespace Xbyak::util;
                Xbyak::Label ok;
                cmp(rbx, static_cast<std::uint32_t>(kMaxLength));
                jbe(ok);
                CallOnStringTooLong(*this, a_report);
                mov(ebx, static_cast<std::uint32_t>(kMaxLength));
                L(ok);
                cmp(byte[r14 + 0x18], 0);
                mov(rax, a_back);
                jmp(rax);
            }
        };

        // site 2: right behind the second count (rcx = the length) that the buffered way of writing uses for the length and for the copy
        struct WriterCopyPatch final : Xbyak::CodeGenerator
        {
            explicit WriterCopyPatch(std::uintptr_t a_back)
            {
                using namespace Xbyak::util;
                Xbyak::Label ok;
                cmp(rcx, static_cast<std::uint32_t>(kMaxLength));
                jbe(ok);
                mov(ecx, static_cast<std::uint32_t>(kMaxLength));
                L(ok);
                mov(rax, ptr[r15 + 0x10]);  // the two instructions that were there
                mov(ptr[rax], cx);
                mov(rdx, a_back);
                jmp(rdx);
            }
        };

        // the code of the writer that is changed, as it is in 1.6.1170 and 1.7.104
        constexpr std::size_t kWriterFirstCount = 0xD1;    // mov rbx, -1 ... (first count, rbx)
        constexpr std::size_t kWriterSite1 = 0xED;         // cmp byte ptr [r14 + 0x18], 0 ; je
        constexpr std::size_t kWriterAfterSite1 = 0xF2;    // the je
        constexpr std::size_t kWriterSecondCount = 0x118;  // mov rcx, -1 ... (second count, rcx)
        constexpr std::size_t kWriterSite2 = 0x12D;        // mov rax, [r15 + 0x10] ; mov [rax], cx
        constexpr std::size_t kWriterAfterSite2 = 0x134;   // mov rax, [r15 + 0x10]
        constexpr std::array<std::uint8_t, 7> kExpectedFirstCount{ 0x48, 0xC7, 0xC3, 0xFF, 0xFF, 0xFF, 0xFF };
        constexpr std::array<std::uint8_t, 7> kExpectedSecondCount{ 0x48, 0xC7, 0xC1, 0xFF, 0xFF, 0xFF, 0xFF };
        constexpr std::array<std::uint8_t, 7> kExpectedSite1{ 0x41, 0x80, 0x7E, 0x18, 0x00, 0x74, 0x70 };
        constexpr std::array<std::uint8_t, 11> kExpectedSite2{ 0x49, 0x8B, 0x47, 0x10, 0x66, 0x89, 0x08, 0x49, 0x8B, 0x47, 0x10 };

        template <std::size_t N>
        inline bool Matches(std::uintptr_t a_address, const std::array<std::uint8_t, N>& a_expected)
        {
            return std::memcmp(reinterpret_cast<const void*>(a_address), a_expected.data(), N) == 0;
        }
    }

#ifdef SKYRIM_AE
    inline void InstallWriter()
    {
        const REL::Relocation<std::uintptr_t> writer{ REL::ID(104679) };  // writes the string table of a save game
        const auto base = writer.address();

        if (!detail::Matches(base + detail::kWriterFirstCount, detail::kExpectedFirstCount) ||
            !detail::Matches(base + detail::kWriterSecondCount, detail::kExpectedSecondCount) ||
            !detail::Matches(base + detail::kWriterSite1, detail::kExpectedSite1) || !detail::Matches(base + detail::kWriterSite2, detail::kExpectedSite2)) {
            REX::WARN("save string table overflow fix (writer) not installed: the code of the writer is not the one that was examined"sv);
            return;
        }

        auto& trampoline = REL::GetTrampoline();
        detail::WriterLengthPatch lengthPatch{ base + detail::kWriterAfterSite1, reinterpret_cast<std::uintptr_t>(&detail::OnStringTooLong) };
        trampoline.write_jmp5(base + detail::kWriterSite1, reinterpret_cast<std::uintptr_t>(trampoline.allocate(lengthPatch)));
        detail::WriterCopyPatch copyPatch{ base + detail::kWriterAfterSite2 };
        trampoline.write_jmp5(base + detail::kWriterSite2, reinterpret_cast<std::uintptr_t>(trampoline.allocate(copyPatch)));  // the two bytes behind the jump are not run

        REX::INFO("installed save string table overflow fix (writer)"sv);
    }
#endif

    inline void InstallReader()
    {
#ifdef SKYRIM_AE
        const REL::Relocation<std::uintptr_t> loader{ REL::ID(104661) };  // reads the string table of a save game
        const auto site = loader.address() + detail::kSite;

        // the code must be the one that was examined (1.6.1170 and 1.7.104): otherwise the fix is not installed
        bool same = true;
        for (std::size_t i = 0; i < detail::kExpected.size(); ++i) {
            const auto expected = detail::kExpected[i];
            if (i >= 0x1A && i < 0x1E) {  // the displacement of the call differs between versions
                continue;
            }
            if (*reinterpret_cast<const std::uint8_t*>(site + i) != expected) {
                same = false;
                break;
            }
        }
        if (!same) {
            REX::WARN("save string table overflow fix not installed: the code of the loader is not the one that was examined"sv);
            return;
        }

        const auto call = loader.address() + detail::kCall;
        const auto displacement = *reinterpret_cast<const std::int32_t*>(call + 1);
        detail::g_originalNewPage = call + 5 + displacement;

        auto& trampoline = REL::GetTrampoline();
        detail::Patch patch{ reinterpret_cast<std::uintptr_t>(&detail::NewPage) };
        trampoline.write_call5(call, reinterpret_cast<std::uintptr_t>(trampoline.allocate(patch)));

        REX::INFO("installed save string table overflow fix (reader)"sv);
#else
        REX::INFO("save string table overflow fix is not available for this runtime"sv);
#endif
    }

#ifndef SKYRIM_AE
    inline void InstallWriter() {}
#endif

    inline void Install()
    {
        InstallReader();
        InstallWriter();
    }
}
