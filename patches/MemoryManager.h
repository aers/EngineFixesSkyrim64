#pragma once

namespace MemoryManager
{
	class MemoryManager
	{
	private:
		MemoryManager() {}
		~MemoryManager() {}

	public:
		void *Alloc(size_t Size, uint32_t Alignment, bool Aligned);
		void Free(void *Memory, bool Aligned);
	};

	class ScrapHeap
	{
	private:
		ScrapHeap() {}
		~ScrapHeap() {}

	public:
		const static uint32_t MAX_ALLOC_SIZE = 0x4000000;

		void *Alloc(size_t Size, uint32_t Alignment);
		void Free(void *Memory);
	};

	bool Patch();
}