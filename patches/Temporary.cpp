#include "../../skse64_common/SafeWrite.h"

namespace Temporary
{
	void TempFixSKEE()
	{
		const auto skee = reinterpret_cast<uintptr_t>(GetModuleHandleA("skee64"));
		if (skee)
		{
			_MESSAGE("skee found, enabling temporary patch to fix XPMSSE 4.3 issue");
			if (*(uint32_t *)(skee + 0x6C10) != 0x0FD28548)
			{
				_MESSAGE("unknown skee version, canceling");
			}
			else
			{
				SafeWrite8(skee + 0x6C10, 0xC3);
				_MESSAGE("patched");
			}
		}
	}

	bool Patch()
	{
		_MESSAGE("- temporary fixes -");
		TempFixSKEE();
		return true;
	}
}