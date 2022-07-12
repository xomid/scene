#pragma once
#include <oui_sheet.h>
#include "BrightnessData.h"
#include "ContrastData.h"

#define IMAGE_EFFECT_RESULT_OK 0
#define IMAGE_EFFECT_RESULT_ERROR 1
#define IMAGE_EFFECT_RESULT_WHOLE_IMAGE 2

bool are_not_equal(double a, double b);

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

enum class RadialBlurMode {
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

typedef RGBGrayLookup CurvesBlob;

struct ImageEffectBlob {
	bool isSet;
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

struct BrightnessContrastBlob : public GrayLookup, public ImageEffectBlob {
	int brightness, contrast;
	bool bLegacy;
public:
	int init(int brightness, int contrast, bool bLegacy);
};

struct ChannelMixInfo {
public:
	// red, green, blue [-2, 2]
	double red, green, blue;
	void reset();
	void set(double red, double green, double blue);
	bool operator!=(const ChannelMixInfo& obj);
};

struct ChannelLevelInfo {
public:
	double gamma;
	byte minRange, maxRange, outputMin, outputMax, lookup[256];
	bool operator!=(const ChannelLevelInfo& obj);
	void fill_lookup(ChannelLevelInfo* rgbInfo);
};

struct LevelsBlob : public ImageEffectBlob {
	ChannelLevelInfo rgbInfo, redInfo, greenInfo, blueInfo;
public:
	byte vslookup[256], rslookup[256], gslookup[256], bslookup[256];
	int init(ChannelLevelInfo& rgbInfo, ChannelLevelInfo& redInfo, ChannelLevelInfo& greenInfo, ChannelLevelInfo& blueInfo);
};


struct ColorBalanceBlob : public ImageEffectBlob {
public:
	unsigned char rlookup[256], glookup[256], blookup[256];
	bool bPreserveLum;
};

struct HSLBlob : public RGBLookup, public ImageEffectBlob {
private:
	bool shouldColorize;
	int _hue, _saturation, _lightness;
public:
	byte finalRedLookup[256], finalGreenLookup[256], finalBlueLookup[256];
	int hue, saturation, lightness;
	int init(bool shouldColorize, int& hue, int& saturation, int& lightness);
};


struct PosterizeBlob : public GrayLookup, public ImageEffectBlob {
	byte threshold;
public:
	int init(byte threshold);
};

struct GlowBlob : public GrayLookup, public ImageEffectBlob {
private:
	int softness, brightness, contrast;
public:
	int weights[2001];
	byte localStore[192096], stack[99];
	int init(int& softness, int& brightness, int& contrast);
};

typedef struct MedianBlob : public GrayLookup, public ImageEffectBlob {
private:
	size_t radius, intensity;
public:
	int leadingEdgeX[256], ha[256], hr[256], hg[256], hb[256];
	int init(size_t& radius, size_t& intensity);
} OutlineBlob;

struct GainBlob : public GrayLookup, public ImageEffectBlob {
	double gain, bias;
public:
	int init(double gain, double bias);
};

struct YPoint
{
	int index;
	float x, y;
	float dx, dy;
	float cubeX, cubeY;
	float distance;
};

struct CrystalizeBlob : public ImageEffectBlob
{
public:

	float edgeThickness = 0.35f;
	bool fadeEdges;
	float scale = 9.0f;
	float stretch = 1.0f;
	float amount = 1.0f;
	float turbulence = 1.0f;
	float distancePower = 3.0f;
	float randomness = 1.0;
	CrystalizeMode mode = CrystalizeMode::Hexagonal;
	byte probabilities[8096];
	float angleCoefficient;
	float gradientCoefficient;

	static constexpr size_t B = 0x100;
	static constexpr size_t BM = 0xff;
	static constexpr size_t N = 0x1000;

	int p[B + B + 2];
	float g3[B + B + 2][3];
	float g2[B + B + 2][2];
	float g1[B + B + 2];
	bool start;
	YPoint results[3];

