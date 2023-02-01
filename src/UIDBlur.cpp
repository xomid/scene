#include "UIDBlur.h"
#include "ImageEffect.h"

int UIDBlur::render(Sheet* srcImage, Sheet* dstImage, int blockLeft, int blockTop, int blockRight, int blockBottom)
{
	return ImageEffect::blur(srcImage, dstImage,
		blockLeft, blockTop, blockRight, blockBottom);
}

