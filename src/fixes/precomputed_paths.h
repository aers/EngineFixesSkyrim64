#pragma once

namespace PrecomputedPaths
{
    namespace detail
    {
        // hook the delete functions
        // this causes a memory leak when you shut down the game, oh no!
        inline REL::Relocation<void(RE::NavMeshInfo*)>    orig_NavMeshInfo_dtor;
        inline REL::Relocation<void(void*, std::size_t)>  orig_Free;
        inline REL::Relocation<void(RE::NavMeshInfoMap*)> orig_NavMeshInfoMap_InitItemImpl;

        inline std::unordered_set<std::uint32_t> navmeshIdsInPaths;

        inline bool HasIssues = false;

        inline void NavMeshInfo_Dtor(RE::NavMeshInfo* a_self)
        {
            if (navmeshIdsInPaths.contains(a_self->navMeshID)) {
                logger::warn("navmesh form ID {:X} is used in a precomputed path but the game considers it unneeded"sv, a_self->navMeshID);
                HasIssues = true;
                return;
            }

            orig_NavMeshInfo_dtor(a_self);
        }

        inline void Free(RE::NavMeshInfo* a_self, std::size_t a_size)
        {
            if (navmeshIdsInPaths.contains(a_self->navMeshID))
                return;

            orig_Free(a_self, a_size);
        }

        inline void NavMeshInfoMap_InitItemImpl(RE::NavMeshInfoMap* a_self)
        {
            if (!Settings::Debug::bPrintDetailedPrecomputedPathInfo.GetValue())
                for (const auto path : a_self->allPaths) {
                    for (auto nmi : *path) {
                        navmeshIdsInPaths.insert(nmi->navMeshID);
                    }
                }

            orig_NavMeshInfoMap_InitItemImpl(a_self);

            if (HasIssues) {
                logger::warn("one or more issues with precomputed paths were detected at game startup"sv);
                logger::warn("this indicates that the last plugin in your load order containing NAVI data contains a precomputed path with navmesh that had its links removed by a previous plugin"sv);
                logger::warn("it is recommended to create a patch to resolve this inconsistency in your navmesh");
            }

            if (Settings::Debug::bPrintDetailedPrecomputedPathInfo.GetValue()) {
                for (std::uint32_t i = 0; i < a_self->allPaths.size(); i++) {
                    auto* precomputedPaths = a_self->allPaths[i];
                    bool  foundProblem = false;

                    for (auto navMeshInfo : *precomputedPaths) {
                        const auto pathingCell = reinterpret_cast<RE::PathingCell*>(navMeshInfo->pathingCell.get());
                        if (*SKSE::stl::unrestricted_cast<std::uintptr_t*>(pathingCell) != RE::PathingCell::VTABLE[0].address()) {
                            foundProblem = true;
                            break;
                        }
                    }

                    if (foundProblem) {
                        logger::info("found problem with precomputed path index {}"sv, i);
                        for (auto navMeshInfo : *precomputedPaths) {
                            const auto pathingCell = reinterpret_cast<RE::PathingCell*>(navMeshInfo->pathingCell.get());
                            bool       isFreed = *SKSE::stl::unrestricted_cast<std::uintptr_t*>(pathingCell) != RE::PathingCell::VTABLE[0].address();
                            if (pathingCell->pathingCellInfo.worldSpaceID != 0) {
                                if (isFreed)
                                    logger::info("!! NAVM ID: error, pointer is freed | Worldspace ID: {:X} Cell X: {} Cell Y: {}", pathingCell->pathingCellInfo.worldSpaceID, pathingCell->pathingCellInfo.cellID.coordinates.x, pathingCell->pathingCellInfo.cellID.coordinates.y);
                                else
                                    logger::info("-- NAVM ID: {:X} | Worldspace ID: {:X} Cell X: {} Cell Y: {}", navMeshInfo->navMeshID, pathingCell->pathingCellInfo.worldSpaceID, pathingCell->pathingCellInfo.cellID.coordinates.x, pathingCell->pathingCellInfo.cellID.coordinates.y);
                            } else {
                                if (isFreed)
                                    logger::info("-- NAVM ID: error, pointer is freed | Interior Cell ID: {:X}", pathingCell->pathingCellInfo.cellID.formID);
                                else
                                    logger::info("-- NAVM ID: {:X} | Interior Cell ID: {:X}", navMeshInfo->navMeshID, pathingCell->pathingCellInfo.cellID.formID);
                            }
                        }
                    }
                }
            }
        }

        inline void Install()
        {
            REL::Relocation _callDtor{ RELOCATION_ID(29531, 30380), VAR_NUM(0x328, 0x348) };
            REL::Relocation _callFree{ RELOCATION_ID(29531, 30380), VAR_NUM(0x335, 0x355) };

            orig_NavMeshInfo_dtor = _callDtor.write_call<5>(NavMeshInfo_Dtor);
            orig_Free = _callFree.write_call<5>(Free);

            REL::Relocation _vtable{ RE::NavMeshInfoMap::VTABLE[0] };
            orig_NavMeshInfoMap_InitItemImpl = _vtable.write_vfunc(19, NavMeshInfoMap_InitItemImpl);
        }
    }

    inline void Install()
    {
        detail::Install();
        logger::info("installed precomputed paths crash fix"sv);
    }
}