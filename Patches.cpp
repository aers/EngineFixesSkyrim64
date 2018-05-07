#include "bugs/DoublePerkApply.h"
#include "bugs/WaterflowTimer.h"
#include "performance/GlobalFormTableCache.h"
#include "performance/DistantTreeCache.h"

bool PatchGame()
{
	gLog.SetSource("Patches");
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