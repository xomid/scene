#include "ImageEffect.h"

ImageEffectBlob::ImageEffectBlob() : bSet(false) {
}

int AutoContrastBlob::init() {
	if (bSet) return 0;

	bSet = true;
	for (int i = 0; i < 256; i++)
	{
		byte val = i;

		if (val >= 100 && val <= 235) val += 20;
		else if (val > 235) val = 255;
		else if (val <= 40 && val >= 20) val -= 20;
		else if (val < 20) val = 0;
		else if (val > 70 && val < 100) val += 10;
		else if (val < 60 && val > 40) val -= 10;

		lookup[i] = val;
	}

	return 0;
}

int ImageEffect::auto_contrast(Sheet* srcImage, Sheet* dstImage, AutoContrastBlob* blob,
	int blockLeft, int blockTop, int blockRight, int blockBottom) {

	int x, y, w, h, p;
	pyte d, s;

	if (!blob || blob->init())
		return 1;

	if (srcImage && dstImage) {
		w = srcImage->w;
		h = srcImage->h;
		p = srcImage->pitch;

		blockLeft = CLAMP3(0, blockLeft, w - 1);
		blockRight = CLAMP3(0, blockRight, w);
		blockTop = CLAMP3(0, blockTop, h - 1);
		blockBottom = CLAMP3(0, blockBottom, h);

		for (y = blockTop; y < blockBottom; ++y) {
			s = srcImage->data + y * p + blockLeft * 3;
			d = dstImage->data + y * p + blockLeft * 3;
			for (x = blockLeft; x < blockRight; ++x) {
				*d++ = blob->lookup[*s++];
				*d++ = blob->lookup[*s++];
				*d++ = blob->lookup[*s++];
			}
		}
	}

	return 0;
}


int ImageEffect::channel_mixer(Sheet* srcImage, Sheet* dstImage, ChannelMixerBlob* blob, bool bMono, bool bPreserveLuminosity,
	int red, int green, int blue, int blockLeft, int blockTop, int blockRight, int blockBottom) {
	return 0;
}


int ImageEffect::color_balance(Sheet* srcImage, Sheet* dstImage, ColorBalanceBlob* blob, bool bPreserveLuminosity,
	int red, int green, int blue, int blockLeft, int blockTop, int blockRight, int blockBottom) {
	return 0;
}


int ImageEffect::curves(Sheet* srcImage, Sheet* dstImage, CurvesBlob* blob,
	int blockLeft, int blockTop, int blockRight, int blockBottom) {

	int x, y, w, h, p;
	pyte d, s;

	if (!blob)
		return 1;

	if (srcImage && dstImage) {
		w = srcImage->w;
		h = srcImage->h;
		p = srcImage->pitch;

		blockLeft = CLAMP3(0, blockLeft, w - 1);
		blockRight = CLAMP3(0, blockRight, w);
		blockTop = CLAMP3(0, blockTop, h - 1);
		blockBottom = CLAMP3(0, blockBottom, h);

		for (y = blockTop; y < blockBottom; ++y) {
			s = srcImage->data + y * p + blockLeft * 3;
			d = dstImage->data + y * p + blockLeft * 3;
			for (x = blockLeft; x < blockRight; ++x) {
				*d++ = blob->vlookup[blob->blookup[*s++]];
				*d++ = blob->vlookup[blob->glookup[*s++]];
				*d++ = blob->vlookup[blob->rlookup[*s++]];
			}
		}
	}

	return 0;
}


int ImageEffect::desaturate(Sheet* srcImage, Sheet* dstImage,
	int blockLeft, int blockTop, int blockRight, int blockBottom) {
	return 0;
}


int ImageEffect::hsl(Sheet* srcImage, Sheet* dstImage, HSLBlob* blob, bool bColoize, int hue, int saturation, int lightness,
	int blockLeft, int blockTop, int blockRight, int blockBottom) {
	return 0;
}


int ImageEffect::levels(Sheet* srcImage, Sheet* dstImage, LevelsBlob* blob, byte min, double mid, byte max, byte outputMin, byte outputMax,
	int blockLeft, int blockTop, int blockRight, int blockBottom) {
	return 0;
}


int ImageEffect::negative(Sheet* srcImage, Sheet* dstImage,
	int blockLeft, int blockTop, int blockRight, int blockBottom) {
	return 0;
}


int ImageEffect::posterize(Sheet* srcImage, Sheet* dstImage, byte levels,
	int blockLeft, int blockTop, int blockRight, int blockBottom) {
	return 0;
}


int ImageEffect::threshold(Sheet* srcImage, Sheet* dstImage, bool bMono, byte levels,
	int blockLeft, int blockTop, int blockRight, int blockBottom) {
	return 0;
}
