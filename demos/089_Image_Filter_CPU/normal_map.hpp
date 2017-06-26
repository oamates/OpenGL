#ifndef _normal_map_included_87123455329187423561347512837562985762035610251246
#define _normal_map_included_87123455329187423561347512837562985762035610251246

#include "nvimage.h"
#include "image.hpp"

//#include "nvmath/Vector.h"


namespace nv
{
	struct image;

	enum NormalMapFilter
	{
		NormalMapFilter_Sobel3x3,	// fine detail
		NormalMapFilter_Sobel5x5,	// medium detail
		NormalMapFilter_Sobel7x7,	// large detail
		NormalMapFilter_Sobel9x9,	// very large
	};

	fimage_t* createNormalMap(const image_t* img, fimage_t::WrapMode wm, Vector4::Arg heightWeights, NormalMapFilter filter = NormalMapFilter_Sobel3x3);
	fimage_t* createNormalMap(const image_t* img, fimage_t::WrapMode wm, Vector4::Arg heightWeights, Vector4::Arg filterWeights);
	fimage_t* createNormalMap(const fimage_t* img, fimage_t::WrapMode wm, Vector4::Arg filterWeights);

	void normalizeNormalMap(fimage_t* img);

} // nv namespace

#endif // _normal_map_included_87123455329187423561347512837562985762035610251246
