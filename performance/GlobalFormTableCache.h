#pragma once
#include "../../skse64_common/Relocation.h"
#include "../../skse64/GameTypes.h"
#include "../TES/BSTHashMap.h"

bool PatchGlobalFormTableCache();
TESForm *GetFormById_Hook(unsigned int FormId);

#define TES_FORM_MASTER_COUNT	256			// Maximum master file index + 1 (2^8, 8 bits)
#define TES_FORM_INDEX_COUNT	16777216	// Maximum index + 1 (2^24, 24 bits)

// offsets 
// sse 1.5.39
#define off_BSREADWRITELOCK_LOCKREAD	0x00C077E0
#define off_BSREADWRITELOCK_UNLOCKREAD  0x00C07AA0
#define off_GLOBALFORMTABLELOCK			0x01EEB150
#define off_GLOBALFORMTABLEPTR			0x01EEACB8

#define off_UNKNOWNFORMFUNCTION0		0x00194970
#define off_UNKNOWNFORMFUNCTION1		0x00196070
#define off_UNKNOWNFORMFUNCTION2		0x00195DA0
#define off_UNKNOWNFORMFUNCTION3		0x00196960

// mutex functions
typedef void(*_MutexLockRead)(BSReadWriteLock * lock);
extern RelocAddr<_MutexLockRead> BSReadWriteLock_LockRead;
typedef void(*_MutexUnlockRead)(BSReadWriteLock * lock);
extern RelocAddr<_MutexUnlockRead> BSReadWriteLock_UnlockRead;

// patch addresses
extern RelocPtr<BSReadWriteLock> g_formTableLock;
extern RelocPtr<BSTHashMap<UInt32, TESForm *> *> g_formTable;

typedef void(*UnknownFormFunction0_)(__int64 form, bool a2);
extern RelocAddr<UnknownFormFunction0_> origFunc0HookAddr;
extern UnknownFormFunction0_ origFunc0;

typedef __int64(*UnknownFormFunction1_)(__int64 a1, __int64 a2, int a3, DWORD *formId, __int64 *a5);
extern RelocAddr <UnknownFormFunction1_> origFunc1HookAddr;
extern UnknownFormFunction1_ origFunc1;

typedef __int64(*UnknownFormFunction2_)(__int64 a1, __int64 a2, int a3, DWORD *formId, __int64 **a5);
extern RelocAddr <UnknownFormFunction2_> origFunc2HookAddr;
extern UnknownFormFunction2_ origFunc2;

typedef __int64(*UnknownFormFunction3_)(__int64 a1, __int64 a2, int a3, __int64 a4);
extern RelocAddr <UnknownFormFunction3_> origFunc3HookAddr;
extern UnknownFormFunction3_ origFunc3;

