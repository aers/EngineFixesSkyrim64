#pragma once

bool CleanSKSECosaves();
uintptr_t PatchIAT(uintptr_t func, const char* dllName, const char* importName);
