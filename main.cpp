#include "../skse64_loader_common/IdentifyEXE.h"
#include "../skse64/PluginAPI.h"
#include "../skse64_common/skse_version.h"
#include "../skse64_common/BranchTrampoline.h"

#include <ShlObj.h>

#include "config.h"
#include "patches.h"
#include "util.h"
#include <cinttypes>

IDebugLog	gLog;
bool		preloaded = false;

// borrowed from expired https://github.com/expired6978/F4SEPlugins/blob/master/f4ee/main.cpp
const std::string & GetRuntimeDirectory(void)
{
	static std::string s_runtimeDirectory;

	if (s_runtimeDirectory.empty())
	{
		// can't determine how many bytes we'll need, hope it's not more than MAX_PATH
		char	runtimePathBuf[MAX_PATH];
		UInt32	runtimePathLength = GetModuleFileName(GetModuleHandle(NULL), runtimePathBuf, sizeof(runtimePathBuf));

		if (runtimePathLength && (runtimePathLength < sizeof(runtimePathBuf)))
		{
			std::string	runtimePath(runtimePathBuf, runtimePathLength);

			// truncate at last slash
			std::string::size_type	lastSlash = runtimePath.rfind('\\');
			if (lastSlash != std::string::npos)	// if we don't find a slash something is VERY WRONG
			{
				s_runtimeDirectory = runtimePath.substr(0, lastSlash + 1);

				_DMESSAGE("runtime root = %s", s_runtimeDirectory.c_str());
			}
			else
			{
				_WARNING("no slash in runtime path? (%s)", runtimePath.c_str());
			}
		}
		else
		{
			_WARNING("couldn't find runtime path (len = %d, err = %08X)", runtimePathLength, GetLastError());
		}
	}

	return s_runtimeDirectory;
}

void SetupLog()
{
	gLog.OpenRelative(CSIDL_MYDOCUMENTS, R"(\My Games\Skyrim Special Edition\SKSE\EngineFixes64.log)");
#ifdef _DEBUG
	gLog.SetLogLevel(IDebugLog::kLevel_DebugMessage);
#else
	gLog.SetLogLevel(IDebugLog::kLevel_Message);
#endif		

	_MESSAGE("EngineFixes64");
}

void LoadConfig()
{
	const std::string& runtimePath = GetRuntimeDirectory();

	if (config::LoadConfig(runtimePath + R"(Data\SKSE\plugins\EngineFixes64.ini)"))
		_MESSAGE("loaded configuration successfully");
	else
		_MESSAGE("using default config");
}

extern "C" {
	// preloader
	void Initialize()
	{
		SetupLog();
		// check version
		// can't use the skse plugin interface for obvious reasons
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

				const uint32_t skse_internal_version = MAKE_EXE_VERSION(version >> 48, version >> 32, version >> 16);

				if (skse_internal_version != RUNTIME_VERSION_1_5_39)
				{
					_FATALERROR("unsupported runtime version %08X", version);
					return;
				}
			}
		}	

		preloaded = true;
		LoadConfig();

		_MESSAGE("plugin was preloaded, allowing preload-required features");
	}

	bool SKSEPlugin_Query(const SKSEInterface * skse, PluginInfo * info)
	{
		if (!preloaded)
			SetupLog();

		// populate info structure
		info->infoVersion = PluginInfo::kInfoVersion;
		info->name = "EngineFixes64 plugin";
		info->version = 1;

		if (skse->isEditor)
		{
			_MESSAGE("loaded in editor, marking as incompatible");
			return false;
		}
		else if (skse->runtimeVersion != RUNTIME_VERSION_1_5_39)
		{
			_FATALERROR("unsupported runtime version %08X", skse->runtimeVersion);
			return false;
		}

		return true;
	}

	bool SKSEPlugin_Load(const SKSEInterface * skse) 
	{
		if (!g_branchTrampoline.Create(1024 * 64))
		{
			_FATALERROR("couldn't create branch trampoline. this is fatal. skipping remainder of init process.");
			return false;
		}

		if (!g_localTrampoline.Create(1024 * 64, nullptr))
		{
			_FATALERROR("couldn't create codegen buffer. this is fatal. skipping remainder of init process.");
			return false;
		}

		if (!preloaded)
			LoadConfig();

		_MESSAGE("patching game engine");

		if (config::patchFormCaching)
			FormCaching::Patch();

		if (config::patchBSReadWriteLock)
			BSReadWriteLockCustom::Patch();

		if (config::patchDoublePerkApply)
			DoublePerkApply::Patch();

		if (config::patchSlowTimeCameraMovement)
			SlowTimeCameraMove::Patch();

		if (config::patchVerticalLookSensitivity)
			VerticalLookSensitivity::Patch();

		if (config::patchWaterflowTimer)
			WaterflowTimer::Patch();

		if (config::patchTreeReflections)
			TreeReflection::Patch();

		_MESSAGE("all patches applied");

		return true;
	}
};