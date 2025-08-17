#pragma once

namespace Fixes::GetKeywordItemCount
{
    namespace detail
    {
        inline bool Eval(RE::TESObjectREFR* a_thisObj, void* a_param1, [[maybe_unused]] void* a_param2, double& a_result)
        {
            a_result = 0.0;
            if (!a_thisObj || !a_param1)
            {
                return true;
            }

            const auto log = RE::ConsoleLog::GetSingleton();
            if (!a_thisObj->GetContainer())
            {
                if (log && log->IsConsoleMode())
                {
                    log->Print("Calling Reference is not a Container Object");
                }
                return true;
            }

            const auto keyword = static_cast<RE::BGSKeyword*>(a_param1);
            const auto inv = a_thisObj->GetInventoryCounts([&](RE::TESBoundObject& a_object) -> bool {
                auto keywordForm = a_object.As<RE::BGSKeywordForm>();
                return keywordForm && keywordForm->HasKeyword(keyword);
            });

            for (const auto& val : inv | std::views::values)
            {
                a_result += val;
            }


            if (log && log->IsConsoleMode())
            {
                log->Print("GetKeywordItemCount >> %0.2f", a_result);
            }

            return true;
        }

        inline bool Execute(const RE::SCRIPT_PARAMETER*, RE::SCRIPT_FUNCTION::ScriptData*, RE::TESObjectREFR* a_thisObj, RE::TESObjectREFR*, RE::Script* a_scriptObj, RE::ScriptLocals*, double& a_result, std::uint32_t&)
        {
            if (!a_scriptObj || a_scriptObj->refObjects.empty())
            {
                return false;
            }

            auto param = a_scriptObj->refObjects.front();
            if (!param->form || param->form->IsNot(RE::FormType::Keyword))
            {
                return false;
            }

            return Eval(a_thisObj, param->form, 0, a_result);
        }

        inline constexpr char LONG_NAME[] = "GetKeywordItemCount";
    }

    inline void Install()
    {
        if (const auto command = RE::SCRIPT_FUNCTION::LocateScriptCommand(detail::LONG_NAME))
        {
            command->executeFunction = detail::Execute;
            command->conditionFunction = detail::Eval;

            REX::INFO("installed getkeyworditemcount fix"sv);
        }
    }
}