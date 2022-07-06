#include "ImageEffect.h"

BrightnessContrastBlob::BrightnessContrastBlob()
	: contrast(-300), // this must be out of range [-100, 150] for the func to initialize it
	brightness(-200), bLegacy(true), blookup{ 0 }, clookup{ 0 } {
}

int BrightnessContrastBlob::init_lookups(int brightness, int contrast, bool bLegacy) {
	brightness = CLAMP3(-100, brightness, 100);
	contrast = CLAMP3(-100, contrast, 100);

	if (this->brightness != brightness || this->contrast != contrast || this->bLegacy != bLegacy) {
		this->brightness = brightness;
		this->contrast = contrast;
		this->bLegacy = bLegacy;

		int x, y, n, i;
		double slope;

		if (bLegacy)
		{
			x = y = 0;

			if (contrast >= 0) {
				x = (abs(contrast) / 100.0) * 127.5;
				n = (2 * (127.5 - x));
				slope = 255.0 / n;
				n = x + n;

				for (i = x; i <= n; i++) {
					y = (i - x) * slope + 0.5;
					clookup[i] = y;
				}
				for (i = 0; i < x; i++) clookup[i] = 0;
				for (i = n + 1; i < 256; i++) clookup[i] = 255;
				for (i = 0; i < 256; i++) blookup[i] = CLAMP255(i + brightness);

			}
			else {
				y = (abs(contrast) / 100.0) * 127.5;
				slope = (2.0 * (127 - y)) / 255.0;
				for (i = 0; i < 256; i++) blookup[i] = y + (i * slope + 0.5);
				for (i = 0; i < 256; i++) clookup[i] = CLAMP255(i + brightness);
			}
		}
		else
		{
			brightness = CLAMP3(0, brightness + 150, 300);
			contrast = CLAMP3(0, contrast + 50, 150);
			memcpy(blookup, BrightnessData[brightness], 256);
			memcpy(clookup, ContrastData[contrast], 256);
		}
	}

	return 0;
}


int ImageEffect::brightness_contrast(Sheet* srcImage, Sheet* dstImage, BrightnessContrastBlob* blob, bool bLegacy,
	int brightness, int contrast, int blockLeft, int blockTop, int blockRight, int blockBottom)
{
	int x, y, w, h, p;
	pyte d, s;

	if (!blob || blob->init_lookups(brightness, contrast, bLegacy))
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
				*d++ = blob->clookup[blob->blookup[*s++]];
				*d++ = blob->clookup[blob->blookup[*s++]];
				*d++ = blob->clookup[blob->blookup[*s++]];
			}
		}
	}

	return 0;
}