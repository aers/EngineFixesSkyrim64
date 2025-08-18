#include "form_caching.h"

namespace Patches::FormCaching
{
    namespace detail
    {
        RE::TESForm* TESDataHandler_GetForm(RE::FormID a_formId)
        {
            RE::TESForm* formPointer = nullptr;

            const std::uint8_t masterId = (a_formId & 0xFF000000) >> 24;
            const std::uint32_t baseId = (a_formId & 0x00FFFFFF);

            // lookup form in our cache first
            {
                HashMap::const_accessor a;

                if (g_formCache[masterId].find(a, baseId)) {
                    formPointer = a->second;
                    return formPointer;
                }
            }

            // if (g_formCache[masterId].if_contains(baseId, [&formPointer](const HashMap::value_type& v) { formPointer = v.second; }))
            // {
            //     return formPointer;
            // }

            // lookup form in bethesda's map
            formPointer = RE::TESForm::LookupByID(a_formId);

            if (formPointer)
            {
                g_formCache[masterId].emplace(baseId, formPointer);
                // g_formCache[masterId].try_emplace_l(baseId, [&formPointer](HashMap::value_type& v) { v.second = formPointer; }, formPointer);
            }

            return formPointer;
        }
    }
}