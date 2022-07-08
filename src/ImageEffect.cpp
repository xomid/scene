#include "ImageEffect.h"
#include "ColorBalanceData.h"

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


struct ColorRGB
{
	double r, g, b;
};

struct ColorLab
{
	double a, l, b;
};

struct ColorXYZ
{
	double x, y, z;
};

struct ColorLch
{
	double l, c, h;
};

#define refX 95.047
#define refY 100.000
#define refZ 108.883
#define refXr 0.4124564
#define refXg 0.3575761
#define refXb 0.1804375
#define refYr 0.2126729
#define refYg 0.7151522
#define refYb 0.0721750
#define refZr 0.0193339 
#define refZg 0.1191920
#define refZb 0.9505041

void LinearRgbToXyz(ColorRGB rgb, ColorXYZ* res)
{
	rgb.r = rgb.r;
	rgb.g = rgb.g;
	rgb.b = rgb.b;
	res->x = rgb.r * refXr + rgb.g * refXg + rgb.b * refXb;
	res->y = rgb.r * refYr + rgb.g * refYg + rgb.b * refYb;
	res->z = rgb.r * refZr + rgb.g * refZg + rgb.b * refZb;
}

void XyzToLinearRgb(ColorXYZ xyz, ColorRGB* res)
{
	xyz.x = xyz.x;
	xyz.y = xyz.y;
	xyz.z = xyz.z;
	res->r = xyz.x * 3.2404542 + xyz.y * -1.5371385 + xyz.z * -0.4985314;
	res->g = xyz.x * -0.9692660 + xyz.y * 1.8760108 + xyz.z * 0.0415560;
	res->b = xyz.x * 0.0556434 + xyz.y * -0.2040259 + xyz.z * 1.0572252;
}

void XyzToRgb(ColorXYZ xyz, ColorRGB* res)
{
	XyzToLinearRgb(xyz, res);
	if (res->r > 0.0031308)
		res->r = 1.055 * pow(res->r, (1.0 / 2.4)) - 0.055;
	else
		res->r = 12.92 * res->r;
	if (res->g > 0.0031308)
		res->g = 1.055 * pow(res->g, (1.0 / 2.4)) - 0.055;
	else
		res->g = 12.92 * res->g;
	if (res->b > 0.0031308)
		res->b = 1.055 * pow(res->b, (1.0 / 2.4)) - 0.055;
	else
		res->b = 12.92 * res->b;
}

void RgbToXyz(ColorRGB rgb, ColorXYZ* res)
{
	if (rgb.r > 0.04045)
		rgb.r = pow((rgb.r + 0.055) / 1.055, 2.4);
	else
		rgb.r = rgb.r / 12.92;
	if (rgb.g > 0.04045)
		rgb.g = pow((rgb.g + 0.055) / 1.055, 2.4);
	else
		rgb.g = rgb.g / 12.92;
	if (rgb.b > 0.04045)
		rgb.b = pow((rgb.b + 0.055) / 1.055, 2.4);
	else
		rgb.b = rgb.b / 12.92;

	LinearRgbToXyz(rgb, res);
}

void XyzToLab(ColorXYZ xyz, ColorLab* res)
{
	xyz.x = xyz.x / refX;
	xyz.y = xyz.y / refY;
	xyz.z = xyz.z / refZ;
	if (xyz.x > 0.008856)
		xyz.x = pow(xyz.x, 1 / 3.0);
	else
		xyz.x = (7.787037 * xyz.x) + (16 / 116.0);
	if (xyz.y > 0.008856)
		xyz.y = pow(xyz.y, 1 / 3.0);
	else
		xyz.y = (7.787037 * xyz.y) + (16 / 116.0);
	if (xyz.z > 0.008856)
		xyz.z = pow(xyz.z, 1 / 3.0);
	else
		xyz.z = (7.787037 * xyz.z) + (16 / 116.0);
	res->l = (116 * xyz.y) - 16;
	res->a = 500 * (xyz.x - xyz.y);
	res->b = 200 * (xyz.y - xyz.z);
}

double f(double v)
{
	double v3, res;
	v3 = v * v * v;
	if (v3 > 0.008856)
		res = v3;
	else
		res = (v - (16 / 116.0)) / 7.787037;
	return res;
}

void LabToXyz(ColorLab lab, ColorXYZ* res)
{
	res->y = (lab.l + 16) / 116.0;
	res->x = lab.a / 500.0 + res->y;
	res->z = res->y - lab.b / 200.0;
	res->x = f(res->x) * refX;
	res->y = f(res->y) * refY;
	res->z = f(res->z) * refZ;
}

