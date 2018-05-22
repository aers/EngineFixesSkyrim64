#pragma once
#include "../TES/BGSSoundCategory.h"


namespace SaveAddedSoundCategories
{
	struct SoundCategoryInfo
	{
		std::string PluginName;
		uint32_t LocalFormId;
		TES::BGSSoundCategory * Category;

		SoundCategoryInfo(std::string name, uint32_t formid, TES::BGSSoundCategory * cat) : PluginName(name), LocalFormId(formid), Category(cat)
		{
		}
	};

	bool Patch();
	void LoadVolumes();
}