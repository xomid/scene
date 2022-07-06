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

enum class NoiseType {
	Gussian, Uniform
};

enum class CrystalizeMode {
	Random, Square, Hexagonal, Octagonal, Triangular
};

enum class BlurMode {
	Spin, Zoom
};

enum class RippleMode {
	Sinuous, Triangle
};

enum class SmearMode {
	Crosses, Lines, Circles, Squares
};


struct RGBGrayLookup {
	unsigned char red[256], green[256], blue[256], gray[256];
};

struct GrayLookup {
	unsigned char gray[256];
};

struct RGBLookup {
	unsigned char red[256], green[256], blue[256];
};


struct ImageEffectBlob {
	bool bSet;
public:
	ImageEffectBlob();
};

struct AutoContrastBlob : public GrayLookup, public ImageEffectBlob {
public:
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

struct ChannelMixInfo {
	// red, green, blue [-200, 200]
	int red, green, blue;
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

struct ChannelLevelInfo {
	byte minRange, maxRange;
	double gamma;
	byte outputMin, outputMax;
};

struct LevelsBlob : public ImageEffectBlob {
public:
	unsigned char rlookup[256], glookup[256], blookup[256];
};

struct PosterizeBlob : public GrayLookup, public ImageEffectBlob {
public:
	int init(byte threshold);
};








struct GainBlob : public GrayLookup, public ImageEffectBlob {
public:
	int init(double gain, double bias);
};


class ImageEffect
{
public:

	static int get_histo(Sheet* srcImage, HistogramBlob* histogramBlob);

	static int apply_lookup_all(Sheet* srcImage, Sheet* dstImage, RGBGrayLookup* lookup,
		int blockLeft, int blockTop, int blockRight, int blockBottom);

	static int apply_lookup_rgb(Sheet* srcImage, Sheet* dstImage, RGBLookup* lookup,
		int blockLeft, int blockTop, int blockRight, int blockBottom);

	static int apply_lookup_gray(Sheet* srcImage, Sheet* dstImage, GrayLookup* lookup,
		int blockLeft, int blockTop, int blockRight, int blockBottom);

	static int convolution(Sheet* srcImage, Sheet* dstImage, float* matrix, size_t rowCount, size_t columnCount,
		int blockLeft, int blockTop, int blockRight, int blockBottom);

	// Adjustments effects
	// these mostly adjust color values as opposed to changing 
	// the pixel arrengment like filters (e.g. gussian blur) do

	static int auto_contrast(Sheet* srcImage, Sheet* dstImage, AutoContrastBlob* blob,
		int blockLeft, int blockTop, int blockRight, int blockBottom);


	// if legacy
	// brightness contrast [-100, 100]
	// else 
	// brightness [-150, 150]
	// contrast [-50, 100]
	static int brightness_contrast(Sheet* srcImage, Sheet* dstImage, BrightnessContrastBlob* blob, bool isLegacy,
		int brightness, int contrast, int blockLeft, int blockTop, int blockRight, int blockBottom);

	// for each channel, red, green, blue [-200, 200]
	static int channel_mixer(Sheet* srcImage, Sheet* dstImage, ChannelMixerBlob* blob, bool isMonochromatic, bool shouldPreserveLuminosity,
		ChannelMixInfo red, ChannelMixInfo green, ChannelMixInfo blue, int blockLeft, int blockTop, int blockRight, int blockBottom);

	// red, green, blue [-100, 100]
	static int color_balance(Sheet* srcImage, Sheet* dstImage, ColorBalanceBlob* blob, bool shouldPreserveLuminosity,
		int red, int green, int blue, int blockLeft, int blockTop, int blockRight, int blockBottom);

	static int curves(Sheet* srcImage, Sheet* dstImage, CurvesBlob* blob,
		int blockLeft, int blockTop, int blockRight, int blockBottom);

	static int desaturate(Sheet* srcImage, Sheet* dstImage,
		int blockLeft, int blockTop, int blockRight, int blockBottom);

	// if shouldColorize
	// hue [0, 360]
	// saturation [0, 100]
	// lightness [-100, 100]
	// else
	// hue [-180, 180]
	// saturation, lightness [-100, 100]
	static int hsl(Sheet* srcImage, Sheet* dstImage, HSLBlob* blob, bool shouldColorize, int hue, int saturation, int lightness,
		int blockLeft, int blockTop, int blockRight, int blockBottom);

