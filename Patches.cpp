#include "bugs/DoublePerkApply.h"
#include "bugs/WaterflowTimer.h"
#include "bugs/TreeReflections.h"
#include "performance/GlobalFormTableCache.h"
#include "performance/DistantTreeCache.h"
#include "bugs/SlowTimeCameraMove.h"

bool PatchGame()
{
	gLog.SetSource("Patches");
	PatchTreeReflections();
	PatchSlowTimeCameraMove();
	_MESSAGE("Patching engine bugs...");
	PatchDoublePerkApply();
	PatchWaterflowTimer();
	gLog.SetSource("Patches");
	_MESSAGE("Done");
	_MESSAGE("Patching engine performance...");
	PatchGlobalFormTableCache();
	PatchDistantTreeCache();
	gLog.SetSource("Patches");
	_MESSAGE("Done");
	gLog.SetSource("RunLog");
	return true;
}
