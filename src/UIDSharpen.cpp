#include "UIDSharpen.h"
#include "ImageEffect.h"

int UIDSharpen::render(Sheet* srcImage, Sheet* dstImage, int blockLeft, int blockTop, int blockRight, int blockBottom)
{
	return ImageEffect::sharpen(srcImage, dstImage,
		blockLeft, blockTop, blockRight, blockBottom);
}