int ImageEffect::color_balance(Sheet* srcImage, Sheet* dstImage, bool shouldPreserveLuminosity,
	int red, int green, int blue, int blockLeft, int blockTop, int blockRight, int blockBottom) {

	VALIDATE_IMAGES();
	DECLARE_VARIABLES();
	CLAMP_BLOCK();

	ColorRGB trgb;
	ColorLab ln, lo, tl;
	ColorRGB rgb;
	ColorXYZ xyz;

	if (shouldPreserveLuminosity)
	{
		trgb.r = double(red) / 50.0;
		trgb.g = double(green) / 50.0;
		trgb.b = double(blue) / 50.0;

		LinearRgbToXyz(trgb, &xyz);
		XyzToLab(xyz, &tl);

		for (y = blockTop; y < blockBottom; ++y) {
			s = src + y * p + blockLeft * 3;
			d = dst + y * p + blockLeft * 3;
			for (x = blockLeft; x < blockRight; ++x) {

				rgb.b = *s++;
				rgb.g = *s++;
				rgb.r = *s++;

				LinearRgbToXyz(rgb, &xyz);
				XyzToLab(xyz, &lo);

				lo.l = lo.l;
				lo.a = lo.a + tl.a;
				lo.b = lo.b + tl.b;

				LabToXyz(lo, &xyz);
				XyzToLinearRgb(xyz, &rgb);

				*d++ = CLAMP255(rgb.b);
				*d++ = CLAMP255(rgb.g);
				*d++ = CLAMP255(rgb.r);
			}
		}
	}
	else
	{
		byte r, g, b;
		b = int(blue + 100.0);
		g = int(green + 100.0);
		r = int(red + 100.0);

		auto blookup = ColorBalanceData[b];
		auto glookup = ColorBalanceData[g];
		auto rlookup = ColorBalanceData[r];

		for (y = blockTop; y < blockBottom; ++y) {
			s = src + y * p + blockLeft * 3;
			d = dst + y * p + blockLeft * 3;
			for (x = blockLeft; x < blockRight; ++x) {
				*d++ = blookup[*s++];
				*d++ = glookup[*s++];
				*d++ = rlookup[*s++];
			}
		}
	}

	return IMAGE_EFFECT_RESULT_OK;
}


static double cm_calculate_norm(ChannelMixInfo* channelInfo, bool shouldPreserveLuminosity)
{
	double sum = channelInfo->red + channelInfo->green + channelInfo->blue;
	if (sum == 0.0 || !shouldPreserveLuminosity)
		return 1.0;
	return fabs(1 / sum);
}

int ImageEffect::channel_mixer(Sheet* srcImage, Sheet* dstImage,
	ChannelMixInfo& grayInfo, ChannelMixInfo& redInfo, ChannelMixInfo& greenInfo, ChannelMixInfo& blueInfo,
	bool isMonochromatic, bool shouldPreserveLuminosity,
	int blockLeft, int blockTop, int blockRight, int blockBottom)
{
	VALIDATE_IMAGES();
	DECLARE_VARIABLES();
	CLAMP_BLOCK();

	byte r, g, b;

	if (isMonochromatic) {
		double grayNorm = cm_calculate_norm(&grayInfo, shouldPreserveLuminosity);
		for (y = blockTop; y < blockBottom; ++y) {
			s = src + y * p + blockLeft * 3;
			d = dst + y * p + blockLeft * 3;
			for (x = blockLeft; x < blockRight; ++x) {
				b = *s++;
				g = *s++;
				r = *s++;

				g = CLAMP255((grayInfo.red * r + grayInfo.green * g + grayInfo.blue * b) * grayNorm + 0.5);
				*d++ = g;
				*d++ = g;
				*d++ = g;
			}
		}
	}
	else {
		double redNorm, greenNorm, blueNorm;
		redNorm = cm_calculate_norm(&redInfo, shouldPreserveLuminosity);
		greenNorm = cm_calculate_norm(&greenInfo, shouldPreserveLuminosity);
		blueNorm = cm_calculate_norm(&blueInfo, shouldPreserveLuminosity);
		for (y = blockTop; y < blockBottom; ++y) {
			s = src + y * p + blockLeft * 3;
			d = dst + y * p + blockLeft * 3;
			for (x = blockLeft; x < blockRight; ++x) {

				b = *s++;
				g = *s++;
				r = *s++;

				*d++ = CLAMP255((blueInfo.red * r + blueInfo.green * g + blueInfo.blue * b) * blueNorm + 0.5);
				*d++ = CLAMP255((greenInfo.red * r + greenInfo.green * g + greenInfo.blue * b) * greenNorm + 0.5);
				*d++ = CLAMP255((redInfo.red * r + redInfo.green * g + redInfo.blue * b) * redNorm + 0.5);
			}
		}
	}


	return IMAGE_EFFECT_RESULT_OK;
}



