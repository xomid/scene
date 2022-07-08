#include "ImageEffect.h"

int BrightnessContrastBlob::init(int brightness, int contrast, bool bLegacy) {
	brightness = CLAMP3(-100, brightness, 100);
	contrast = CLAMP3(-100, contrast, 100);

	if (!isSet || this->brightness != brightness || this->contrast != contrast || this->bLegacy != bLegacy) {

		int x, y, n, i;
		double slope;

		/*
		for (i = 0; i < 256; i++) 
					gray[i] = CLAMP255(i + brightness);

				for (i = 0; i < 256; i++) {
					int b = gray[i];
					int c = 0;
					if (b >= x && b <= n) c = CLAMP255(int((b - x) * slope + 0.5));
					else c = 255;
					gray[i] = c;
				}

		for (i = 0; i < 256; i++)
			br[i] = CLAMP255(i + brightness);

		for (i = 0; i < 256; i++) {
			int c = 0;
			if (i >= x && i <= n) c = CLAMP255(int((i - x) * slope + 0.5));
			else c = 255;
			co[i] = c;
		}

		for (i = 0; i < 256; i++)
			gray[i] = co[br[i]];
		
		*/

		if (bLegacy)
		{
			x = y = 0;

			for (i = 0; i < 256; i++) {
				int b = 0;
				int c = 0;

				if (contrast >= 0) {
					b = CLAMP255(i + brightness);

					x = (abs(contrast) / 100.0) * 127.5;
					n = (2 * (127.5 - x));
					slope = 255.0 / n;
					n = x + n;

					c = 0;
					if (b >= x && b <= n) c = CLAMP255(int((b - x) * slope + 0.5));
					else if (b > n) c = 255;
				}
				else {
					y = (abs(contrast) / 100.0) * 127.5;
					slope = (2.0 * (127 - y)) / 255.0;
					b = y + (i * slope + 0.5);
					c = CLAMP255(b + brightness);
				}

				gray[i] = c;
			}
		}
		else
		{
			brightness = CLAMP3(0, brightness + 150, 300);
			contrast = CLAMP3(0, contrast + 50, 150);
			auto contrastLookup = ContrastData[contrast];
			auto brightnessLookup = BrightnessData[brightness];
			for (i = 0; i < 256; i++)
				gray[i] = contrastLookup[brightnessLookup[i]];
		}

		isSet = true;
		this->bLegacy = bLegacy;
		this->brightness = brightness;
		this->contrast = contrast;
	}

	return 0;
}


int ImageEffect::brightness_contrast(Sheet* srcImage, Sheet* dstImage, BrightnessContrastBlob* blob, bool bLegacy,
	int brightness, int contrast, int blockLeft, int blockTop, int blockRight, int blockBottom)
{
	if (!blob || blob->init(brightness, contrast, bLegacy))
		return IMAGE_EFFECT_RESULT_ERROR;

	return apply_lookup_gray(srcImage, dstImage, blob, blockLeft, blockTop, blockRight, blockBottom);
}