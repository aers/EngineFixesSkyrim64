#pragma once

namespace Fixes::IsInCombatConsoleCrash
{
    namespace detail
    {
        inline bool NotAnActor(RE::TESObjectREFR* a_thisObj, double& a_result)
        {
            if (a_thisObj && a_thisObj->Is(RE::FormType::ActorCharacter)) {
                return false;
            }

            a_result = 0.0;

            const auto log = RE::ConsoleLog::GetSingleton();
            if (log && log->IsConsoleMode()) {
                log->Print("%s is not in combat", a_thisObj ? a_thisObj->GetDisplayFullName() : "");
            }

            return true;
        }

        inline RE::SCRIPT_FUNCTION::Execute_t*   _Execute;
        inline RE::SCRIPT_FUNCTION::Condition_t* _Eval;

        inline bool Execute(const RE::SCRIPT_PARAMETER* a_paramInfo, RE::SCRIPT_FUNCTION::ScriptData* a_scriptData, RE::TESObjectREFR* a_thisObj, RE::TESObjectREFR* a_containingObj, RE::Script* a_scriptObj, RE::ScriptLocals* a_locals, double& a_result, std::uint32_t& a_opcodeOffsetPtr)
        {
            return NotAnActor(a_thisObj, a_result) || _Execute(a_paramInfo, a_scriptData, a_thisObj, a_containingObj, a_scriptObj, a_locals, a_result, a_opcodeOffsetPtr);
        }

        inline bool Eval(RE::TESObjectREFR* a_thisObj, void* a_param1, void* a_param2, double& a_result)
        {
            return NotAnActor(a_thisObj, a_result) || _Eval(a_thisObj, a_param1, a_param2, a_result);
        }

        inline constexpr char LONG_NAME[] = "IsInCombat";
    }

    inline void Install()
    {
        if (const auto command = RE::SCRIPT_FUNCTION::LocateScriptCommand(detail::LONG_NAME)) {
            detail::_Execute = command->executeFunction;
            detail::_Eval = command->conditionFunction;
            command->executeFunction = detail::Execute;
            command->conditionFunction = detail::Eval;

            REX::INFO("installed isincombat console crash fix"sv);
        }
    }
}