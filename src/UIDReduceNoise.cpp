#include "UIDReduceNoise.h"
#include "ImageEffect.h"

int UIDReduceNoise::render(Sheet* srcImage, Sheet* dstImage, int blockLeft, int blockTop, int blockRight, int blockBottom)
{
	return ImageEffect::reduce_noise(srcImage, dstImage,
		blockLeft, blockTop, blockRight, blockBottom);
}

