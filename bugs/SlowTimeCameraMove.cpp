#include "../skse64_common/Relocation.h"
#include "../skse64_common/SafeWrite.h"

RelocAddr<uintptr_t> CameraMove_Timer1(0x00850A7F);
RelocAddr<uintptr_t> CameraMove_Timer2(0x00850AE6);
RelocAddr<uintptr_t> CameraMove_Timer3(0x00850C4D);
RelocAddr<uintptr_t> CameraMove_Timer4(0x00850FEA);
RelocAddr<uintptr_t> CameraMove_Timer5(0x008510B7);

bool PatchSlowTimeCameraMove()
{
	SafeWrite8(CameraMove_Timer1.GetUIntPtr(), 0xC9); // C5->C9
	SafeWrite8(CameraMove_Timer2.GetUIntPtr(), 0x62); // 5E->62
	SafeWrite8(CameraMove_Timer3.GetUIntPtr(), 0xFB); // F7->FB
	SafeWrite8(CameraMove_Timer4.GetUIntPtr(), 0x5E); // 5A->5E
	SafeWrite8(CameraMove_Timer5.GetUIntPtr(), 0x91); // 8D->91

	return true;
}