#include "UIDMaximum.h"
#include "ImageEffect.h"

int UIDMaximum::render(Sheet* srcImage, Sheet* dstImage, int blockLeft, int blockTop, int blockRight, int blockBottom)
{
	return ImageEffect::maximum(srcImage, dstImage,
		blockLeft, blockTop, blockRight, blockBottom);
}

