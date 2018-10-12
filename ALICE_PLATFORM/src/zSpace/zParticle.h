#pragma once
#ifdef zSPACE_EXPORTS
#define zSPACE_API __declspec(dllexport)
#else
#define zSPACE_API _declspec(dllimport)
#endif
#include <zSpace/zVector.h>

namespace zSpace
{
	class zSPACE_API zParticle
	{
	public:
		zParticle();
		~zParticle();
	};


}

