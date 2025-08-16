#pragma once

namespace Fixes::BethesdaNetCrash
{
    namespace detail
    {
        inline errno_t hk_wcsrtombs_s(std::size_t* a_retval, char* a_dst, rsize_t a_dstsz, const wchar_t** a_src, rsize_t a_len, [[maybe_unused]] std::mbstate_t* a_ps)
        {
            const auto numChars = WideCharToMultiByte(CP_UTF8, 0, *a_src, static_cast<int>(a_len), nullptr, 0, nullptr, nullptr);

            std::string str;
            char* dst = nullptr;
            rsize_t dstsz = 0;
            if (a_dst)
            {
                dst = a_dst;
                dstsz = a_dstsz;
            }
            else
            {
                str.resize(numChars);
                dst = str.data();
                dstsz = str.max_size();
            }

            bool err;
            if (a_src && numChars != 0 && numChars <= dstsz)
                err = WideCharToMultiByte(CP_UTF8, 0, *a_src, static_cast<int>(a_len), dst, numChars, nullptr, nullptr) ? false : true;
            else
                err = true;

            if (err)
            {
                if (a_retval)
                    *a_retval = static_cast<std::size_t>(-1);
                if (a_dst && a_dstsz != 0 && a_dstsz <= (std::numeric_limits<rsize_t>::max)())
                    a_dst[0] = '\0';
                return GetLastError();
            }

            if (a_retval)
                *a_retval = static_cast<std::size_t>(numChars);

            return 0;
        }
    }

    inline void Install()
    {
        REL::PatchIAT(detail::hk_wcsrtombs_s, "API-MS-WIN-CRT-CONVERT-L1-1-0.dll", "wcsrtombs_s");

        REX::INFO("installed bethesda net crash fix"sv);
    }
}