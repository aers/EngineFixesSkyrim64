#include <unordered_set>

#include <ShlObj.h>

#include "skse64_common/Utilities.h"

#include "REL/Relocation.h"
#include "SKSE/SafeWrite.h"
#include "SKSE/Version.h"

#include "utils.h"

std::string savesFolderPath;
std::unordered_set<std::string> existingSaves;

bool InitSavePath(int folderID, const char* relPath)
{
    char path[MAX_PATH];

    HRESULT err = SHGetFolderPath(NULL, folderID | CSIDL_FLAG_CREATE, NULL, SHGFP_TYPE_CURRENT, path);
    if (!SUCCEEDED(err))
    {
        return false;
    }

    strcat_s(path, sizeof(path), relPath);

    savesFolderPath = path;

    return true;
}

bool CleanSKSECosaves()
{
    _VMESSAGE("- skse cosave cleaner -");

    if (!InitSavePath(CSIDL_MYDOCUMENTS, R"(\My Games\Skyrim Special Edition\Saves\)"))
    {
        _VMESSAGE("unable to find saves folder, aborting");
        return false;
    }

    _VMESSAGE("save folder: %s", savesFolderPath.c_str());

    // find valid saves
    WIN32_FIND_DATA ffd;
    HANDLE hFind = INVALID_HANDLE_VALUE;
    DWORD dwError = 0;

    hFind = FindFirstFile((savesFolderPath + "*.ess").c_str(), &ffd);

    if (hFind == INVALID_HANDLE_VALUE)
    {
        _VMESSAGE("no save files found in save folder");
        return true;
    }

    do
    {
        //_MESSAGE("found %s", ffd.cFileName);
        std::string tempString = ffd.cFileName;
        tempString.erase(tempString.size() - 4, 4);
        existingSaves.insert(tempString);
    } while (FindNextFile(hFind, &ffd) != 0);

    dwError = GetLastError();

    if (dwError != ERROR_NO_MORE_FILES)
    {
        _VMESSAGE("find file loop failed with error %d, aborting", dwError);
        FindClose(hFind);
        return false;
    }

    FindClose(hFind);

    _VMESSAGE("found %d valid saves", existingSaves.size());

    // erase cosaves
    hFind = FindFirstFile((savesFolderPath + "*.skse").c_str(), &ffd);

    if (hFind == INVALID_HANDLE_VALUE)
    {
        _VMESSAGE("no skse cosave files found in save folder");
        return true;
    }

    int countDeleted = 0;

    do
    {
        std::string tempString = ffd.cFileName;
        tempString.erase(tempString.size() - 5, 5);
        if (!existingSaves.count(tempString))
        {
            //_MESSAGE("orphaned cosave %s detected, deleting", ffd.cFileName);
            if (DeleteFile((savesFolderPath + ffd.cFileName).c_str()))
            {
                countDeleted++;
                //_MESSAGE("deleted");
            }
            else
            {
                dwError = GetLastError();
                _VMESSAGE("delete failed on %s, dwError %d", ffd.cFileName, dwError);
            }
        }
    } while (FindNextFile(hFind, &ffd) != 0);

    dwError = GetLastError();

    if (dwError != ERROR_NO_MORE_FILES)
    {
        _VMESSAGE("find file loop failed with error %d, aborting", dwError);
        FindClose(hFind);
        return false;
    }

    FindClose(hFind);

    _VMESSAGE("deleted %d orphaned cosaves", countDeleted);

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
