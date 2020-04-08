#include "utils.h"

#include <unordered_set>

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