	static int levels(Sheet* srcImage, Sheet* dstImage, LevelsBlob* blob, 
		ChannelLevelInfo rgbInfo, ChannelLevelInfo redInfo, ChannelLevelInfo, ChannelLevelInfo blueInfo,
		int blockLeft, int blockTop, int blockRight, int blockBottom);

	static int negative(Sheet* srcImage, Sheet* dstImage,
		int blockLeft, int blockTop, int blockRight, int blockBottom);

	// levels [2, 255]
	static int posterize(Sheet* srcImage, Sheet* dstImage, PosterizeBlob* blob, byte levels,
		int blockLeft, int blockTop, int blockRight, int blockBottom);

	// levels [0, 255]
	static int threshold(Sheet* srcImage, Sheet* dstImage, bool isMonochromatic, byte levels,
		int blockLeft, int blockTop, int blockRight, int blockBottom);

	// filter effects
	// these effects may change the whole structure and color of pixels
	// as opposed to what adjustment effects do
	
	
	// amount [0.1, 400]
	static int add_noise(Sheet* srcImage, Sheet* dstImage, NoiseType noiseType, double amount,
		bool isMonochromatic, int blockLeft, int blockTop, int blockRight, int blockBottom);

	static int blur(Sheet* srcImage, Sheet* dstImage, 
		int blockLeft, int blockTop, int blockRight, int blockBottom);

	// amount [-2, 1]
	// pivotX, pivotY [0, 1]
	static int bulge(Sheet* srcImage, Sheet* dstImage, double amount, bool shouldStretch, double pivotX, double pivotY,
		int blockLeft, int blockTop, int blockRight, int blockBottom);

	static int bump(Sheet* srcImage, Sheet* dstImage,
		int blockLeft, int blockTop, int blockRight, int blockBottom);

	// edgeThickness, randomness [0, 1]
	// distancePower [0.01, 10]
	// scale [0.01, 256]

	static int crystalize(Sheet* srcImage, Sheet* dstImage, CrystalizeMode crystalizeMode, bool shouldFadeEdges,
		double edgeThickness, double randomness, double distancePower, double scale,
		int blockLeft, int blockTop, int blockRight, int blockBottom);

	// iteration [1, 20]
	static int despeckle(Sheet* srcImage, Sheet* dstImage, size_t iteration,
		int blockLeft, int blockTop, int blockRight, int blockBottom);

	// radius [0, 254]
	static int gussian_blur(Sheet* srcImage, Sheet* dstImage, size_t radius,
		int blockLeft, int blockTop, int blockRight, int blockBottom);

	// gian, bias [0, 1]
	static int gain(Sheet* srcImage, Sheet* dstImage, GainBlob* blob, double gain, double bias,
		int blockLeft, int blockTop, int blockRight, int blockBottom);

	// softness [1, 16]
	// contrast, brightness [-100, 100]
	static int glow(Sheet* srcImage, Sheet* dstImage, int softness, int brightness, int contrast,
		int blockLeft, int blockTop, int blockRight, int blockBottom);

	// telorance, scale [0, 100]
	static int marble(Sheet* srcImage, Sheet* dstImage, double telorance, double scale,
		int blockLeft, int blockTop, int blockRight, int blockBottom);

	static int maximum(Sheet* srcImage, Sheet* dstImage,
		int blockLeft, int blockTop, int blockRight, int blockBottom);

	// radius [1, 200]
	// percentile [0, 100]
	static int median(Sheet* srcImage, Sheet* dstImage, size_t radius, size_t percentile,
		int blockLeft, int blockTop, int blockRight, int blockBottom);

	static int minimum(Sheet* srcImage, Sheet* dstImage,
		int blockLeft, int blockTop, int blockRight, int blockBottom);

	// size [3, 2000]
	// angle [-PI, PI]
	static int motion_blur(Sheet* srcImage, Sheet* dstImage, size_t size, double angle,
		int blockLeft, int blockTop, int blockRight, int blockBottom);

	// brushSize [1, 100]
	// coareness [3, 255]
	static int old_paint(Sheet* srcImage, Sheet* dstImage, size_t brushSize, size_t coareness,
		int blockLeft, int blockTop, int blockRight, int blockBottom);

