#include "../../skse64_common/Relocation.h"
#include "../../skse64_common/BranchTrampoline.h"

#include "BSReadWriteLockCustom.h"
#include "engine_fixes_64/util.h"
#include "skse64_common/Utilities.h"

// thx Nukem9 https://github.com/Nukem9/SykrimSETest/blob/master/skyrim64_test/src/patches/TES/BSReadWriteLock.cpp
namespace BSReadWriteLockCustom
{
	// all these functions are in order
	// E8 ? ? ? ? 89 37  ->
	RelocAddr<uintptr_t> BSReadWriteLock_Ctor(0x00C077C0);
	RelocAddr<uintptr_t> BSReadWriteLock_LockForRead(0x00C077E0);
	RelocAddr<uintptr_t> BSReadWriteLock_LockForWrite(0x00C07860);
	// unused function 0x00C078F0
	RelocAddr<uintptr_t> BSReadWriteLock_LockForReadAndWrite(0x00C07960);
	// unused function 0x00C079E0
	RelocAddr<uintptr_t> BSReadWriteLock_TryLockForWrite(0x00C07A50);
	RelocAddr<uintptr_t> BSReadWriteLock_UnlockRead(0x00C07AA0);
	RelocAddr<uintptr_t> BSReadWriteLock_UnlockWrite(0x00C07AB0);
	RelocAddr<uintptr_t> BSReadWriteLock_IsWritingThread(0x00C07AE0);
	RelocAddr<uintptr_t> BSAutoReadAndWriteLock_Initialize(0x00C07B00);
	RelocAddr<uintptr_t> BSAutoReadAndWriteLock_Deinitialize(0x00C07B50);

	BSReadWriteLock::BSReadWriteLock()
	{
	}

	BSReadWriteLock::~BSReadWriteLock()
	{
	}

	void BSReadWriteLock::LockForRead()
	{
		for (uint32_t count = 0; !TryLockForRead();)
		{
			if (++count > 1000)
				YieldProcessor();
		}
	}

	void BSReadWriteLock::UnlockRead()
	{
		if (IsWritingThread())
			return;

		m_Bits.fetch_add(-READER, std::memory_order_release);
	}

	bool BSReadWriteLock::TryLockForRead()
	{
		if (IsWritingThread())
			return true;

		// fetch_add is considerably (100%) faster than compare_exchange,
		// so here we are optimizing for the common (lock success) case.
		int16_t value = m_Bits.fetch_add(READER, std::memory_order_acquire);

		if (value & WRITER)
		{
			m_Bits.fetch_add(-READER, std::memory_order_release);
			return false;
		}

		return true;
	}

	void BSReadWriteLock::LockForWrite()
	{
		for (uint32_t count = 0; !TryLockForWrite();)
		{
			if (++count > 1000)
				YieldProcessor();
		}
	}

	void BSReadWriteLock::UnlockWrite()
	{
		if (--m_WriteCount > 0)
			return;

		m_ThreadId.store(0, std::memory_order_release);
		m_Bits.fetch_and(~WRITER, std::memory_order_release);
	}

	bool BSReadWriteLock::TryLockForWrite()
	{
		if (IsWritingThread())
		{
			m_WriteCount++;
			return true;
		}

		int16_t expect = 0;
		if (m_Bits.compare_exchange_strong(expect, WRITER, std::memory_order_acq_rel))
		{
			m_WriteCount = 1;
			m_ThreadId.store(GetCurrentThreadId(), std::memory_order_release);
			return true;
		}

		return false;
	}

	void BSReadWriteLock::LockForReadAndWrite()
	{
		// This is only called from BSAutoReadAndWriteLock (no-op, it's always a write lock now)
	}

	bool BSReadWriteLock::IsWritingThread()
	{
		return m_ThreadId == GetCurrentThreadId();
	}

	BSAutoReadAndWriteLock *BSAutoReadAndWriteLock::Initialize(BSReadWriteLock *Child)
	{
		m_Lock = Child;
		m_Lock->LockForWrite();

		return this;
	}

	void BSAutoReadAndWriteLock::Deinitialize()
	{
		m_Lock->UnlockWrite();
	}

	bool Patch()
	{
		_MESSAGE("- custom bsreadwritelock (mutex) -");
		_MESSAGE("detouring functions");
		g_branchTrampoline.Write6Branch(BSReadWriteLock_Ctor.GetUIntPtr(), GetFnAddr(BSReadWriteLock::__ctor__));
		g_branchTrampoline.Write6Branch(BSReadWriteLock_LockForRead.GetUIntPtr(), GetFnAddr(&BSReadWriteLock::LockForRead));
		g_branchTrampoline.Write6Branch(BSReadWriteLock_LockForWrite.GetUIntPtr(), GetFnAddr(&BSReadWriteLock::LockForWrite));
		g_branchTrampoline.Write6Branch(BSReadWriteLock_LockForReadAndWrite.GetUIntPtr(), GetFnAddr(&BSReadWriteLock::LockForReadAndWrite));
		g_branchTrampoline.Write6Branch(BSReadWriteLock_TryLockForWrite.GetUIntPtr(), GetFnAddr(&BSReadWriteLock::TryLockForWrite));		
		g_branchTrampoline.Write6Branch(BSReadWriteLock_UnlockRead.GetUIntPtr(), GetFnAddr(&BSReadWriteLock::UnlockRead));
		g_branchTrampoline.Write6Branch(BSReadWriteLock_UnlockWrite.GetUIntPtr(), GetFnAddr(&BSReadWriteLock::UnlockWrite));
		g_branchTrampoline.Write6Branch(BSReadWriteLock_IsWritingThread.GetUIntPtr(), GetFnAddr(&BSReadWriteLock::IsWritingThread));
		g_branchTrampoline.Write6Branch(BSAutoReadAndWriteLock_Initialize.GetUIntPtr(), GetFnAddr(&BSAutoReadAndWriteLock::Initialize));
		g_branchTrampoline.Write6Branch(BSAutoReadAndWriteLock_Deinitialize.GetUIntPtr(), GetFnAddr(&BSAutoReadAndWriteLock::Deinitialize));
		_MESSAGE("success");
		return true;
	}
}