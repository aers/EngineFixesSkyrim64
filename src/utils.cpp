#include "skse64_common/Utilities.h"
#include "skse64_common/SafeWrite.h"
#include "skse64_common/skse_version.h"

#include "utils.h"

bool GetFileVersion(const char* path, VS_FIXEDFILEINFO* info, std::string* outProductName,
                    std::string* outProductVersion)
{
    bool result = false;

    UInt32 versionSize = GetFileVersionInfoSize(path, nullptr);
    if (!versionSize)
    {
        _ERROR("GetFileVersionInfoSize failed (%08X)", GetLastError());
        return false;
    }

    UInt8* versionBuf = new UInt8[versionSize];
    if (versionBuf)
    {
        if (GetFileVersionInfo(path, NULL, versionSize, versionBuf))
        {
            VS_FIXEDFILEINFO* retrievedInfo = nullptr;
            UInt32 realVersionSize = sizeof(VS_FIXEDFILEINFO);

            if (VerQueryValue(versionBuf, "\\", (void **)&retrievedInfo, (PUINT)&realVersionSize) && retrievedInfo)
            {
                *info = *retrievedInfo;
                result = true;
            }
            else
            {
                _ERROR("VerQueryValue failed (%08X)", GetLastError());
            }

            if (outProductName)
            {
                // try to get the product name, failure is ok
                char* productName = nullptr;
                UInt32 productNameLen = 0;
                if (VerQueryValue(versionBuf, "\\StringFileInfo\\040904B0\\ProductName", (void **)&productName,
                                  (PUINT)&productNameLen) && productNameLen && productName)
                {
                    *outProductName = productName;
                }
            }

            {
                char* productVersion = nullptr;
                UInt32 productVersionLen = 0;
                if (VerQueryValue(versionBuf, "\\StringFileInfo\\040904B0\\ProductVersion", (void **)&productVersion,
                                  (PUINT)&productVersionLen) && productVersionLen && productVersion)
                {
                    *outProductVersion = productVersion;
                }
            }
        }
        else
        {
            _ERROR("GetFileVersionInfo failed (%08X)", GetLastError());
        }

        delete[] versionBuf;
    }

    return result;
}

bool VersionStrToInt(const std::string& verStr, UInt64* out)
{
    UInt64 result = 0;
    int parts[4];

    if (sscanf_s(verStr.c_str(), "%d.%d.%d.%d", &parts[0], &parts[1], &parts[2], &parts[3]) != 4)
        return false;

    for (int i = 0; i < 4; i++)
    {
        if (parts[i] > 0xFFFF)
            return false;

        result <<= 16;
        result |= parts[i];
    }

    *out = result;

    return true;
}

uint32_t GetGameVersion()
{
    TCHAR filename[MAX_PATH];
    GetModuleFileName(nullptr, filename, MAX_PATH);

    VS_FIXEDFILEINFO info;
    std::string productName;
    std::string productVersion;

    if (GetFileVersion(filename, &info, &productName, &productVersion))
    {
        uint64_t version;

        VersionStrToInt(productVersion, &version);

        return MAKE_EXE_VERSION(version >> 48, version >> 32, version >> 16);
    }

    return 0;
}

uintptr_t PatchIAT(uintptr_t func, const char* dllName, const char* importName)
{
    const auto addr = reinterpret_cast<uintptr_t>(GetIATAddr(reinterpret_cast<void*>(RelocationManager::s_baseAddr),
                                                             dllName, importName));
    if (addr)
    {
        const auto orig = *reinterpret_cast<uintptr_t *>(addr);
        SafeWrite64(addr, func);
        return orig;
    }

    return NULL;
}
