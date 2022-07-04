#pragma once
#include <oui_sheet.h>
#include "BrightnessData.h"
#include "ContrastData.h"

enum class ColorChannel {
	None,
	RGB,
	Red,
	Green,
	Blue
};

struct ImageEffectBlob {
	bool bSet;
public:
	ImageEffectBlob();
};

struct AutoContrastBlob : public ImageEffectBlob {
public:
	unsigned char lookup[256];
	int init();
};

struct HistogramBlob : public ImageEffectBlob {
public:
	unsigned long grayHisto[256], bHisto[256], gHisto[256], rHisto[256], grayMax, rMax, gMax, bMax;
	HistogramBlob();
};

struct BrightnessContrastBlob : public ImageEffectBlob {
public:
	unsigned char blookup[256], clookup[256];
	int brightness, contrast;
	bool bLegacy;

	BrightnessContrastBlob();
	int init_lookups(int brightness, int contrast, bool bLegacy);
};


struct ChannelMixerBlob : public ImageEffectBlob {
public:
	unsigned char rlookup[256], glookup[256], blookup[256];
	bool bMono, bPreserveLum;
};

struct ColorBalanceBlob : public ImageEffectBlob {
public:
	unsigned char rlookup[256], glookup[256], blookup[256];
	bool bPreserveLum;
};

struct CurvesBlob : public ImageEffectBlob {
public:
	unsigned char vlookup[256], rlookup[256], glookup[256], blookup[256];
};

struct HSLBlob : public ImageEffectBlob {
public:
	unsigned char rlookup[256], glookup[256], blookup[256];
};

struct LevelsBlob : public ImageEffectBlob {
public:
	unsigned char rlookup[256], glookup[256], blookup[256];
};

class ImageEffect
{
public:

	static int get_histo(Sheet* srcImage, HistogramBlob* histogramBlob);

	// Adjustments effects
	// these mostly adjust color values as opposed to changing 
	// the pixel arrengment like filters (e.g. gussian blur) do

	static int auto_contrast(Sheet* srcImage, Sheet* dstImage, AutoContrastBlob* blob,
		int blockLeft, int blockTop, int blockRight, int blockBottom);

	static int brightness_contrast(Sheet* srcImage, Sheet* dstImage, BrightnessContrastBlob* blob, bool bLegacy,
		int brightness, int contrast, int blockLeft, int blockTop, int blockRight, int blockBottom);

	static int channel_mixer(Sheet* srcImage, Sheet* dstImage, ChannelMixerBlob* blob, bool bMono, bool bPreserveLuminosity,
		int red, int green, int blue, int blockLeft, int blockTop, int blockRight, int blockBottom);

	static int color_balance(Sheet* srcImage, Sheet* dstImage, ColorBalanceBlob* blob, bool bPreserveLuminosity,
		int red, int green, int blue, int blockLeft, int blockTop, int blockRight, int blockBottom);

	static int curves(Sheet* srcImage, Sheet* dstImage, CurvesBlob* blob,
		int blockLeft, int blockTop, int blockRight, int blockBottom);

	static int desaturate(Sheet* srcImage, Sheet* dstImage,
		int blockLeft, int blockTop, int blockRight, int blockBottom);

	static int hsl(Sheet* srcImage, Sheet* dstImage, HSLBlob* blob, bool bColoize, int hue, int saturation, int lightness,
		int blockLeft, int blockTop, int blockRight, int blockBottom);

	static int levels(Sheet* srcImage, Sheet* dstImage, LevelsBlob* blob, byte min, double mid, byte max, byte outputMin, byte outputMax,
		int blockLeft, int blockTop, int blockRight, int blockBottom);

	static int negative(Sheet* srcImage, Sheet* dstImage,
		int blockLeft, int blockTop, int blockRight, int blockBottom);

	static int posterize(Sheet* srcImage, Sheet* dstImage, byte levels,
		int blockLeft, int blockTop, int blockRight, int blockBottom);

	static int threshold(Sheet* srcImage, Sheet* dstImage, bool bMono, byte levels,
		int blockLeft, int blockTop, int blockRight, int blockBottom);

};

