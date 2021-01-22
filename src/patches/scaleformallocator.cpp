#pragma once

namespace
{
    class CustomAllocator :
        public RE::GSysAllocPaged
    {
    public:
        [[nodiscard]] static CustomAllocator* GetSingleton()
        {
            static CustomAllocator singleton;
            return std::addressof(singleton);
        }

        static void Install()
        {
            REL::Relocation<std::uintptr_t> target{ REL::ID(80300), 0xED };
            auto& trampoline = SKSE::GetTrampoline();
            _init = trampoline.write_call<5>(target.address(), Init);
        }

    protected:
        void GetInfo(Info* a_info) const override
        {
            assert(a_info != nullptr);

            a_info->minAlign = __STDCPP_DEFAULT_NEW_ALIGNMENT__;
            a_info->maxAlign = 0;
            a_info->granularity = 1u << 16;
            a_info->sysDirectThreshold = 1;
            a_info->maxHeapGranularity = 0;
            a_info->hasRealloc = false;
        }

        void* Alloc(std::size_t a_size, std::size_t a_align) override
        {
            return a_size > 0 ?
                       scalable_aligned_malloc(a_size, a_align) :
                       nullptr;
        }

        bool Free(void* a_ptr, std::size_t, std::size_t) override
        {
            if (a_ptr)
                scalable_aligned_free(a_ptr);
            return true;
        }

        void* AllocSysDirect(
            std::size_t a_size,
            std::size_t a_alignment,
            std::size_t* a_actualSize,
            std::size_t* a_actualAlign) override
        {
            assert(a_actualSize != nullptr);
            assert(a_actualAlign != nullptr);

            *a_actualSize = a_size;
            *a_actualAlign = a_alignment;
            return a_size > 0 ?
                       scalable_aligned_malloc(a_size, a_alignment) :
                       nullptr;
        }

        bool FreeSysDirect(void* a_ptr, std::size_t, std::size_t) override
        {
            if (a_ptr)
                scalable_aligned_free(a_ptr);
            return true;
        }

    private:
        CustomAllocator() = default;
        CustomAllocator(const CustomAllocator&) = delete;
        CustomAllocator(CustomAllocator&&) = delete;
        ~CustomAllocator() = default;
        CustomAllocator& operator=(const CustomAllocator&) = delete;
        CustomAllocator& operator=(CustomAllocator&&) = delete;

        static void Init(const RE::GMemoryHeap::HeapDesc& a_rootHeapDesc, RE::GSysAllocBase*)
        {
            return _init(a_rootHeapDesc, GetSingleton());
        }

        static inline REL::Relocation<decltype(Init)> _init;
    };
}

namespace patches
{
    bool PatchScaleformAllocator()
    {
        logger::trace("- scalform allocator patch -"sv);

        CustomAllocator::Install();

        logger::trace("success"sv);
        return true;
    }
}
