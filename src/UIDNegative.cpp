#include "UIDNegative.h"
#include "ImageEffect.h"

int UIDNegative::render(Sheet* srcImage, Sheet* dstImage, int blockLeft, int blockTop, int blockRight, int blockBottom)
{
	return ImageEffect::negative(srcImage, dstImage,
		blockLeft, blockTop, blockRight, blockBottom);
}

