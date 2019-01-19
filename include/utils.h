#pragma once

uint32_t GetGameVersion();
uintptr_t PatchIAT(uintptr_t func, const char * dllName, const char * importName);