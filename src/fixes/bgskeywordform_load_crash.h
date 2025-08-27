#pragma once

namespace Fixes::BGSKeywordFormLoadCrash
{
    namespace detail
    {
        inline REL::Relocation<bool(RE::TESFile*, std::uint32_t)> TESFile_SetOffsetChunk { REL::ID(13985) };

        inline SafetyHookInline orig_BGSKeywordForm_Load;

        inline void BGSKeywordForm_Load(RE::BGSKeywordForm* a_self, RE::TESFile* a_file)
        {
            // no file(?)
            if (a_file == nullptr)
                return;

            std::uint32_t currentChunkOffset = a_file->formoffset + a_file->fileOffset + 24;

            std::uint32_t numKeywords = 0;

            a_file->ReadData(&numKeywords, sizeof(numKeywords));

            // if keywords > 0, run original routine
            if (numKeywords > 0) {
                TESFile_SetOffsetChunk(a_file, currentChunkOffset);
                orig_BGSKeywordForm_Load.call(a_self, a_file);
                return;
            }

            // if keywords = 0 and file ends, return
            if (!a_file->SeekNextSubrecord()) {
                logger::warn("fixing invalid keyword form detected at formID {:X} in file {}"sv, a_file->currentform.formID, a_file->fileName);
                return;
            }

            // if this new record is not KWDA, return to the previous subrecord before returning to normal execution
            auto currentSubrecordType = a_file->GetCurrentSubRecordType();

            if (currentSubrecordType != 0x4144574B) // KWDA
            {
                logger::warn("fixing invalid keyword form detected at formID {:X} in file {}"sv, a_file->currentform.formID, a_file->fileName);
                TESFile_SetOffsetChunk(a_file, currentChunkOffset);
            }

            // if the next subrecord was KWDA it's ok to just return and skip it
        }

        inline void Install()
        {
            orig_BGSKeywordForm_Load = safetyhook::create_inline(REL::ID(14255).address(), BGSKeywordForm_Load);
        }
    }

    inline void Install()
    {
        detail::Install();
        logger::info("installed bgskeywordform load crash fix"sv);
    }
}