	int init();

	float noise2(float x, float y);

	float checkCube(float x, float y, int cubeX, int cubeY);
};

struct MarbleBlob : public ImageEffectBlob {
	double telorance, scale;
public:
	int pp[514];
	float g3[514][3];
	float g2[514][2];
	float g1[514];
	float sinTable[256], cosTable[256], out[2];
	int init(double& telorance, double& scale);
};

struct OldPaintBlob : public ImageEffectBlob {
public:
	int localStore[20480];
};

struct PencilSketchBlob : public GrayLookup, public ImageEffectBlob {
private:
	size_t brushSize;
	int range;
public:
	int weights[100 * 2 + 1];
	byte localStore[19296];
	int init(size_t& brushSize, int& range);
};

struct ThresholdBlob : public GrayLookup, public ImageEffectBlob {
private:
	byte threshold;
	bool isMonochromatic;
public:
	int init(byte threshold, bool isMonochromatic);
};

struct SurfaceBlurBlob : public ImageEffectBlob {
private:
	double radius, level;
public:
	float mat[101];
	unsigned short imat[202];
	int init(double& radius, double& level);
};

struct StampBlob : public ImageEffectBlob {
private:
	size_t radius;
	double threshold;
public:
	int weights[100 * 2 + 1];
	byte localStore[19296];
	int init(size_t& radius, double& threshold);
};

struct SoftPortraitBlob : public GrayLookup, public ImageEffectBlob {
private:
	size_t softness, warmness;
	int brightness;
public:
	int weights[41];
	byte localStore[41 * 6 * 16];
	int init(size_t& softness, size_t& warmness, int& brightness);
};

struct SmartBlurBlob : public ImageEffectBlob {
private:
	size_t radius, threshold;
public:
	float kernel[201];
	int init(size_t& radius, size_t& threshold);
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
	static int channel_mixer(Sheet* srcImage, Sheet* dstImage,
		ChannelMixInfo& gray, ChannelMixInfo& red, ChannelMixInfo& green, ChannelMixInfo& blue,
		bool isMonochromatic, bool shouldPreserveLuminosity,
		int blockLeft, int blockTop, int blockRight, int blockBottom);

	// red, green, blue [-100, 100]
	static int color_balance(Sheet* srcImage, Sheet* dstImage, bool shouldPreserveLuminosity,
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
		ChannelLevelInfo& rgbInfo, ChannelLevelInfo& redInfo, ChannelLevelInfo& greenInfo, ChannelLevelInfo& blueInfo,
		int blockLeft, int blockTop, int blockRight, int blockBottom);

	static int negative(Sheet* srcImage, Sheet* dstImage,
		int blockLeft, int blockTop, int blockRight, int blockBottom);

	// levels [2, 255]
	static int posterize(Sheet* srcImage, Sheet* dstImage, PosterizeBlob* blob, byte threshold,
		int blockLeft, int blockTop, int blockRight, int blockBottom);

