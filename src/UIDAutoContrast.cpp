#include "UIDAutoContrast.h"
#include "ImageEffect.h"

int UIDAutoContrast::render(Sheet* srcImage, Sheet* dstImage, int blockLeft, int blockTop, int blockRight, int blockBottom)
{
	return ImageEffect::auto_contrast(srcImage, dstImage, &blob,
		blockLeft, blockTop, blockRight, blockBottom);
}

