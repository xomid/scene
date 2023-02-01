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



static double biasf(double a, double b)
{
	//		return (double)Math.pow(a, Math.log(b) / Math.log(0.5));
	return a / ((1. / b - 2.) * (1. - a) + 1.);
}

static double gainf(double a, double b) {
	/*
	double p = (double)Math.log(1.0 - b) / (double)Math.log(0.5);

	if (a < .001)
	return 0.0f;
	else if (a > .999)
	return 1.0f;
	if (a < 0.5)
	return (double)Math.pow(2 * a, p) / 2;
	else
	return 1.0f - (double)Math.pow(2 * (1. - a), p) / 2;
	*/
	double c = (1.0 / b - 2.0) * (1.0 - 2.0 * a);
	if (a < 0.5)
		return a / (c + 1.0);
	else
		return (c - a) / (c - 1.0);
}


int ImageEffect::apply_lookup_all(Sheet* srcImage, Sheet* dstImage, RGBGrayLookup* lookup,
	int blockLeft, int blockTop, int blockRight, int blockBottom) {

	VALIDATE_IMAGES();
	DECLARE_VARIABLES();
	CLAMP_BLOCK();

	pyte red, green, blue, gray;
	gray = lookup->gray;
	red = lookup->red;
	green = lookup->green;
	blue = lookup->blue;

	for (y = blockTop; y < blockBottom; ++y) {
		s = src + y * p + blockLeft * 3;
		d = dst + y * p + blockLeft * 3;
		for (x = blockLeft; x < blockRight; ++x) {
			*d++ = gray[blue[*s++]];
			*d++ = gray[green[*s++]];
			*d++ = gray[red[*s++]];
		}
	}

	return IMAGE_EFFECT_RESULT_OK;
}

int ImageEffect::apply_lookup_rgb(Sheet* srcImage, Sheet* dstImage, RGBLookup* lookup,
	int blockLeft, int blockTop, int blockRight, int blockBottom) {

	VALIDATE_IMAGES();
	DECLARE_VARIABLES();
	CLAMP_BLOCK();

	pyte red, green, blue;
	red = lookup->red;
	green = lookup->green;
	blue = lookup->blue;

	for (y = blockTop; y < blockBottom; ++y) {
		s = src + y * p + blockLeft * 3;
		d = dst + y * p + blockLeft * 3;
		for (x = blockLeft; x < blockRight; ++x) {
			*d++ = blue[*s++];
			*d++ = green[*s++];
			*d++ = red[*s++];
		}
	}

	return IMAGE_EFFECT_RESULT_OK;
}

int ImageEffect::apply_lookup_gray(Sheet* srcImage, Sheet* dstImage, GrayLookup* lookup,
	int blockLeft, int blockTop, int blockRight, int blockBottom) {

	VALIDATE_IMAGES();
	DECLARE_VARIABLES();
	CLAMP_BLOCK();

	auto gray = lookup->gray;
	for (y = blockTop; y < blockBottom; ++y) {
		s = src + y * p + blockLeft * 3;
		d = dst + y * p + blockLeft * 3;
		for (x = blockLeft; x < blockRight; ++x) {
			*d++ = gray[*s++];
			*d++ = gray[*s++];
			*d++ = gray[*s++];
		}
	}

	return IMAGE_EFFECT_RESULT_OK;
}

int ImageEffect::auto_contrast(Sheet* srcImage, Sheet* dstImage, AutoContrastBlob* blob,
	int blockLeft, int blockTop, int blockRight, int blockBottom) {

	if (!blob || blob->init())
		return 1;

	return apply_lookup_gray(srcImage, dstImage, blob, blockLeft, blockTop, blockRight, blockBottom);
}

int ImageEffect::negative(Sheet* srcImage, Sheet* dstImage,
	int blockLeft, int blockTop, int blockRight, int blockBottom) {

	VALIDATE_IMAGES();
	DECLARE_VARIABLES();
	CLAMP_BLOCK();

	for (y = blockTop; y < blockBottom; ++y) {
		s = src + y * p + blockLeft * 3;
		d = dst + y * p + blockLeft * 3;
		for (x = blockLeft; x < blockRight; ++x) {
			*d = 0xff - *d; ++d;
			*d = 0xff - *d; ++d;
			*d = 0xff - *d; ++d;
		}
	}

	return IMAGE_EFFECT_RESULT_OK;
}

int ImageEffect::posterize(Sheet* srcImage, Sheet* dstImage, PosterizeBlob* blob, byte threshold,
	int blockLeft, int blockTop, int blockRight, int blockBottom)
{
	if (blob == NULL || blob->init(threshold)) return 1;
	return apply_lookup_gray(srcImage, dstImage, blob, blockLeft, blockTop, blockRight, blockBottom);
}

int GainBlob::init(double gain, double bias) {
	gain = CLAMP3F(0., gain, 1.);
	bias = CLAMP3F(0., bias, 1.);

	if (!isSet || are_not_equal(this->gain, gain) || are_not_equal(this->bias, bias)) {
		double f;
		for (int i = 0; i < 256; i++)
		{
			f = i / 255.;
			f = gainf(f, gain);
			f = biasf(f, bias);
			gray[i] = CLAMP255(int(f * 255.));
		}
		isSet = true;
		this->gain = gain;
		this->bias = bias;

		std::cout << "GainBlob is being recalculated.\n";
	}
	return IMAGE_EFFECT_RESULT_OK;
}

int ImageEffect::gain(Sheet* srcImage, Sheet* dstImage, GainBlob* blob, double gain, double bias,
	int blockLeft, int blockTop, int blockRight, int blockBottom) {

	if (!blob || blob->init(gain, bias))
		return 1;

	return apply_lookup_gray(srcImage, dstImage, blob, blockLeft, blockTop, blockRight, blockBottom);
}

