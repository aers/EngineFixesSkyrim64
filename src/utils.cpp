#include "utils.h"

std::optional<std::filesystem::path> GetSavesPath()
{
    wchar_t* buffer;
    auto result = SHGetKnownFolderPath(FOLDERID_Documents, KNOWN_FOLDER_FLAG::KF_FLAG_DEFAULT, nullptr, std::addressof(buffer));
    std::unique_ptr<wchar_t[], decltype(&CoTaskMemFree)> mydocs(buffer, CoTaskMemFree);
    if (result != S_OK)
        return std::nullopt;

    std::filesystem::path path = mydocs.get();
    path /= "My Games"sv;
    path /= "Skyrim Special Edition"sv;

    auto sLocalSavePath = RE::GetINISetting("sLocalSavePath:General");
    if (sLocalSavePath)
        path /= sLocalSavePath->GetString();
    else
    {
        logger::error("Failed to get local save path ini setting"sv);
        return std::nullopt;
    }

    std::error_code ec;
    if (!std::filesystem::exists(path, ec) || ec)
    {
        logger::error(FMT_STRING("Path \"{}\" does not exist"), path.string());
        if (ec)
            logger::error(FMT_STRING("Error message: {}"), ec.message());

        return std::nullopt;
    }
    else
        return std::make_optional(std::move(path));
}

bool CleanSKSECosaves()
{
    auto savesPath = GetSavesPath();
    if (!savesPath)
        return false;

    constexpr auto REGEX_CONSTANTS = boost::regex_constants::ECMAScript | boost::regex_constants::icase;
    const boost::regex cosavePattern(R"(.*\.skse$)", REGEX_CONSTANTS);
    std::vector<std::filesystem::path> matches;
    std::error_code ec;

    for (const auto& dirEntry : std::filesystem::directory_iterator(*savesPath))
    {
        if (dirEntry.is_regular_file())
        {
            const auto& cosave = dirEntry.path();
            if (boost::regex_match(cosave.filename().string(), cosavePattern))
            {
                auto save = cosave;
                save.replace_extension(".ess"sv);
                if (!std::filesystem::exists(save, ec))
                {
                    if (!ec)
                        matches.push_back(cosave);
                    else
                        logger::error(FMT_STRING("Error while checking if \"{}\" exists: {}"), save.string(), ec.message());
                }
            }
        }
    }

    for (const auto& match : matches)
    {
        if (!std::filesystem::remove(match, ec) || ec)
        {
            logger::error(FMT_STRING("Failed to remove \"{}\""), match.string());
            if (ec)
                logger::error(FMT_STRING("Error message: {}"), ec.message());
        }
    }

    return true;
}
