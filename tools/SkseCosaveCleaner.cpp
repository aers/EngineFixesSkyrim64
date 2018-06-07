#include "ShlObj.h"
#include <unordered_set>


namespace SkseCosaveCleaner
{
	std::string savesFolderPath;
	std::unordered_set<std::string> existingSaves;

	bool InitSavePath(int folderID, const char * relPath)
	{
		char	path[MAX_PATH];

		HRESULT err = SHGetFolderPath(NULL, folderID | CSIDL_FLAG_CREATE, NULL, SHGFP_TYPE_CURRENT, path);
		if (!SUCCEEDED(err))
		{
			return false;
		}

		strcat_s(path, sizeof(path), relPath);

		savesFolderPath = path;

		return true;
	}

	bool Clean()
	{
		_MESSAGE("- skse cosave cleaner -");

		if (!InitSavePath(CSIDL_MYDOCUMENTS, R"(\My Games\Skyrim Special Edition\Saves\)"))
		{
			_MESSAGE("unable to find saves folder, aborting");
			return false;
		}

		_MESSAGE("save folder: %s", savesFolderPath.c_str());

		// find valid saves
		WIN32_FIND_DATA ffd;
		HANDLE hFind = INVALID_HANDLE_VALUE;
		DWORD dwError = 0;

		hFind = FindFirstFile((savesFolderPath + "*.ess").c_str(), &ffd);

		if (hFind == INVALID_HANDLE_VALUE)
		{
			_MESSAGE("no save files found in save folder");
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
			_MESSAGE("find file loop failed with error %d, aborting", dwError);
			FindClose(hFind);
			return false;
		}

		FindClose(hFind);

		_MESSAGE("found %d valid saves", existingSaves.size());

		// erase cosaves
		hFind = FindFirstFile((savesFolderPath + "*.skse").c_str(), &ffd);

		if (hFind == INVALID_HANDLE_VALUE)
		{
			_MESSAGE("no skse cosave files found in save folder");
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
					_MESSAGE("delete failed on %s, dwError %d", ffd.cFileName, dwError);
				}
			}
		} while (FindNextFile(hFind, &ffd) != 0);

		dwError = GetLastError();

		if (dwError != ERROR_NO_MORE_FILES)
		{
			_MESSAGE("find file loop failed with error %d, aborting", dwError);
			FindClose(hFind);
			return false;
		}

		FindClose(hFind);

		_MESSAGE("deleted %d orphaned cosaves", countDeleted);

		return true;
	}
}