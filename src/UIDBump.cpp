#include "UIDBump.h"
#include "ImageEffect.h"

int UIDBump::render(Sheet* srcImage, Sheet* dstImage, int blockLeft, int blockTop, int blockRight, int blockBottom)
{
	return ImageEffect::bump(srcImage, dstImage,
		blockLeft, blockTop, blockRight, blockBottom);
}

