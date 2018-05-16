#pragma once

#include "../skse64_common/Utilities.h"
#include "../skse64_common/SafeWrite.h"

// version stuff from skse64_loader_common
bool GetFileVersion(const char * path, VS_FIXEDFILEINFO * info, std::string * outProductName, std::string * outProductVersion);
bool VersionStrToInt(const std::string & verStr, UInt64 * out);
uintptr_t PatchIAT(uintptr_t func, const char * dllName, const char * importName);