int ImageEffect::curves(Sheet* srcImage, Sheet* dstImage, CurvesBlob* blob,
	int blockLeft, int blockTop, int blockRight, int blockBottom)
{
	if (!blob) return IMAGE_EFFECT_RESULT_ERROR;
	return apply_lookup_all(srcImage, dstImage, blob, blockLeft, blockTop, blockRight, blockBottom);
}








#define GIMP_RGB_LUMINANCE_RED    (0.2126)
#define GIMP_RGB_LUMINANCE_GREEN  (0.7152)
#define GIMP_RGB_LUMINANCE_BLUE   (0.0722)

struct SHSL
{
	double h, s, l;
};

struct SRGB
{
	double r, g, b;
};


static inline double hsl_value(double n1, double n2, double hue)
{
	double val;

	if (hue > 6.0)
		hue -= 6.0;
	else if (hue < 0.0)
		hue += 6.0;

	if (hue < 1.0)
		val = n1 + (n2 - n1) * hue;
	else if (hue < 3.0)
		val = n2;
	else if (hue < 4.0)
		val = n1 + (n2 - n1) * (4.0 - hue);
	else
		val = n1;

	return val;
}

void hsl_to_rgb(SHSL* hsl, SRGB* rgb)
{
	if (hsl->s == 0)
	{
		rgb->r = hsl->l;
		rgb->g = hsl->l;
		rgb->b = hsl->l;
	}
	else
	{
		double m1, m2;

		if (hsl->l <= 0.5)
			m2 = hsl->l * (1.0 + hsl->s);
		else
			m2 = hsl->l + hsl->s - hsl->l * hsl->s;

		m1 = 2.0 * hsl->l - m2;

		rgb->r = hsl_value(m1, m2, hsl->h * 6.0 + 2.0);
		rgb->g = hsl_value(m1, m2, hsl->h * 6.0);
		rgb->b = hsl_value(m1, m2, hsl->h * 6.0 - 2.0);
	}
}

__forceinline double Hue_2_RGB(double v1, double v2, double vH)
{
	if (vH < 0) vH += 1;
	if (vH > 1) vH -= 1;
	if ((6 * vH) < 1) return (v1 + (v2 - v1) * 6 * vH);
	if ((2 * vH) < 1) return (v2);
	if ((3 * vH) < 2) return (v1 + (v2 - v1) * (0.6666666666666667 - vH) * 6.0);

	return (v1);
}

__forceinline void RgbToHsv(pyte src, pyte dst, double hue, double saturation)
{
	int var_R, var_G, var_B, var_Min, var_Max;
	double L, H, S, del_R, del_G, del_B, del_Max;

	var_B = (*src++);                  //RGB from 0 to 255
	var_G = (*src++);
	var_R = (*src++);

	var_Min = Min(Min(var_R, var_G), var_B);    //Min. value of RGB
	var_Max = Max(Max(var_R, var_G), var_B);   //Max. value of RGB
	del_Max = (var_Max - var_Min);       //Delta RGB value

	L = (var_Max + var_Min) / 510.0;

	if (del_Max == 0)
	{
		H = 0;
		S = 0;
	}
	else                                    //Chromatic data...
	{
		if (L < 0.5) S = del_Max / (var_Max + var_Min);
		else         S = del_Max / (510 - var_Max - var_Min);

		del_R = (((var_Max - var_R) / 6.0) + (del_Max / 2.0)) / del_Max;
		del_G = (((var_Max - var_G) / 6.0) + (del_Max / 2.0)) / del_Max;
		del_B = (((var_Max - var_B) / 6.0) + (del_Max / 2.0)) / del_Max;

		if (var_R == var_Max) H = del_B - del_G;
		else if (var_G == var_Max) H = 0.3333333333333333 + del_R - del_B;
		else if (var_B == var_Max) H = 0.6666666666666667 + del_G - del_R;

		if (H < 0) H += 1;
		if (H > 1) H -= 1;
	}

	H += hue;
	if (H > 1) H = H - 1;
	if (H < 0) H = H + 1;
	S = fminf(fmaxf(S + (saturation), 0), 1);

	double var_1, var_2;
	int R, G, B;

	if (S == 0)                       //HSL from 0 to 1
	{
		R = L * 255;                      //RGB results from 0 to 255
		G = L * 255;
		B = L * 255;
	}
	else
	{
		if (L < 0.5) var_2 = L * (1 + S);
		else         var_2 = (L + S) - (S * L);

		var_1 = 2 * L - var_2;

		R = 255 * Hue_2_RGB(var_1, var_2, H + 0.3333333333333333);
		G = 255 * Hue_2_RGB(var_1, var_2, H);
		B = 255 * Hue_2_RGB(var_1, var_2, H - 0.3333333333333333);
	}

	dst[0] = B;
	dst[1] = G;
	dst[2] = R;
}


