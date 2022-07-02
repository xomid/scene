#pragma once
#include <oui_sheet.h>
#include "BrightnessData.h"
#include "ContrastData.h"

struct ImageEffectBlob {
	bool bSet;
public:
	ImageEffectBlob();
};

struct BrightnessContrastBlob : public ImageEffectBlob {
public:
	unsigned char blookup[256], clookup[256];
	int brightness, contrast;
	bool bLegacy;

	BrightnessContrastBlob();
	int init_lookups(int brightness, int contrast, bool bLegacy);
};

class ImageEffect
{
public:
	// Adjustments effects
	static int brightness_contrast(Sheet* srcImage, Sheet* dstImage, BrightnessContrastBlob* blob, bool bLegacy,
		int brightness, int contrast, int blockLeft, int blockTop, int blockRight, int blockBottom);
};

