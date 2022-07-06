#include "ImageEffect.h"

#define CLAMP3F(minValue, value, maxValue) fmin(fmax(value, minValue), maxValue)

#define VALIDATE_IMAGES()\
if (srcImage == NULL || dstImage == NULL || srcImage->is_useless() || dstImage->is_useless() || \
srcImage->w != srcImage->w || srcImage->h != srcImage->h || srcImage->nbpp != srcImage->nbpp || srcImage->pitch != srcImage->pitch)\
return 1;

#define DECLARE_VARIABLES() \
int x, y, w, h, p, right, bottom;\
pyte src, dst, d, s;\
w = srcImage->w;\
h = srcImage->h;\
p = srcImage->pitch;\
src = srcImage->data;\
dst = dstImage->data;\
right = w - 1;\
bottom = h - 1;

#define CLAMP_BLOCK()\
blockLeft = CLAMP3(0, blockLeft, w - 1);\
blockRight = CLAMP3(0, blockRight, w);\
blockTop = CLAMP3(0, blockTop, h - 1);\
blockBottom = CLAMP3(0, blockBottom, h);


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

		gray[i] = val;
	}

	return 0;
}


int ImageEffect::channel_mixer(Sheet* srcImage, Sheet* dstImage, ChannelMixerBlob* blob, bool bMono, bool bPreserveLuminosity,
	ChannelMixInfo red, ChannelMixInfo green, ChannelMixInfo blue, int blockLeft, int blockTop, int blockRight, int blockBottom) {
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


int ImageEffect::hsl(Sheet* srcImage, Sheet* dstImage, HSLBlob* blob, bool bColoize, int hue, int saturation, int lightness,
	int blockLeft, int blockTop, int blockRight, int blockBottom) {
	return 0;
}


int ImageEffect::levels(Sheet* srcImage, Sheet* dstImage, LevelsBlob* blob,
	ChannelLevelInfo rgbInfo, ChannelLevelInfo redInfo, ChannelLevelInfo, ChannelLevelInfo blueInfo,
	int blockLeft, int blockTop, int blockRight, int blockBottom) {
	return 0;
}


int ImageEffect::threshold(Sheet* srcImage, Sheet* dstImage, bool isMonochromatic, byte levels,
	int blockLeft, int blockTop, int blockRight, int blockBottom) {
	return 0;
}



