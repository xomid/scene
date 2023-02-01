#include "UIDMinimum.h"
#include "ImageEffect.h"

int UIDMinimum::render(Sheet* srcImage, Sheet* dstImage, int blockLeft, int blockTop, int blockRight, int blockBottom)
{
	return ImageEffect::minimum(srcImage, dstImage,
		blockLeft, blockTop, blockRight, blockBottom);
}

