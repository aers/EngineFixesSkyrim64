#pragma once

namespace Fixes::MusicOverlap
{
    namespace detail
    {
        inline void DoFinish(RE::BSIMusicType* a_self, bool a_immediate)
        {
            a_self->tracks[a_self->currentTrackIndex]->DoFinish(a_immediate, std::max(a_self->fadeTime, 4.0f));
            a_self->typeStatus = static_cast<RE::BSIMusicType::MUSIC_STATUS>(a_self->tracks[a_self->currentTrackIndex]->GetMusicStatus());
        }
    }

    inline void Install()
    {
        // BSIMusicType interface, second vtbl
        REL::Relocation vtbl{ RE::BGSMusicType::VTABLE[1] };
        vtbl.write_vfunc(0x3, detail::DoFinish);

        logger::info("installed music overlap fix"sv);
    }
}