	// radius [1, 200]
	// intensity [0, 100]
	static int outline(Sheet* srcImage, Sheet* dstImage, size_t radius, size_t intensity,
		int blockLeft, int blockTop, int blockRight, int blockBottom);

	// brushSize [1, 50]
	// range [-20, 20]
	static int pencil_sketch(Sheet* srcImage, Sheet* dstImage, size_t brushSize, size_t range, 
		int blockLeft, int blockTop, int blockRight, int blockBottom);

	// cellSize [1, 200]
	static int pixelate(Sheet* srcImage, Sheet* dstImage, size_t cellSize,
		int blockLeft, int blockTop, int blockRight, int blockBottom);

	// amount [1, 100]
	// pivotX, pivotY [0, 1]
	static int radial_blur(Sheet* srcImage, Sheet* dstImage, BlurMode blurMode, double amount, double pivotX, double pivotY,
		int blockLeft, int blockTop, int blockRight, int blockBottom);

	// amount [1, 200]
	static int random_jitter(Sheet* srcImage, Sheet* dstImage, size_t amount,
		int blockLeft, int blockTop, int blockRight, int blockBottom);

	static int reduce_noise(Sheet* srcImage, Sheet* dstImage,
		int blockLeft, int blockTop, int blockRight, int blockBottom);

	// waveLength [1, 200]
	// amplitude [1, 100]
	static int ripple(Sheet* srcImage, Sheet* dstImage, RippleMode rippleMode, double waveLength, double amplitude,
		int blockLeft, int blockTop, int blockRight, int blockBottom);

	static int sharpen(Sheet* srcImage, Sheet* dstImage,
		int blockLeft, int blockTop, int blockRight, int blockBottom);

	// radius [3, 100]
	// threshold [0, 255]
	static int smart_blur(Sheet* srcImage, Sheet* dstImage, size_t radius, size_t threshold,
		int blockLeft, int blockTop, int blockRight, int blockBottom);

	// amount [1, 100]
	// mix, density [0, 1]
	static int smear(Sheet* srcImage, Sheet* dstImage, SmearMode smearMode, double amount, double mix, double density, double angle,
		int blockLeft, int blockTop, int blockRight, int blockBottom);

	// softness [0, 10]
	// warmness [0, 40]
	// brightness [-100, 100]
	static int soft_portrait(Sheet* srcImage, Sheet* dstImage, size_t softness, size_t warmness, int brightness,
		int blockLeft, int blockTop, int blockRight, int blockBottom);

	// radius, threshold [0, 100]
	static int stamp(Sheet* srcImage, Sheet* dstImage, size_t radius, double threshold,
		int blockLeft, int blockTop, int blockRight, int blockBottom);

	// radius [1, 100]
	// level [2, 255]
	static int surface_blur(Sheet* srcImage, Sheet* dstImage, double radius, double level,
		int blockLeft, int blockTop, int blockRight, int blockBottom);

	// angle [-PI/4, PI/4]
	// pivotX, pivotY [0, 1]
	static int swirl(Sheet* srcImage, Sheet* dstImage, double angle, bool shouldStretch, double pivotX, double pivotY,
		int blockLeft, int blockTop, int blockRight, int blockBottom);

	// size [2, 1000]
	// amount [-20, 20]
	// angle [-PI, PI]
	// pivotX, pivotY [0, 1]
	static int tile_glass(Sheet* srcImage, Sheet* dstImage, double size, double amount, double angle, double pivotX, double pivotY,
		int blockLeft, int blockTop, int blockRight, int blockBottom);

	static int unsharp(Sheet* srcImage, Sheet* dstImage,
		int blockLeft, int blockTop, int blockRight, int blockBottom);

	// telorance [0, 1000]
	// scale [0, 100]
	// pivotX, pivotY [0, 1]
	static int water(Sheet* srcImage, Sheet* dstImage, double telorance, double scale, double pivotX, double pivotY,
		int blockLeft, int blockTop, int blockRight, int blockBottom);

	// waveLength [2, 200]
	// amplitude [1, 50]
	// pivotX, pivotY [0, 1]
	static int wave(Sheet* srcImage, Sheet* dstImage, double waveLength, double amplitude, double pivotX, double pivotY,
		int blockLeft, int blockTop, int blockRight, int blockBottom);
};

