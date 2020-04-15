#include <unordered_set>

#include <ShlObj.h>

#include "skse64_common/Utilities.h"

#include <filesystem>
#include <memory>
#include <optional>
#include <regex>
#include <system_error>
#include <vector>

#include "RE/Skyrim.h"
#include "REL/Relocation.h"
#include "SKSE/SafeWrite.h"
#include "SKSE/Version.h"

#include "utils.h"

std::optional<std::filesystem::path> GetSavesPath()
{
    wchar_t* buffer;
    auto result = SHGetKnownFolderPath(FOLDERID_Documents, KNOWN_FOLDER_FLAG::KF_FLAG_DEFAULT, NULL, &buffer);
    std::unique_ptr<wchar_t[], decltype(&CoTaskMemFree)> mydocs(buffer, CoTaskMemFree);
    if (result != S_OK)
    {
        return std::nullopt;
    }

    std::filesystem::path path = mydocs.get();
    path /= "My Games";
    path /= "Skyrim Special Edition";

    auto sLocalSavePath = RE::GetINISetting("sLocalSavePath:General");
    if (sLocalSavePath)
    {
        path /= sLocalSavePath->GetString();
    }
    else
    {
        _ERROR("Failed to get local save path ini setting");
        return std::nullopt;
    }

    std::error_code ec;
    if (!std::filesystem::exists(path, ec) || ec)
    {
        _ERROR("Path \"%s\" does not exist", path.c_str());
        if (ec)
        {
            _ERROR("Error message: %s", ec.message().c_str());
        }
        return std::nullopt;
    }
    else
    {
        return std::make_optional(std::move(path));
    }
}

bool CleanSKSECosaves()
{
    auto savesPath = GetSavesPath();
    if (!savesPath)
    {
        return false;
    }

    constexpr auto REGEX_CONSTANTS = std::regex_constants::grep | std::regex_constants::icase;
    std::regex savePattern(".*\\.ess$", REGEX_CONSTANTS);
    std::regex cosavePattern(".*\\.skse$", REGEX_CONSTANTS);
    std::vector<std::filesystem::path> matches;
    std::error_code ec;

    for (auto& dirEntry : std::filesystem::directory_iterator(*savesPath))
    {
        if (dirEntry.is_regular_file())
        {
            auto& path = dirEntry.path();
            if (std::regex_match(path.filename().string(), cosavePattern))
            {
                std::filesystem::path saveFile = path.parent_path();
                saveFile /= path.stem().string() + ".ess";
                if (!std::filesystem::exists(saveFile, ec))
                {
                    if (!ec)
                    {
                        matches.push_back(path);
                    }
                    else
                    {
                        _ERROR("Error while checking if \"%s\" exists: %s", saveFile.c_str(), ec.message().c_str());
                    }
                }
            }
        }
    }

    for (auto& match : matches)
    {
        if (!std::filesystem::remove(match, ec) || ec)
        {
            _ERROR("Failed to remove \"%s\"", match.c_str());
            if (ec)
            {
                _ERROR("Error message: %s", ec.message().c_str());
            }
        }
    }

    return true;
}

bool GetFileVersion(const char* path, VS_FIXEDFILEINFO* info, std::string* outProductName, std::string* outProductVersion)
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

            if (VerQueryValue(versionBuf, "\\", (void**)&retrievedInfo, (PUINT)&realVersionSize) && retrievedInfo)
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
                if (VerQueryValue(versionBuf, "\\StringFileInfo\\040904B0\\ProductName", (void**)&productName, (PUINT)&productNameLen) && productNameLen && productName)
                {
                    *outProductName = productName;
                }
            }

            {
                char* productVersion = nullptr;
                UInt32 productVersionLen = 0;
                if (VerQueryValue(versionBuf, "\\StringFileInfo\\040904B0\\ProductVersion", (void**)&productVersion, (PUINT)&productVersionLen) && productVersionLen && productVersion)
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

uintptr_t PatchIAT(uintptr_t func, const char* dllName, const char* importName)
{
    const auto addr = reinterpret_cast<std::uintptr_t>(GetIATAddr(REL::Module::BasePtr(), dllName, importName));
    if (addr)
    {
        const auto orig = *reinterpret_cast<std::uintptr_t*>(addr);
        SKSE::SafeWrite64(addr, func);
        return orig;
    }

    return NULL;
}
