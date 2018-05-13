#include "../../skse64_common/Relocation.h"
#include "../../skse64_common/BranchTrampoline.h"

#include "BSReadWriteLockCustom.h"

// thx Nukem9 https://github.com/Nukem9/SykrimSETest/blob/master/skyrim64_test/src/patches/TES/BSReadWriteLock.cpp
namespace BSReadWriteLockCustom
{
	// all these functions are in order
	// E8 ? ? ? ? 89 37  ->
	// RelocAddr<uintptr_t> BSReadWriteLock_Ctor(0x00C077C0);
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

	bool IsWritingThread(BSReadWriteLock * thisPtr)
	{
		return thisPtr->m_ThreadId == GetCurrentThreadId();
	}

	bool TryLockForRead(BSReadWriteLock * thisPtr)
	{
		if (IsWritingThread(thisPtr))
			return true;

		// fetch_add is considerably (100%) faster than compare_exchange,
		// so here we are optimizing for the common (lock success) case.
		int16_t value = thisPtr->m_Bits.fetch_add(BSReadWriteLock::READER, std::memory_order_acquire);

		if (value & BSReadWriteLock::WRITER)
		{
			thisPtr->m_Bits.fetch_add(-BSReadWriteLock::READER, std::memory_order_release);
			return false;
		}

		return true;
	}

	void LockForRead(BSReadWriteLock * thisPtr)
	{
		for (uint32_t count = 0; !TryLockForRead(thisPtr);)
		{
			if (++count > 1000)
				YieldProcessor();
		}
	}

	void UnlockRead(BSReadWriteLock * thisPtr)
	{
		if (IsWritingThread(thisPtr))
			return;

		thisPtr->m_Bits.fetch_add(-BSReadWriteLock::READER, std::memory_order_release);
	}


	bool TryLockForWrite(BSReadWriteLock * thisPtr)
	{
		if (IsWritingThread(thisPtr))
		{
			thisPtr->m_WriteCount++;
			return true;
		}

		int16_t expect = 0;
		if (thisPtr->m_Bits.compare_exchange_strong(expect, BSReadWriteLock::WRITER, std::memory_order_acq_rel))
		{
			thisPtr->m_WriteCount = 1;
			thisPtr->m_ThreadId.store(GetCurrentThreadId(), std::memory_order_release);
			return true;
		}

		return false;
	}

	void LockForWrite(BSReadWriteLock * thisPtr)
	{
		for (uint32_t count = 0; !TryLockForWrite(thisPtr);)
		{
			if (++count > 1000)
				YieldProcessor();
		}
	}

	void UnlockWrite(BSReadWriteLock * thisPtr)
	{
		if (--thisPtr->m_WriteCount > 0)
			return;

		thisPtr->m_ThreadId.store(0, std::memory_order_release);
		thisPtr->m_Bits.fetch_and(~BSReadWriteLock::WRITER, std::memory_order_release);
	}



	void LockForReadAndWrite(BSReadWriteLock * thisPtr)
	{
		// This is only called from BSAutoReadAndWriteLock (no-op, it's always a write lock now)
	}


	 BSAutoReadAndWriteLock * Initialize(BSAutoReadAndWriteLock * thisPtr, BSReadWriteLock *Child)
	 {
	 	thisPtr->m_Lock = Child;
	 	LockForWrite(thisPtr->m_Lock);
 
	 	return thisPtr;
	 }
 
	 void Deinitialize(BSAutoReadAndWriteLock * thisPtr)
	 {
	 	UnlockWrite(thisPtr->m_Lock);
	 }

	bool Patch()
	{
		_MESSAGE("- custom bsreadwritelock (mutex) -");
		_MESSAGE("detouring functions");
		g_branchTrampoline.Write6Branch(BSReadWriteLock_LockForRead.GetUIntPtr(), uintptr_t(LockForRead));
		g_branchTrampoline.Write6Branch(BSReadWriteLock_LockForWrite.GetUIntPtr(), uintptr_t(LockForWrite));
		g_branchTrampoline.Write6Branch(BSReadWriteLock_LockForReadAndWrite.GetUIntPtr(), uintptr_t(LockForReadAndWrite));
		g_branchTrampoline.Write6Branch(BSReadWriteLock_TryLockForWrite.GetUIntPtr(), uintptr_t(TryLockForWrite));
		g_branchTrampoline.Write6Branch(BSReadWriteLock_UnlockRead.GetUIntPtr(), uintptr_t(UnlockRead));
		g_branchTrampoline.Write6Branch(BSReadWriteLock_UnlockWrite.GetUIntPtr(), uintptr_t(UnlockWrite));
		g_branchTrampoline.Write6Branch(BSReadWriteLock_IsWritingThread.GetUIntPtr(), uintptr_t(IsWritingThread));
		g_branchTrampoline.Write6Branch(BSAutoReadAndWriteLock_Initialize.GetUIntPtr(), uintptr_t(Initialize));
		g_branchTrampoline.Write6Branch(BSAutoReadAndWriteLock_Deinitialize.GetUIntPtr(), uintptr_t(Deinitialize));
		_MESSAGE("success");
		return true;
	}
}