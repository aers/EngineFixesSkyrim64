#include "../util.h"

// see https://docs.microsoft.com/en-us/cpp/c-runtime-library/reference/setmaxstdio
// fopen hooks just for logging purposes

namespace MaxStdio
{
	decltype(&fopen_s) VC140_fopen_s;
	errno_t hk_fopen_s(FILE **File, const char *Filename, const char *Mode)
	{
		errno_t err = VC140_fopen_s(File, Filename, Mode);

		if (err != 0)
			_MESSAGE("WARNING: Error occurred trying to open file: fopen_s(%s, %s), errno %d", Filename, Mode, err);

		return err;
	}

	decltype(&_wfopen_s) VC140_wfopen_s;
	errno_t hk_wfopen_s(FILE **File, const wchar_t *Filename, const wchar_t *Mode)
	{
		errno_t err = VC140_wfopen_s(File, Filename, Mode);

		if (err != 0)
			_MESSAGE("WARNING: Error occurred trying to open file: _wfopen_s(%p, %p), errno %d", Filename, Mode, err);

		return err;
	}

	decltype(&fopen) VC140_fopen;
	FILE *hk_fopen(const char *Filename, const char *Mode)
	{
		FILE *f = VC140_fopen(Filename, Mode);

		if (!f)
			_MESSAGE("WARNING: Error occurred trying to open file: fopen(%s, %s)", Filename, Mode);

		return f;
	}

	bool Patch()
	{
		_MESSAGE("- max stdio -");

		const HMODULE crtStdioModule = GetModuleHandleA("API-MS-WIN-CRT-STDIO-L1-1-0.DLL");

		if (!crtStdioModule)
		{
			_MESSAGE("crt stdio module not found, failed");
			return false;
		}
		
		int newMax = ((decltype(&_setmaxstdio))GetProcAddress(crtStdioModule, "_setmaxstdio"))(2048);

		_MESSAGE("max stdio set to %d", newMax);

		*(void **)&VC140_fopen_s = (uintptr_t *)PatchIAT(GetFnAddr(hk_fopen_s), "API-MS-WIN-CRT-STDIO-L1-1-0.DLL", "fopen_s");
		*(void **)&VC140_wfopen_s = (uintptr_t *)PatchIAT(GetFnAddr(hk_wfopen_s), "API-MS-WIN-CRT-STDIO-L1-1-0.DLL", "wfopen_s");
		*(void **)&VC140_fopen = (uintptr_t *)PatchIAT(GetFnAddr(hk_fopen), "API-MS-WIN-CRT-STDIO-L1-1-0.DLL", "fopen");

		_MESSAGE("hooked stdio fopen for logging purposes");
		_MESSAGE("success");

		return true;
	}
}