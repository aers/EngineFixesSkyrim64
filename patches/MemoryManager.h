#pragma once

namespace MemoryManager
{
	class MemoryManager
	{
	public:
		MemoryManager() {}
		~MemoryManager() {}

		void *Alloc(size_t Size, uint32_t Alignment, bool Aligned);
		void Free(void *Memory, bool Aligned);
	};

	bool Patch();
}