void colorize_init(HSLBlob* blob)
{
	int i;

	blob->hue = 180.0;
	blob->saturation = 50.0;
	blob->lightness = 0.0;

	for (i = 0; i < 256; i++)
	{
		blob->red[i] = i * GIMP_RGB_LUMINANCE_RED;
		blob->green[i] = i * GIMP_RGB_LUMINANCE_GREEN;
		blob->blue[i] = i * GIMP_RGB_LUMINANCE_BLUE;
	}
}


void colorize_calculate(HSLBlob* blob)
{
	SHSL hsl;
	SRGB rgb;
	int    i;

	hsl.h = blob->hue / 360.0;
	hsl.s = blob->saturation / 100.0;

	for (i = 0; i < 256; i++)
	{
		hsl.l = i / 255.0;

		hsl_to_rgb(&hsl, &rgb);

		blob->finalRedLookup[i] = 255.0 * rgb.r;
		blob->finalGreenLookup[i] = 255.0 * rgb.g;
		blob->finalBlueLookup[i] = 255.0 * rgb.b;
	}
}

int HSLBlob::init(bool shouldColorize, int& hue, int& saturation, int& lightness)
{
	if (shouldColorize) {
		hue = CLAMP3(0, hue, 360);
		saturation = CLAMP3(0, saturation, 100);
	}
	else {
		hue = CLAMP3(-180, hue, 180);
		saturation = CLAMP3(-100, saturation, 100);
	}
	lightness = CLAMP3(-100, lightness, 100);

	if (!isSet ||
		this->shouldColorize != shouldColorize ||
		this->_hue != hue ||
		this->_saturation != saturation ||
		this->_lightness != lightness) {

		colorize_init(this);
		this->hue = hue;
		this->saturation = saturation;
		this->lightness = lightness;
		colorize_calculate(this);

		isSet = true;
		this->shouldColorize == shouldColorize;
		this->_hue == hue;
		this->_saturation == saturation;
		this->_lightness == lightness;
	}

	return IMAGE_EFFECT_RESULT_OK;
}


int ImageEffect::hsl(Sheet* srcImage, Sheet* dstImage, HSLBlob* blob, bool shouldColorize, int hue, int saturation, int lightness,
	int blockLeft, int blockTop, int blockRight, int blockBottom) {

	VALIDATE_IMAGES();
	DECLARE_VARIABLES();
	CLAMP_BLOCK();

	if (!blob || blob->init(shouldColorize, hue, saturation, lightness))
		return IMAGE_EFFECT_RESULT_ERROR;
	
	if (shouldColorize)
	{
		int lum;
		byte lumb;

		for (y = blockTop; y < blockBottom; ++y) {
			s = src + y * p + blockLeft * 3;
			d = dst + y * p + blockLeft * 3;
			for (x = blockLeft; x < blockRight; ++x) {

				lum = (blob->blue[*s++] +
					blob->green[*s++] +
					blob->red[*s++]);

				if (blob->lightness > 0)
				{
					lum = (double)lum * (100.0 - blob->lightness) / 100.0;
					lum += 255 - (100.0 - blob->lightness) * 255.0 / 100.0;
				}
				else if (blob->lightness < 0)
				{
					lum = (double)lum * (blob->lightness + 100.0) / 100.0;
				}

				lumb = CLAMP255(lum);
				*d++ = blob->finalBlueLookup[lumb];
				*d++ = blob->finalGreenLookup[lumb];
				*d++ = blob->finalRedLookup[lumb];
			}
		}
	}
	else
	{
		double sat = fmin(fmax((lightness / 100.), -1.), 1.);
		double huef = double(hue) / 360.0;

		if (lightness >= 0)
		{
			for (y = blockTop; y < blockBottom; ++y) {
				s = src + y * p + blockLeft * 3;
				d = dst + y * p + blockLeft * 3;
				for (x = blockLeft; x < blockRight; ++x) {
					RgbToHsv(s, d, huef, saturation / 100.);

					d[0] = d[0] + (255 - d[0]) * sat;
					d[1] = d[1] + (255 - d[1]) * sat;
					d[2] = d[2] + (255 - d[2]) * sat;
					s += 3;
					d += 3;
				}
			}
		}
		else
		{
			for (y = blockTop; y < blockBottom; ++y) {
				s = src + y * p + blockLeft * 3;
				d = dst + y * p + blockLeft * 3;
				for (x = blockLeft; x < blockRight; ++x) {
					RgbToHsv(s, d, huef, saturation / 100.);

					d[0] = d[0] + d[0] * sat;
					d[1] = d[1] + d[1] * sat;
					d[2] = d[2] + d[2] * sat;
					s += 3;
					d += 3;
				}
			}
		}
	}
	return IMAGE_EFFECT_RESULT_OK;
}


