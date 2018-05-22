// 48 
#include "GameForms.h"

namespace TES
{
	class BGSShaderParticleGeometryData : public TESForm
	{
	public:
		enum { kTypeID = kFormType_SPGD };

		tArray<float>	data;		// 20 "DATA" in form; actually a mixed array of floats/ints, but is primarily floats
		TESTexture	texture;	// 38
	};
}
