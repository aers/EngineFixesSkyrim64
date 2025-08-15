#include "clean_cosaves.h"

namespace Util::CoSaves
{
    std::optional<std::filesystem::path> GetSavesPath()
    {
        wchar_t* buffer;
        auto result = REX::W32::SHGetKnownFolderPath(REX::W32::FOLDERID_Documents, KNOWN_FOLDER_FLAG::KF_FLAG_DEFAULT, nullptr, std::addressof(buffer));
        std::unique_ptr<wchar_t[], decltype(&REX::W32::CoTaskMemFree)> mydocs(buffer, REX::W32::CoTaskMemFree);
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
            REX::ERROR("Failed to get local save path ini setting"sv);
            return std::nullopt;
        }

        std::error_code ec;
        if (!std::filesystem::exists(path, ec) || ec)
        {
            REX::ERROR("Path \"{}\" does not exist"sv, path.string());
            if (ec)
                REX::ERROR("Error message: {}"sv, ec.message());

            return std::nullopt;
        }
        else
            return std::make_optional(std::move(path));
    }

    bool Clean()
    {
        auto savesPath = GetSavesPath();
        if (!savesPath)
            return false;

        REX::TRACE("cleaning cosaves in path {}"sv, savesPath->string());

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
                            REX::ERROR("Error while checking if \"{}\" exists: {}"sv, save.string(), ec.message());
                    }
                }
            }
        }

        for (const auto& match : matches)
        {
            REX::TRACE("removing \"{}\""sv, match.string());
            if (!std::filesystem::remove(match, ec) || ec)
            {
                REX::ERROR("Failed to remove \"{}\""sv, match.string());
                if (ec)
                    REX::ERROR("Error message: {}"sv, ec.message());
            }
        }

        return true;
    }
}