	// levels [0, 255]
	static int threshold(Sheet* srcImage, Sheet* dstImage, ThresholdBlob* blob, bool isMonochromatic, byte threshold,
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
	static int crystalize(Sheet* srcImage, Sheet* dstImage, CrystalizeBlob* crystalizeBlob,
		CrystalizeMode crystalizeMode, bool shouldFadeEdges,
		double edgeThickness, double randomness, double distancePower, double scale,
		int blockLeft, int blockTop, int blockRight, int blockBottom);

	// iteration [1, 20]
	static int despeckle(Sheet* srcImage, Sheet* dstImage, size_t iteration,
		int blockLeft, int blockTop, int blockRight, int blockBottom);

	// radius [0, 254]
	static int gaussian_blur(Sheet* srcImage, Sheet* dstImage, size_t radius,
		int blockLeft, int blockTop, int blockRight, int blockBottom);

	// gian, bias [0, 1]
	static int gain(Sheet* srcImage, Sheet* dstImage, GainBlob* blob, double gain, double bias,
		int blockLeft, int blockTop, int blockRight, int blockBottom);

	// softness [1, 16]
	// contrast, brightness [-100, 100]
	static int glow(Sheet* srcImage, Sheet* dstImage, GlowBlob* blob, int softness, int brightness, int contrast,
		int blockLeft, int blockTop, int blockRight, int blockBottom);

	// telorance, scale [0, 100]
	static int marble(Sheet* srcImage, Sheet* dstImage, MarbleBlob* blob, double telorance, double scale,
		int blockLeft, int blockTop, int blockRight, int blockBottom);

	static int maximum(Sheet* srcImage, Sheet* dstImage,
		int blockLeft, int blockTop, int blockRight, int blockBottom);

	// radius [1, 200]
	// percentile [0, 100]
	static int median(Sheet* srcImage, Sheet* dstImage, MedianBlob* blob, size_t radius, size_t percentile,
		int blockLeft, int blockTop, int blockRight, int blockBottom);

	static int minimum(Sheet* srcImage, Sheet* dstImage,
		int blockLeft, int blockTop, int blockRight, int blockBottom);

	// size [3, 2000]
	// angle [-PI, PI]
	static int motion_blur(Sheet* srcImage, Sheet* dstImage, size_t size, double angle,
		int blockLeft, int blockTop, int blockRight, int blockBottom);

	// brushSize [1, 100]
	// coareness [3, 255]
	static int oil_paint(Sheet* srcImage, Sheet* dstImage, OldPaintBlob* blob, size_t brushSize, size_t coareness,
		int blockLeft, int blockTop, int blockRight, int blockBottom);

	// radius [1, 200]
	// intensity [0, 100]
	static int outline(Sheet* srcImage, Sheet* dstImage, OutlineBlob* blob, size_t radius, size_t intensity,
		int blockLeft, int blockTop, int blockRight, int blockBottom);

	// brushSize [1, 50]
	// range [-20, 20]
	static int pencil_sketch(Sheet* srcImage, Sheet* dstImage, PencilSketchBlob* blob, size_t brushSize, int range,
		int blockLeft, int blockTop, int blockRight, int blockBottom);

	// cellSize [1, 200]
	static int pixelate(Sheet* srcImage, Sheet* dstImage, size_t cellSize,
		int blockLeft, int blockTop, int blockRight, int blockBottom);

	// amount [1, 100]
	// pivotX, pivotY [0, 1]
	static int radial_blur(Sheet* srcImage, Sheet* dstImage, RadialBlurMode blurMode, double amount, double pivotX, double pivotY,
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
	static int smart_blur(Sheet* srcImage, Sheet* dstImage, SmartBlurBlob* blob, size_t radius, size_t threshold,
		int blockLeft, int blockTop, int blockRight, int blockBottom);

	// amount [1, 100]
	// mix, density [0, 1]
	static int smear(Sheet* srcImage, Sheet* dstImage, SmearMode smearMode, double amount, double mix, double density, double angle,
		int blockLeft, int blockTop, int blockRight, int blockBottom);

	// softness [0, 10]
	// warmness [0, 40]
	// brightness [-100, 100]
	static int soft_portrait(Sheet* srcImage, Sheet* dstImage, SoftPortraitBlob* blob, size_t softness, size_t warmness, int brightness,
		int blockLeft, int blockTop, int blockRight, int blockBottom);

	// radius, threshold [0, 100]
	static int stamp(Sheet* srcImage, Sheet* dstImage, StampBlob* blob, size_t radius, double threshold,
		int blockLeft, int blockTop, int blockRight, int blockBottom);

	// radius [1, 100]
	// level [2, 255]
	static int surface_blur(Sheet* srcImage, Sheet* dstImage, SurfaceBlurBlob* blob, double radius, double level,
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

