#pragma once

namespace Patches::MaxStdIO
{
    inline void Install()
    {
        const auto handle = REX::W32::GetModuleHandleW(L"API-MS-WIN-CRT-STDIO-L1-1-0.DLL");
        const auto proc =
            handle ?
                reinterpret_cast<decltype(&_setmaxstdio)>(REX::W32::GetProcAddress(handle, "_setmaxstdio")) :
                nullptr;
        if (proc != nullptr) {
            const auto get = reinterpret_cast<decltype(&_getmaxstdio)>(REX::W32::GetProcAddress(handle, "_getmaxstdio"));
            const auto old = get();
            auto       result = proc(8192);
            if (result != -1) {
                if (get)
                    logger::info("set max stdio to {} from {}"sv, result, old);
                else
                    logger::info("set max stdio to {}"sv, result);

                return;
            }
            result = proc(2048);
            if (result != -1) {
                if (get)
                    logger::info("set max stdio to {} from {}"sv, result, old);
                else
                    logger::info("set max stdio to {}"sv, result);
            }
        } else {
            logger::error("failed to install MaxStdIO patch"sv);
        }
    }
}