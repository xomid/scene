#include "UIDUnsharp.h"
#include "ImageEffect.h"

int UIDUnsharp::render(Sheet* srcImage, Sheet* dstImage, int blockLeft, int blockTop, int blockRight, int blockBottom)
{
	return ImageEffect::unsharp(srcImage, dstImage,
		blockLeft, blockTop, blockRight, blockBottom);
}

