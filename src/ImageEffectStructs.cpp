#include "ImageEffect.h"


ImageEffectBlob::ImageEffectBlob() : isSet(false) {
}

int AutoContrastBlob::init() {
	if (!isSet) {
		isSet = true;
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
	}

	return IMAGE_EFFECT_RESULT_OK;
}

int PosterizeBlob::init(byte threshold) {
	if (!isSet) {
		for (int i = 0; i < 256; i++)
			gray[i] = 255 * (threshold * i / 256) / (threshold - 1);
		isSet = true;
	}

	return IMAGE_EFFECT_RESULT_OK;
}

int GlowBlob::init(int& softness, int& brightness, int& contrast) {

	softness = CLAMP3(1, softness, 16);
	contrast = CLAMP3(-100, contrast, 100);
	brightness = CLAMP3(-100, brightness, 100);

	if (!isSet || this->softness != softness || this->brightness != brightness || this->contrast != contrast) {
		int i;
		int size = 1 + (softness * 2);

		for (i = 0; i <= softness; ++i)
		{
			weights[i] = 16 * (i + 1);
			weights[size - i - 1] = weights[i];
		}

		double D = (100 + contrast) / 100.0;

		for (i = 0; i < 256; i++)
			gray[i] = CLAMP255((i - 128) * D + (brightness + 128) + 0.5);

		isSet = true;
		this->softness == softness;
		this->brightness == brightness;
		this->contrast == contrast;
	}

	return IMAGE_EFFECT_RESULT_OK;
}


int MedianBlob::init(size_t& radius, size_t& percentile) {

	radius = CLAMP3(0, radius, 200);
	percentile = CLAMP3(0, percentile, 100);

	if (!isSet || this->radius != radius || this->percentile != percentile) {
		auto rad = radius;
		auto precent = percentile;
		memset(leadingEdgeX, 0, (rad + 1) * sizeof(int));
		auto cutoff = ((rad * 2 + 1) * (rad * 2 + 1) + 2) / 4;
		int u, v;

		for (v = 0; v <= rad; ++v)
			for (u = 0; u <= rad; ++u)
				if (u * u + v * v <= cutoff)
					leadingEdgeX[v] = u;

		isSet = true;
		this->radius == radius;
		this->percentile == percentile;
	}

	return IMAGE_EFFECT_RESULT_OK;
}