#include "UIDDesaturate.h"
#include "ImageEffect.h"

int UIDDesaturate::render(Sheet* srcImage, Sheet* dstImage, int blockLeft, int blockTop, int blockRight, int blockBottom)
{
	return ImageEffect::desaturate(srcImage, dstImage,
		blockLeft, blockTop, blockRight, blockBottom);
}