void ChannelLevelInfo::fill_lookup(ChannelLevelInfo* rgbInfo) {

	int i, e, k;
	double range, gamma = 1. / (this->gamma * rgbInfo->gamma);

	e = maxRange + 1;
	range = maxRange - minRange;

	for (i = 0; i < minRange; i++)
		lookup[i] = 0;
	for (i = e; i < 256; i++)
		lookup[i] = 255;

	for (i = minRange; i < e; i++)
		lookup[i] = CLAMP255(255 * pow((i - minRange) / range, gamma));
}

int ImageEffect::levels(Sheet* srcImage, Sheet* dstImage, LevelsBlob* blob,
	ChannelLevelInfo& rgbInfo, ChannelLevelInfo& redInfo, ChannelLevelInfo& greenInfo, ChannelLevelInfo& blueInfo,
	int blockLeft, int blockTop, int blockRight, int blockBottom) 
{
	if (!blob || blob->init(rgbInfo, redInfo, greenInfo, blueInfo))
		return IMAGE_EFFECT_RESULT_ERROR;
	
	VALIDATE_IMAGES();
	DECLARE_VARIABLES();
	CLAMP_BLOCK();

	auto vlookup = rgbInfo.lookup;
	auto rlookup = redInfo.lookup;
	auto glookup = greenInfo.lookup;
	auto blookup = blueInfo.lookup;

	auto vslookup = blob->vslookup;
	auto rslookup = blob->rslookup;
	auto gslookup = blob->gslookup;
	auto bslookup = blob->bslookup;

	for (y = blockTop; y < blockBottom; ++y) {
		s = src + y * p + blockLeft * 3;
		d = dst + y * p + blockLeft * 3;
		for (x = blockLeft; x < blockRight; ++x) {
			*d++ = vlookup[vslookup[bslookup[blookup[*s++]]]];
			*d++ = vlookup[vslookup[gslookup[glookup[*s++]]]];
			*d++ = vlookup[vslookup[rslookup[rlookup[*s++]]]];
		}
	}

	return IMAGE_EFFECT_RESULT_OK;
}


int ImageEffect::threshold(Sheet* srcImage, Sheet* dstImage, ThresholdBlob* blob, bool isMonochromatic, byte threshold,
	int blockLeft, int blockTop, int blockRight, int blockBottom) {

	if (!blob || blob->init(threshold, isMonochromatic))
		return 1;

	if (!isMonochromatic)
		return apply_lookup_gray(srcImage, dstImage, blob, blockLeft, blockTop, blockRight, blockBottom);

	VALIDATE_IMAGES();
	DECLARE_VARIABLES();
	CLAMP_BLOCK();

	byte* gray = blob->gray, g;
	for (y = blockTop; y < blockBottom; ++y) {
		s = src + y * p + blockLeft * 3;
		d = dst + y * p + blockLeft * 3;
		for (x = blockLeft; x < blockRight; ++x) {
			g = gray[int((Max(Max(s[0], s[1]), s[2]) + Min(Min(s[0], s[1]), s[2])) / 2.0 + 0.5)];
			*d++ = g;
			*d++ = g;
			*d++ = g;
			s += 3;
		}
	}

	return IMAGE_EFFECT_RESULT_OK;
}



