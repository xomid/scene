#include "ImageEffect.h"

bool are_not_equal(double a, double b) {
	return fabs(a - b) > DBL_EPSILON;
}

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

#define MAX_WANIMATE 50

typedef struct RG
{
	byte B, G, R;
} *LR;

typedef struct RGA
{
	byte B, G, R, A;
} *LAR;

typedef struct RGG
{
	byte G;
} *LGR;


float func(float t)
{
	return -1.0 / 2 * (cos(PI * t / 1) - 1);
}

void mixColorsa(float t, pyte clr1, pyte clr2, pyte dst)
{
	int a1 = clr1[3];
	int r1 = clr1[2];
	int g1 = clr1[1];
	int b1 = clr1[0];
	int a2 = clr2[3];
	int r2 = clr2[2];
	int g2 = clr2[1];
	int b2 = clr2[0];

	dst[0] = (byte)(b1 + t * (b2 - b1));
	dst[1] = (byte)(g1 + t * (g2 - g1));
	dst[2] = (byte)(r1 + t * (r2 - r1));
	dst[3] = (byte)(a1 + t * (a2 - a1));
}

void mixColors(float t, pyte clr1, pyte clr2, pyte dst)
{
	int r1 = clr1[2];
	int g1 = clr1[1];
	int b1 = clr1[0];
	int r2 = clr2[2];
	int g2 = clr2[1];
	int b2 = clr2[0];
	r1 = (int)(r1 + t * (r2 - r1));
	g1 = (int)(g1 + t * (g2 - g1));
	b1 = (int)(b1 + t * (b2 - b1));

	dst[0] = b1;
	dst[1] = g1;
	dst[2] = r1;
}

void mixColorsg(float t, pyte clr1, pyte clr2, pyte dst)
{
	int g1 = clr1[0];
	int g2 = clr2[0];
	dst[0] = byte(g1 + t * (g2 - g1));
}

inline int smooth(byte* v)
{
	int minindex = 0, maxindex = 0, min = INT_MAX, max = INT_MIN;

	for (int i = 0; i < 9; i++) {
		if (i != 4) {
			if (v[i] < min) {
				min = v[i];
				minindex = i;
			}
			if (v[i] > max) {
				max = v[i];
				maxindex = i;
			}
		}
	}
	if (v[4] < min)
		return v[minindex];
	if (v[4] > max)
		return v[maxindex];
	return v[4];
}

static float blurMatrix[] =
{
	1 / 16.0f, 1 / 8.0f, 1 / 16.0f,
	1 / 8.0f, 1 / 4.0f, 1 / 8.0f,
	1 / 16.0f, 1 / 8.0f, 1 / 16.0f
};

static const unsigned short stackblur_mul[255] =
{
		512,512,456,512,328,456,335,512,405,328,271,456,388,335,292,512,
		454,405,364,328,298,271,496,456,420,388,360,335,312,292,273,512,
		482,454,428,405,383,364,345,328,312,298,284,271,259,496,475,456,
		437,420,404,388,374,360,347,335,323,312,302,292,282,273,265,512,
		497,482,468,454,441,428,417,405,394,383,373,364,354,345,337,328,
		320,312,305,298,291,284,278,271,265,259,507,496,485,475,465,456,
		446,437,428,420,412,404,396,388,381,374,367,360,354,347,341,335,
		329,323,318,312,307,302,297,292,287,282,278,273,269,265,261,512,
		505,497,489,482,475,468,461,454,447,441,435,428,422,417,411,405,
		399,394,389,383,378,373,368,364,359,354,350,345,341,337,332,328,
		324,320,316,312,309,305,301,298,294,291,287,284,281,278,274,271,
		268,265,262,259,257,507,501,496,491,485,480,475,470,465,460,456,
		451,446,442,437,433,428,424,420,416,412,408,404,400,396,392,388,
		385,381,377,374,370,367,363,360,357,354,350,347,344,341,338,335,
		332,329,326,323,320,318,315,312,310,307,304,302,299,297,294,292,
		289,287,285,282,280,278,275,273,271,269,267,265,263,261,259
};

static const byte stackblur_shr[255] =
{
		9, 11, 12, 13, 13, 14, 14, 15, 15, 15, 15, 16, 16, 16, 16, 17,
		17, 17, 17, 17, 17, 17, 18, 18, 18, 18, 18, 18, 18, 18, 18, 19,
		19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 20, 20, 20,
		20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 21,
		21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21,
		21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 22, 22, 22, 22, 22, 22,
		22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22,
		22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 23,
		23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23,
		23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23,
		23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23,
		23, 23, 23, 23, 23, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24,
		24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24,
		24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24,
		24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24,
		24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24
};


bool haveNextNextGaussian;
long seed;
double nextNextGaussian;
long serialVersionUID = 3905348978240129619L;
long multiplier = 0x5deece66dL;

int nexti(int bits) {
	seed = (seed * multiplier + 0xbL) & ((1L << 48) - 1);
	return (int)(seed >> (48 - bits));
}

double nextDouble() {
	return (double(rand()) / RAND_MAX);
}

double nextGaussian()
{
	if (haveNextNextGaussian)
	{
		haveNextNextGaussian = false;
		return nextNextGaussian;
	}
	double v1, v2, s;
	do {
		v1 = 2 * nextDouble() - 1;
		v2 = 2 * nextDouble() - 1;
		s = v1 * v1 + v2 * v2;
	} while (s >= 1);

	double norm = sqrt(-2 * log(s) / s);
	nextNextGaussian = v2 * norm;
	haveNextNextGaussian = true;
	return v1 * norm;
}

float nextFloat() {
	return (float(rand()) / RAND_MAX);
}

inline int random(int x, bool isGussian, double amount)
{
	x += (int)(((isGussian ? nextGaussian() : 2 * nextFloat() - 1)) * amount);
	if (x < 0)
		x = 0;
	else if (x > 0xff)
		x = 0xff;
	return x;
}

void setSeed(long iseed)
{
	seed = int(pow(iseed, multiplier)) & ((1L << 48) - 1);
	haveNextNextGaussian = false;
}

inline byte overlay_blend(int s, int d)
{
	int t;

	if (d < 128)
	{
		t = 2 * d * s + 0x80;
		t = ((t >> 8) + t) >> 8;
	}
	else
	{
		t = 2 * (255 - d) * (255 - s) + 0x80;
		t = ((t >> 8) + t) >> 8;
		t = 255 - t;
	}

	return (byte)t;
}

__forceinline byte pepperAndSalt(byte c, byte v1, byte v2)
{
	if (c < v1)
		c++;
	if (c < v2)
		c++;
	if (c > v1)
		c--;
	if (c > v2)
		c--;
	return c;
}

int ImageEffect::desaturate(Sheet* srcImage, Sheet* dstImage,
	int blockLeft, int blockTop, int blockRight, int blockBottom) {

	VALIDATE_IMAGES();
	DECLARE_VARIABLES();
	CLAMP_BLOCK();
	byte gray;

	for (y = blockTop; y < blockBottom; ++y)
	{
		s = src + y * p + blockLeft * 3;
		d = dst + y * p + blockLeft * 3;
		for (x = blockLeft; x < blockRight; ++x, s += 3)
		{
			gray = CLAMP255(int((Max(Max(s[0], s[1]), s[2]) + Min(Min(s[0], s[1]), s[2])) / 2.0 + 0.5));
			*d++ = gray;
			*d++ = gray;
			*d++ = gray;
		}
	}

	return IMAGE_EFFECT_RESULT_OK;
}

int ImageEffect::convolution(Sheet* srcImage, Sheet* dstImage, float* matrix, size_t rowCount, size_t columnCount,
	int blockLeft, int blockTop, int blockRight, int blockBottom)
{
	VALIDATE_IMAGES();
	DECLARE_VARIABLES();
	CLAMP_BLOCK();

	int ix, iy, r, g, b, a, ioffset, moffset, row, col, rowMin, rowMax, colMin, colMax;
	float hf;
	byte c;

	r = 0;
	g = 0;
	b = 0;
	a = 255;
	rowMin = -int(rowCount / 2);
	rowMax = rowMin + int(rowCount);
	colMin = -int(columnCount / 2);
	colMax = colMin + int(columnCount);

	for (y = blockTop; y < blockBottom; ++y)
	{
		s = src + y * p + blockLeft * 3;
		d = dst + y * p + blockLeft * 3;
		for (x = blockLeft; x < blockRight; ++x)
		{
			r = 0, g = 0, b = 0;

			for (row = rowMin; row < rowMax; row++)
			{
				iy = y + row;
				if (0 <= iy && iy < bottom) ioffset = iy * p;
				else ioffset = y * p;

				moffset = columnCount * (row - rowMin) - colMin;

				for (col = colMin; col < colMax; col++)
				{
					ix = x + col;
					if (0 > ix || ix > right) ix = x;
					s = src + ioffset + 3 * ix;
					hf = matrix[moffset + col];
					r += s[2] * hf;
					g += s[1] * hf;
					b += s[0] * hf;
				}
			}

			*d++ = CLAMP255(b);
			*d++ = CLAMP255(g);
			*d++ = CLAMP255(r);
		}
	}

	return IMAGE_EFFECT_RESULT_OK;
}

int ImageEffect::add_noise(Sheet* srcImage, Sheet* dstImage, NoiseType noiseType, double amount,
	bool isMonochromatic, int blockLeft, int blockTop, int blockRight, int blockBottom) {

	VALIDATE_IMAGES();
	DECLARE_VARIABLES();
	CLAMP_BLOCK();

	int ix, iy;
	bool isGaussian = noiseType == NoiseType::Gussian;
	amount = fmin(fmax(amount, 0.1), 400.);
	byte r, g, b, a;
	int n;

	for (y = blockTop; y < blockBottom; ++y)
	{
		s = src + y * p + blockLeft * 3;
		d = dst + y * p + blockLeft * 3;
		for (x = blockLeft; x < blockRight; ++x)
		{
			b = *s++;
			g = *s++;
			r = *s++;

			if (nextFloat() <= 1.0f)
			{
				if (isMonochromatic) {
					n = (int)(((isGaussian ? nextGaussian() : 2 * nextFloat() - 1)) * amount);
					r = CLAMP255(r + n);
					g = CLAMP255(g + n);
					b = CLAMP255(b + n);
				}
				else {
					r = random(r, isGaussian, amount);
					g = random(g, isGaussian, amount);
					b = random(b, isGaussian, amount);
				}
			}

			*d++ = b;
			*d++ = g;
			*d++ = r;
		}
	}

	return IMAGE_EFFECT_RESULT_OK;
}

int ImageEffect::bulge(Sheet* srcImage, Sheet* dstImage, double amount, bool shouldStretch, double pivotX, double pivotY,
	int blockLeft, int blockTop, int blockRight, int blockBottom) {

	VALIDATE_IMAGES();
	DECLARE_VARIABLES();
	CLAMP_BLOCK();


	amount = CLAMP3F(-2., amount, 1.);
	pivotX = CLAMP3F(0., pivotX, 1.);
	pivotY = CLAMP3F(0., pivotY, 1.);

	double un_x, un_y, fx, fy;
	int i, nSrcX, nSrcY, nSrcX_1, nSrcY_1;
	pyte px0, px1, px2, px3;
	double m0, m1, my, maxrad, u, v, r, rscale1, rscale2;
	double sc = w > h ? (double)w / (double)h : (double)h / (double)w;
	pivotX *= w;
	pivotY *= h;
	maxrad = (pivotX < pivotY ? pivotX : pivotY);

	if (shouldStretch)
	{
		if (w > h)
		{
			for (y = blockTop; y < blockBottom; ++y)
			{
				d = dst + y * p + blockLeft * 3;
				for (x = blockLeft; x < blockRight; ++x)
				{
					u = ((double)x - pivotX) / sc;
					v = (double)y - pivotY;
					r = sqrt(u * u + v * v);
					rscale1 = 1.0 - (r / maxrad);

					if (rscale1 > 0)
					{
						rscale2 = 1.0 - amount * rscale1 * rscale1;
						un_x = u * rscale2 * sc + pivotX;
						un_y = v * rscale2 + pivotY;
					}
					else
					{
						un_x = x;
						un_y = y;
					}

					nSrcX = CLAMP3(0, (int)un_x, right),
						nSrcY = CLAMP3(0, (int)un_y, bottom),
						nSrcX_1 = CLAMP3(0, nSrcX + 1, right),
						nSrcY_1 = CLAMP3(0, nSrcY + 1, bottom);

					px0 = src + nSrcY * p + 3 * nSrcX;
					px1 = src + nSrcY * p + 3 * nSrcX_1;
					px2 = src + nSrcY_1 * p + 3 * nSrcX;
					px3 = src + nSrcY_1 * p + 3 * nSrcX_1;

					fx = un_x - nSrcX;
					fy = un_y - nSrcY;

					for (i = 0; i < 3; i++)
					{
						m0 = px0[i] + fx * (px1[i] - px0[i]),
							m1 = px2[i] + fx * (px3[i] - px2[i]),
							my = m0 + fy * (m1 - m0);

						*d++ = CLAMP255(my);
					}
				}
			}
		}
		else
		{
			for (y = blockTop; y < blockBottom; ++y)
			{
				d = dst + y * p + blockLeft * 3;
				for (x = blockLeft; x < blockRight; ++x)
				{
					u = x - pivotX;
					v = (y - pivotY) / sc;
					r = sqrt(u * u + v * v);
					rscale1 = 1.0 - (r / maxrad);

					if (rscale1 > 0)
					{
						rscale2 = 1.0 - amount * rscale1 * rscale1;
						un_x = u * rscale2 + pivotX;
						un_y = v * rscale2 * sc + pivotY;
					}
					else
					{
						un_x = x;
						un_y = y;
					}

					nSrcX = CLAMP3(0, (int)un_x, right),
						nSrcY = CLAMP3(0, (int)un_y, bottom),
						nSrcX_1 = CLAMP3(0, nSrcX + 1, right),
						nSrcY_1 = CLAMP3(0, nSrcY + 1, bottom);

					px0 = src + nSrcY * p + 3 * nSrcX;
					px1 = src + nSrcY * p + 3 * nSrcX_1;
					px2 = src + nSrcY_1 * p + 3 * nSrcX;
					px3 = src + nSrcY_1 * p + 3 * nSrcX_1;

					fx = un_x - nSrcX;
					fy = un_y - nSrcY;

					for (i = 0; i < 3; i++)
					{
						m0 = px0[i] + fx * (px1[i] - px0[i]),
							m1 = px2[i] + fx * (px3[i] - px2[i]),
							my = m0 + fy * (m1 - m0);

						*d++ = CLAMP255(my);
					}
				}
			}
		}
	}
	else
	{
		for (y = blockTop; y < blockBottom; ++y)
		{
			d = dst + y * p + blockLeft * 3;
			for (x = blockLeft; x < blockRight; ++x)
			{
				u = x - pivotX;
				v = y - pivotY;
				r = sqrt(u * u + v * v);
				rscale1 = 1.0 - (r / maxrad);

				if (rscale1 > 0)
				{
					rscale2 = 1.0 - amount * rscale1 * rscale1;
					un_x = u * rscale2 + pivotX;
					un_y = v * rscale2 + pivotY;
				}
				else
				{
					un_x = x;
					un_y = y;
				}

				nSrcX = CLAMP3(0, (int)un_x, right),
					nSrcY = CLAMP3(0, (int)un_y, bottom),
					nSrcX_1 = CLAMP3(0, nSrcX + 1, right),
					nSrcY_1 = CLAMP3(0, nSrcY + 1, bottom);

				px0 = src + nSrcY * p + 3 * nSrcX;
				px1 = src + nSrcY * p + 3 * nSrcX_1;
				px2 = src + nSrcY_1 * p + 3 * nSrcX;
				px3 = src + nSrcY_1 * p + 3 * nSrcX_1;

				fx = un_x - nSrcX;
				fy = un_y - nSrcY;

				for (i = 0; i < 3; i++)
				{
					m0 = px0[i] + fx * (px1[i] - px0[i]),
						m1 = px2[i] + fx * (px3[i] - px2[i]),
						my = m0 + fy * (m1 - m0);

					*d++ = CLAMP255(my);
				}
			}
		}
	}

	return IMAGE_EFFECT_RESULT_OK;
}

int ImageEffect::blur(Sheet* srcImage, Sheet* dstImage,
	int blockLeft, int blockTop, int blockRight, int blockBottom) {
	return convolution(srcImage, dstImage, blurMatrix, 3, 3, blockLeft, blockTop, blockRight, blockBottom);
}

int ImageEffect::bump(Sheet* srcImage, Sheet* dstImage,
	int blockLeft, int blockTop, int blockRight, int blockBottom) {

	static float bumpMatrix[]
	{
		-1.0f, -1.0f, 0.0f,
			-1.0f, 1.0f, 1.0f,
			0.0f, 1.0f, 1.0f
	};

	return convolution(srcImage, dstImage, bumpMatrix, 3, 3, blockLeft, blockTop, blockRight, blockBottom);
}

int ImageEffect::crystalize(Sheet* srcImage, Sheet* dstImage, CrystalizeBlob* crystalizeBlob, CrystalizeMode crystalizeMode, bool shouldFadeEdges,
	double edgeThickness, double randomness, double distancePower, double scale,
	int blockLeft, int blockTop, int blockRight, int blockBottom) {

	VALIDATE_IMAGES();
	DECLARE_VARIABLES();

	edgeThickness = CLAMP3F(0., edgeThickness, 1.);
	randomness = CLAMP3F(0., randomness, 1.);
	distancePower = CLAMP3F(0.01, distancePower, 10.);
	scale = CLAMP3F(0.01, scale, 256.);

	if (!crystalizeBlob || crystalizeBlob->init())
		return IMAGE_EFFECT_RESULT_ERROR;

	float nx, ny, f, f1, f2, fx, fy, a, angle, D, angleCoefficient, gradientCoefficient;
	int i, ix, iy, bx, by, j;
	auto* results = crystalizeBlob->results;
	byte v[4], v2[4];
	byte edgeColor[4] = { 0, 0, 0, 0 };

	crystalizeBlob->start = 1;
	crystalizeBlob->mode = crystalizeMode;
	crystalizeBlob->edgeThickness = edgeThickness;
	crystalizeBlob->randomness = randomness;
	crystalizeBlob->distancePower = distancePower;
	crystalizeBlob->scale = scale;
	angleCoefficient = crystalizeBlob->angleCoefficient;
	gradientCoefficient = crystalizeBlob->gradientCoefficient;

	for (y = 0; y < h; ++y)
	{
		s = src + y * p;
		d = dst + y * p;
		for (x = 0; x < w; ++x)
		{
			nx = x;
			ny = y;
			nx /= scale;
			ny /= scale;
			nx += 1000;
			ny += 1000;

			results[0].distance = FLT_MAX;
			results[1].distance = FLT_MAX;
			results[2].distance = FLT_MAX;

			bx = (int)nx;
			by = (int)ny;
			fx = nx - bx;
			fy = ny - by;

			D = crystalizeBlob->checkCube(fx, fy, bx, by);
			if (D > fy)
				D = crystalizeBlob->checkCube(fx, fy + 1, bx, by - 1);
			if (D > 1 - fy)
				D = crystalizeBlob->checkCube(fx, fy - 1, bx, by + 1);
			if (D > fx) {
				crystalizeBlob->checkCube(fx + 1, fy, bx - 1, by);
				if (D > fy)
					D = crystalizeBlob->checkCube(fx + 1, fy + 1, bx - 1, by - 1);
				if (D > 1 - fy)
					D = crystalizeBlob->checkCube(fx + 1, fy - 1, bx - 1, by + 1);
			}
			if (D > 1 - fx) {
				D = crystalizeBlob->checkCube(fx - 1, fy, bx + 1, by);
				if (D > fy)
					D = crystalizeBlob->checkCube(fx - 1, fy + 1, bx + 1, by - 1);
				if (D > 1 - fy)
					D = crystalizeBlob->checkCube(fx - 1, fy - 1, bx + 1, by + 1);
			}

			f = 0;

			if (angleCoefficient != 0)
			{
				angle = atan2(ny - results[0].y, nx - results[0].x);
				if (angle < 0) angle += 2 * PI;
				angle /= 4 * PI;
				f += angleCoefficient * angle;
			}

			if (gradientCoefficient != 0) {
				a = 1 / (results[0].dy + results[0].dx);
				f += gradientCoefficient * a;
			}

			f1 = results[0].distance;
			f2 = results[1].distance;
			ix = CLAMP3(0, (results[0].x - 1000) * scale, right);
			iy = CLAMP3(0, (results[0].y - 1000) * scale, bottom);

			f = (f2 - f1) / edgeThickness;

			if (f < 0) f = 0;
			else if (f >= edgeThickness) f = 1;
			else
			{
				f = f / edgeThickness;
				f = f * f * (3 - 2 * f);
			}

			s = src + iy * p + 3 * ix;
			v[0] = s[0];
			v[1] = s[1];
			v[2] = s[2];

			if (shouldFadeEdges)
			{
				ix = CLAMP3(0, (results[1].x - 1000) * scale, right);
				iy = CLAMP3(0, (results[1].y - 1000) * scale, bottom);

				s = src + iy * p + 3 * ix;

				v2[0] = (byte)(s[0] + 0.5f * (v[0] - s[0]));
				v2[1] = (byte)(s[1] + 0.5f * (v[1] - s[1]));
				v2[2] = (byte)(s[2] + 0.5f * (v[2] - s[2]));

				*d++ = (byte)(v2[0] + f * (v[0] - v2[0]));
				*d++ = (byte)(v2[1] + f * (v[1] - v2[1]));
				*d++ = (byte)(v2[2] + f * (v[2] - v2[2]));
			}
			else
			{
				*d++ = (byte)(edgeColor[0] + f * (v[0] - edgeColor[0]));
				*d++ = (byte)(edgeColor[1] + f * (v[1] - edgeColor[1]));
				*d++ = (byte)(edgeColor[2] + f * (v[2] - edgeColor[2]));
			}
		}

	}

	return 2;
}

int ImageEffect::despeckle(Sheet* srcImage, Sheet* dstImage, size_t iteration,
	int blockLeft, int blockTop, int blockRight, int blockBottom) {

	VALIDATE_IMAGES();
	DECLARE_VARIABLES();

	iteration = CLAMP3(1, iteration, 20);

	pyte t, m, b;
	byte ob, og, or ;
	bool xIn, yIn;
	long ti = 0;

	memcpy(dst, src, h * p);

	while (iteration-- > 0)
	{
		for (y = 0; y < h; ++y)
		{
			s = src + y * p + blockLeft * 3;
			d = dst + y * p + blockLeft * 3;
			yIn = y > 0 && y < bottom;
			for (x = 0; x < w; ++x)
			{
				xIn = x > 0 && x < right;
				m = d;
				or = m[2];
				og = m[1];
				ob = m[0];
				t = b = 0;

				if (yIn) {
					t = dst + (y - 1) * p + 3 * x;
					b = dst + (y + 1) * p + 3 * x;
					or = pepperAndSalt(or , t[2], b[2]);
					og = pepperAndSalt(og, t[1], b[1]);
					ob = pepperAndSalt(ob, t[0], b[0]);
				}

				if (xIn) {
					or = pepperAndSalt(or , m[-1], m[5]);
					og = pepperAndSalt(og, m[-2], m[4]);
					ob = pepperAndSalt(ob, m[-3], m[3]);
				}

				if (yIn && xIn) {
					or = pepperAndSalt(or , t[-1], b[5]);
					og = pepperAndSalt(og, t[-2], b[4]);
					ob = pepperAndSalt(ob, t[-3], b[3]);

					or = pepperAndSalt(or , b[-1], t[5]);
					og = pepperAndSalt(og, b[-2], t[4]);
					ob = pepperAndSalt(ob, b[-3], t[3]);
				}

				*d++ = ob;
				*d++ = og;
				*d++ = or ;
			}

			ti++;
		}
	}

	return 2;
}

int ImageEffect::gussian_blur(Sheet* srcImage, Sheet* dstImage, size_t radius,
	int blockLeft, int blockTop, int blockRight, int blockBottom) {

	VALIDATE_IMAGES();
	DECLARE_VARIABLES();
	CLAMP_BLOCK();

	radius = CLAMP3(0, radius, 254);
	static unsigned char stack[514];
	memcpy(dst, src, h * p);
	memset(stack, 0, sizeof(stack));

	uint32_t xp, yp, i;
	uint32_t sp;
	uint32_t stack_start;
	byte* stack_ptr;
	byte* src_ptr;
	byte* dst_ptr;

	unsigned long sum_r;
	unsigned long sum_g;
	unsigned long sum_b;
	unsigned long sum_in_r;
	unsigned long sum_in_g;
	unsigned long sum_in_b;
	unsigned long sum_out_r;
	unsigned long sum_out_g;
	unsigned long sum_out_b;

	uint32_t wm = w - 1;
	uint32_t hm = h - 1;
	uint32_t div = (radius * 2) + 1;
	uint32_t mul_sum = stackblur_mul[radius];
	byte shr_sum = stackblur_shr[radius];

	int u = 0;

	for (y = 0; y < h; y++)
	{
		sum_r = sum_g = sum_b =
			sum_in_r = sum_in_g = sum_in_b =
			sum_out_r = sum_out_g = sum_out_b = 0;

		src_ptr = dst + p * y; // start of line (0,y)

		for (i = 0; i <= radius; i++)
		{
			stack_ptr = &stack[3 * i];
			stack_ptr[0] = src_ptr[0];
			stack_ptr[1] = src_ptr[1];
			stack_ptr[2] = src_ptr[2];

			sum_r += src_ptr[0] * (i + 1);
			sum_g += src_ptr[1] * (i + 1);
			sum_b += src_ptr[2] * (i + 1);

			sum_out_r += src_ptr[0];
			sum_out_g += src_ptr[1];
			sum_out_b += src_ptr[2];
		}


		for (i = 1; i <= radius; i++)
		{
			if (i <= wm) src_ptr += 3;
			stack_ptr = &stack[3 * (i + radius)];
			stack_ptr[0] = src_ptr[0];
			stack_ptr[1] = src_ptr[1];
			stack_ptr[2] = src_ptr[2];

			sum_r += src_ptr[0] * (radius + 1 - i);
			sum_g += src_ptr[1] * (radius + 1 - i);
			sum_b += src_ptr[2] * (radius + 1 - i);

			sum_in_r += src_ptr[0];
			sum_in_g += src_ptr[1];
			sum_in_b += src_ptr[2];
		}


		sp = radius;
		xp = radius;
		if (xp > wm) xp = wm;
		src_ptr = dst + y * p + 3 * xp; //   img.pix_ptr(xp, y);
		dst_ptr = dst + y * p; // img.pix_ptr(0, y);
		for (x = 0; x < w; x++)
		{
			dst_ptr[0] = byte((sum_r * mul_sum) >> shr_sum);
			dst_ptr[1] = byte((sum_g * mul_sum) >> shr_sum);
			dst_ptr[2] = byte((sum_b * mul_sum) >> shr_sum);

			dst_ptr += 3;

			sum_r -= sum_out_r;
			sum_g -= sum_out_g;
			sum_b -= sum_out_b;

			stack_start = sp + div - radius;
			if (stack_start >= div) stack_start -= div;
			stack_ptr = &stack[3 * stack_start];

			sum_out_r -= stack_ptr[0];
			sum_out_g -= stack_ptr[1];
			sum_out_b -= stack_ptr[2];

			if (xp < wm)
			{
				src_ptr += 3;
				++xp;
			}

			stack_ptr[0] = src_ptr[0];
			stack_ptr[1] = src_ptr[1];
			stack_ptr[2] = src_ptr[2];

			sum_in_r += src_ptr[0];
			sum_in_g += src_ptr[1];
			sum_in_b += src_ptr[2];
			sum_r += sum_in_r;
			sum_g += sum_in_g;
			sum_b += sum_in_b;

			++sp;
			if (sp >= div) sp = 0;
			stack_ptr = &stack[sp * 3];

			sum_out_r += stack_ptr[0];
			sum_out_g += stack_ptr[1];
			sum_out_b += stack_ptr[2];

			sum_in_r -= stack_ptr[0];
			sum_in_g -= stack_ptr[1];
			sum_in_b -= stack_ptr[2];
		}

		u++;
	}

	for (x = 0; x < w; x++)
	{
		sum_r = sum_g = sum_b =
			sum_in_r = sum_in_g = sum_in_b =
			sum_out_r = sum_out_g = sum_out_b = 0;

		src_ptr = dst + 3 * x; // x,0
		for (i = 0; i <= radius; i++)
		{
			stack_ptr = &stack[3 * i];
			stack_ptr[0] = src_ptr[0];
			stack_ptr[1] = src_ptr[1];
			stack_ptr[2] = src_ptr[2];

			sum_r += src_ptr[0] * (i + 1);
			sum_g += src_ptr[1] * (i + 1);
			sum_b += src_ptr[2] * (i + 1);

			sum_out_r += src_ptr[0];
			sum_out_g += src_ptr[1];
			sum_out_b += src_ptr[2];
		}
		for (i = 1; i <= radius; i++)
		{
			if (i <= hm) src_ptr += p; // +stride
			stack_ptr = &stack[3 * (i + radius)];
			stack_ptr[0] = src_ptr[0];
			stack_ptr[1] = src_ptr[1];
			stack_ptr[2] = src_ptr[2];

			sum_r += src_ptr[0] * (radius + 1 - i);
			sum_g += src_ptr[1] * (radius + 1 - i);
			sum_b += src_ptr[2] * (radius + 1 - i);

			sum_in_r += src_ptr[0];
			sum_in_g += src_ptr[1];
			sum_in_b += src_ptr[2];
		}

		sp = radius;
		yp = radius;
		if (yp > hm) yp = hm;
		src_ptr = dst + yp * p + 3 * x;
		dst_ptr = dst + 3 * x;
		for (y = 0; y < h; y++)
		{
			dst_ptr[0] = byte((sum_r * mul_sum) >> shr_sum);
			dst_ptr[1] = byte((sum_g * mul_sum) >> shr_sum);
			dst_ptr[2] = byte((sum_b * mul_sum) >> shr_sum);
			dst_ptr += p;

			sum_r -= sum_out_r;
			sum_g -= sum_out_g;
			sum_b -= sum_out_b;

			stack_start = sp + div - radius;
			if (stack_start >= div) stack_start -= div;
			stack_ptr = &stack[3 * stack_start];

			sum_out_r -= stack_ptr[0];
			sum_out_g -= stack_ptr[1];
			sum_out_b -= stack_ptr[2];

			if (yp < hm)
			{
				src_ptr += p; // stride
				++yp;
			}

			stack_ptr[0] = src_ptr[0];
			stack_ptr[1] = src_ptr[1];
			stack_ptr[2] = src_ptr[2];

			sum_in_r += src_ptr[0];
			sum_in_g += src_ptr[1];
			sum_in_b += src_ptr[2];

			sum_r += sum_in_r;
			sum_g += sum_in_g;
			sum_b += sum_in_b;

			++sp;
			if (sp >= div) sp = 0;
			stack_ptr = &stack[sp * 3];

			sum_out_r += stack_ptr[0];
			sum_out_g += stack_ptr[1];
			sum_out_b += stack_ptr[2];

			sum_in_r -= stack_ptr[0];
			sum_in_g -= stack_ptr[1];
			sum_in_b -= stack_ptr[2];
		}

		u++;
	}

	return IMAGE_EFFECT_RESULT_WHOLE_IMAGE;
}

int ImageEffect::maximum(Sheet* srcImage, Sheet* dstImage,
	int blockLeft, int blockTop, int blockRight, int blockBottom) {
	VALIDATE_IMAGES();
	DECLARE_VARIABLES();
	CLAMP_BLOCK();

	int ix, iy, r, g, b, ioffset, moffset, row, col, rowMin, rowMax, colMin, colMax;

	auto rowCount = 3;
	auto columnCount = 3;

	rowMin = -int(rowCount / 2);
	rowMax = rowMin + int(rowCount);
	colMin = -int(columnCount / 2);
	colMax = colMin + int(columnCount);

	for (y = blockTop; y < blockBottom; ++y)
	{
		s = src + y * p + blockLeft * 3;
		d = dst + y * p + blockLeft * 3;
		for (x = blockLeft; x < blockRight; ++x)
		{
			r = 0, g = 0, b = 0;

			for (row = rowMin; row < rowMax; row++)
			{
				iy = y + row;
				if (0 <= iy && iy < bottom) ioffset = iy * p;
				else ioffset = y * p;

				moffset = columnCount * (row - rowMin) - colMin;

				for (col = colMin; col < colMax; col++)
				{
					ix = x + col;
					if (0 > ix || ix > right) ix = x;
					s = src + ioffset + 3 * ix;

					r = Max(s[2], r);
					g = Max(s[1], g);
					b = Max(s[0], b);
				}
			}

			*d++ = CLAMP255(b);
			*d++ = CLAMP255(g);
			*d++ = CLAMP255(r);
		}
	}

	return IMAGE_EFFECT_RESULT_OK;
}

int ImageEffect::minimum(Sheet* srcImage, Sheet* dstImage,
	int blockLeft, int blockTop, int blockRight, int blockBottom) {
	VALIDATE_IMAGES();
	DECLARE_VARIABLES();
	CLAMP_BLOCK();

	int ix, iy, r, g, b, ioffset, moffset, row, col, rowMin, rowMax, colMin, colMax;

	auto rowCount = 3;
	auto columnCount = 3;

	rowMin = -int(rowCount / 2);
	rowMax = rowMin + int(rowCount);
	colMin = -int(columnCount / 2);
	colMax = colMin + int(columnCount);

	for (y = blockTop; y < blockBottom; ++y)
	{
		s = src + y * p + blockLeft * 3;
		d = dst + y * p + blockLeft * 3;
		for (x = blockLeft; x < blockRight; ++x)
		{
			r = 0, g = 0, b = 0;

			for (row = rowMin; row < rowMax; row++)
			{
				iy = y + row;
				if (0 <= iy && iy < bottom) ioffset = iy * p;
				else ioffset = y * p;

				moffset = columnCount * (row - rowMin) - colMin;

				for (col = colMin; col < colMax; col++)
				{
					ix = x + col;
					if (0 > ix || ix > right) ix = x;
					s = src + ioffset + 3 * ix;

					r = Min(s[2], r);
					g = Min(s[1], g);
					b = Min(s[0], b);
				}
			}

			*d++ = CLAMP255(b);
			*d++ = CLAMP255(g);
			*d++ = CLAMP255(r);
		}
	}

	return IMAGE_EFFECT_RESULT_OK;
}

int ImageEffect::sharpen(Sheet* srcImage, Sheet* dstImage,
	int blockLeft, int blockTop, int blockRight, int blockBottom) {
	static float sharpenMatrix[] =
	{
		0.0f, -0.2f, 0.0f,
		-0.2f, 1.8f, -0.2f,
		0.0f, -0.2f, 0.0f
	};
	return convolution(srcImage, dstImage, sharpenMatrix, 3, 3, blockLeft, blockTop, blockRight, blockBottom);
}

int ImageEffect::reduce_noise(Sheet* srcImage, Sheet* dstImage,
	int blockLeft, int blockTop, int blockRight, int blockBottom) {
	VALIDATE_IMAGES();
	DECLARE_VARIABLES();
	CLAMP_BLOCK();

	int r, g, b, r2, b2, g2, row, col;
	int ioffset, iy, ix, moffset;
	float hf;
	byte c;
	byte red[9], grn[9], blu[9];

	for (y = blockTop; y < blockBottom; ++y)
	{
		s = src + y * p + blockLeft * 3;
		d = dst + y * p + blockLeft * 3;
		for (x = blockLeft; x < blockRight; ++x)
		{
			r = 0, g = 0, b = 0;

			for (row = -1; row < 2; row++)
			{
				iy = y + row;
				ioffset;
				if (0 <= iy && iy < bottom) ioffset = iy * p;
				else ioffset = y * p;

				moffset = 3 * (row + 1) + 1;

				for (col = -1; col < 2; col++)
				{
					ix = x + col;
					if (0 > ix || ix > right) ix = x;
					s = src + ioffset + 3 * ix;

					blu[moffset + col] = s[0];
					grn[moffset + col] = s[1];
					red[moffset + col] = s[2];
				}
			}

			*d++ = smooth(blu);
			*d++ = smooth(grn);
			*d++ = smooth(red);
		}
	}

	return IMAGE_EFFECT_RESULT_OK;
}

int ImageEffect::tile_glass(Sheet* srcImage, Sheet* dstImage, double size, double amount, double angle, double pivotX, double pivotY,
	int blockLeft, int blockTop, int blockRight, int blockBottom) {

	VALIDATE_IMAGES();
	DECLARE_VARIABLES();
	CLAMP_BLOCK();

	size = CLAMP3F(2., size, 1000.);
	amount = CLAMP3F(-20., amount, 20.);
	angle = CLAMP3F(-PI, angle, PI);
	pivotX = CLAMP3F(0., pivotX, 1.);
	pivotY = CLAMP3F(0., pivotY, 1.);

	double u, v, z, t, icos, isin;
	int xSample, ySample;
	double i, j, scale, curvature;
	int r, g, b, a;
	icos = cos(angle);
	isin = sin(angle);

	struct FPOINT
	{
		double   x;
		double   y;
	};

#define aasamples  17
	FPOINT aapt[aasamples];

	isin = sin(angle);
	icos = cos(angle);
	scale = PI / size;
	amount = amount == 0 ? 1 : amount;
	curvature = amount * amount / 10.0 * (abs(amount) / (double)amount);

	pivotX = w * pivotX;
	pivotY = h * pivotY;

	for (int i = 0; i < aasamples; i++)
	{
		double  x = (i * 4) / (double)aasamples,
			y = i / (double)aasamples;
		x = x - (int)x;
		aapt[i].x = icos * x + isin * y;
		aapt[i].y = icos * y - isin * x;
	}

	for (y = blockTop; y < blockBottom; ++y)
	{
		s = src + y * p + blockLeft * 3;
		d = dst + y * p + blockLeft * 3;
		for (x = blockLeft; x < blockRight; ++x)
		{
			i = (double)x - pivotX;
			j = (double)y - pivotY;

			b = 0, g = 0, r = 0, a = 0;
			for (int mm = 0; mm < aasamples; mm++)
			{
				u = i + aapt[mm].x;
				v = j - aapt[mm].y;

				z = icos * u + isin * v;
				t = -isin * u + icos * v;

				z += curvature * tan(z * scale);
				t += curvature * tan(t * scale);
				u = icos * z - isin * t;
				v = isin * z + icos * t;

				xSample = (int)(pivotX + u);
				ySample = (int)(pivotY + v);

				xSample = CLAMP3(0, xSample, right);
				ySample = CLAMP3(0, ySample, bottom);

				s = src + ySample * p + 3 * xSample;
				b += s[0];
				g += s[1];
				r += s[2];
			}

			*d++ = CLAMP255(b / aasamples);
			*d++ = CLAMP255(g / aasamples);
			*d++ = CLAMP255(r / aasamples);

		}

	}

	return IMAGE_EFFECT_RESULT_OK;
}

int ImageEffect::unsharp(Sheet* srcImage, Sheet* dstImage,
	int blockLeft, int blockTop, int blockRight, int blockBottom) {
	return convolution(srcImage, dstImage, blurMatrix, 3, 3, blockLeft, blockTop, blockRight, blockBottom);
}

int ImageEffect::wave(Sheet* srcImage, Sheet* dstImage, double waveLength, double amplitude, double pivotX, double pivotY,
	int blockLeft, int blockTop, int blockRight, int blockBottom) {

	VALIDATE_IMAGES();
	DECLARE_VARIABLES();
	CLAMP_BLOCK();

	waveLength = CLAMP3F(2., waveLength, 200.);
	amplitude = CLAMP3F(1., amplitude, 50.);
	pivotX = CLAMP3F(0., pivotX, 1.);
	pivotY = CLAMP3F(0., pivotY, 1.);

	double un_x, un_y, amnt, dx, dy, fScaleX, fScaleY;
	int nSrcX, nSrcY, nSrcX_1, nSrcY_1;
	double u, v, fx, fy;
	pyte px0, px1, px2, px3;
	double m0, m1, my;
	fScaleX = 1.0, fScaleY = 1.0;
	if (w < h) fScaleX = (double)w / (double)h;
	else if (w > h) fScaleY = (double)w / (double)h;

	pivotX = w * pivotX;
	pivotY = h * pivotY;

	for (y = blockTop; y < blockBottom; ++y)
	{
		s = src + y * p + blockLeft * 3;
		d = dst + y * p + blockLeft * 3;
		for (x = blockLeft; x < blockRight; ++x)
		{
			u = (x - w);
			v = (y - h);

			dx = (x - pivotX) * fScaleX,
				dy = (y - pivotY) * fScaleY,
				amnt = amplitude * sin(2 * PI * sqrt(dx * dx + dy * dy) / waveLength);

			un_x = (amnt + dx) / fScaleX + pivotX;
			un_y = (amnt + dy) / fScaleY + pivotY;
			nSrcX = CLAMP3(0, (int)un_x, right);
			nSrcY = CLAMP3(0, (int)un_y, bottom);

			nSrcX_1 = CLAMP3(0, nSrcX + 1, right);
			nSrcY_1 = CLAMP3(0, nSrcY + 1, bottom);

			px0 = src + nSrcY * p + 3 * nSrcX;
			px1 = src + nSrcY * p + 3 * nSrcX_1;
			px2 = src + nSrcY_1 * p + 3 * nSrcX;
			px3 = src + nSrcY_1 * p + 3 * nSrcX_1;

			fx = un_x - nSrcX;
			fy = un_y - nSrcY;

			for (int i = 0; i < 3; i++)
			{
				m0 = px0[i] + fx * (px1[i] - px0[i]),
					m1 = px2[i] + fx * (px3[i] - px2[i]),
					my = m0 + fy * (m1 - m0);
				*d++ = CLAMP255(my);
			}
		}
	}

	return IMAGE_EFFECT_RESULT_OK;
}

int ImageEffect::water(Sheet* srcImage, Sheet* dstImage, double telorance, double scale, double pivotX, double pivotY,
	int blockLeft, int blockTop, int blockRight, int blockBottom) {
	VALIDATE_IMAGES();
	DECLARE_VARIABLES();
	CLAMP_BLOCK();

	telorance = CLAMP3F(0., telorance, 1000.);
	scale = CLAMP3F(0., scale, 100.);
	pivotX = CLAMP3F(0., pivotX, 1.);
	pivotY = CLAMP3F(0., pivotY, 1.);

	float sinTable[256], cosTable[256], turbulence, xScale, yScale, angle, rx0, rx1, ry0, ry1, * q, a, b,
		ssx, ssy, t, u, v, f, fx, fy, cx, cy;
	int bx0, bx1, by0, by1, b00, b10, b01, b11, j, srcX, srcY;
	pyte nw, sw, ne, se;
	float xWeight, yWeight;
	float wavelength = 32;
	float amplitude = scale / 100.0;
	float phase = 1;
	float radius = 50;
	float radius2 = 0;
	int icentreX;
	int icentreY;
	float out[2];
	float m0, m1;
	icentreX = w * pivotX;
	icentreY = h * pivotY;
	radius = telorance;
	radius2 = radius * radius;
	float dx, dy, distance2;
	float amount, distance;
	int i;

	for (y = blockTop; y < blockBottom; ++y)
	{
		s = src + y * p + blockLeft * 3;
		d = dst + y * p + blockLeft * 3;
		for (x = blockLeft; x < blockRight; ++x)
		{

			dx = x - icentreX;
			dy = y - icentreY;
			distance2 = dx * dx + dy * dy;

			if (distance2 > radius2) {
				out[0] = x;
				out[1] = y;
			}
			else {
				distance = (float)sqrt(distance2);
				amount = amplitude * (float)sin(distance / wavelength * PI * 2 - phase);
				amount *= (radius - distance) / radius;
				if (distance != 0)
					amount *= wavelength / distance;
				out[0] = x + dx * amount;
				out[1] = y + dy * amount;
			}

			srcX = (int)floor(out[0]);
			srcY = (int)floor(out[1]);
			xWeight = out[0] - srcX;
			yWeight = out[1] - srcY;

			if (srcX >= 0 && srcX < right && srcY >= 0 && srcY < bottom)
			{
				i = p * srcY + 3 * srcX;
				nw = src + i;
				ne = src + i + 3;
				sw = src + i + p;
				se = src + i + p + 3;
			}
			else
			{
				nw = src + CLAMP3(0, srcY, bottom) * p + 3 * CLAMP3(0, srcX, right);
				ne = src + CLAMP3(0, srcY, bottom) * p + 3 * CLAMP3(0, srcX + 1, right);
				sw = src + CLAMP3(0, srcY + 1, bottom) * p + 3 * CLAMP3(0, srcX, right);
				se = src + CLAMP3(0, srcY + 1, bottom) * p + 3 * CLAMP3(0, srcX + 1, right);
			}

			cx = 1.0f - xWeight;
			cy = 1.0f - yWeight;

			m0 = cx * nw[0] + xWeight * ne[0];
			m1 = cx * sw[0] + xWeight * se[0];
			*d++ = (byte)(cy * m0 + yWeight * m1);

			m0 = cx * nw[1] + xWeight * ne[1];
			m1 = cx * sw[1] + xWeight * se[1];
			*d++ = (byte)(cy * m0 + yWeight * m1);

			m0 = cx * nw[2] + xWeight * ne[2];
			m1 = cx * sw[2] + xWeight * se[2];
			*d++ = (byte)(cy * m0 + yWeight * m1);
		}
	}

	return IMAGE_EFFECT_RESULT_OK;
}

int ImageEffect::glow(Sheet* srcImage, Sheet* dstImage, GlowBlob* blob, int softness, int brightness, int contrast,
	int blockLeft, int blockTop, int blockRight, int blockBottom) {
	VALIDATE_IMAGES();
	DECLARE_VARIABLES();
	CLAMP_BLOCK();

	if (!blob || blob->init(softness, brightness, contrast))
		return 1;

	int i, j, r, wx, wy, wp, wwx, srcX, srcY, wr;
	int64_t wwxx, waSum, wcSum, aSum, bSum, gSum, rSum, * rSums, * gSums, * bSums, * waSums, * aSums, * wcSums;
	pyte c, ptr;
	int alpha, red, green, blue, wlen;
	uint64_t arraysLength;

	auto lookup = blob->gray;
	auto weights = blob->weights;
	auto localStore = blob->localStore;

	r = softness;
	wlen = 1 + (r * 2);
	auto localStoreSize = sizeof(blob->localStore);
	ptr = localStore;

	waSums = (int64_t*)ptr;
	ptr += wlen * sizeof(int64_t);

	wcSums = (int64_t*)ptr;
	ptr += wlen * sizeof(int64_t);

	aSums = (int64_t*)ptr;
	ptr += wlen * sizeof(int64_t);

	bSums = (int64_t*)ptr;
	ptr += wlen * sizeof(int64_t);

	gSums = (int64_t*)ptr;
	ptr += wlen * sizeof(int64_t);

	rSums = (int64_t*)ptr;
	ptr += wlen * sizeof(int64_t);

	arraysLength = (uint64_t)(sizeof(int64_t) * wlen);

	for (y = 0; y < h; ++y)
	{
		s = src + y * p;
		d = dst + y * p;

		memset(localStore, 0, (uint64_t)localStoreSize);

		waSum = 0;
		wcSum = 0;
		bSum = 0;
		gSum = 0;
		rSum = 0;

		for (wx = 0; wx < wlen; ++wx)
		{
			srcX = 0 + wx - r;
			waSums[wx] = 0;
			wcSums[wx] = 0;
			bSums[wx] = 0;
			gSums[wx] = 0;
			rSums[wx] = 0;

			if (srcX >= 0 && srcX < w)
			{
				for (wy = 0; wy < wlen; ++wy)
				{
					srcY = y + wy - r;

					if (srcY >= 0 && srcY < h)
					{
						c = src + srcY * p + 3 * srcX;
						wp = weights[wy];

						waSums[wx] += wp;
						wp *= 256;
						wcSums[wx] += wp;
						wp >>= 8;

						bSums[wx] += wp * c[0];
						gSums[wx] += wp * c[1];
						rSums[wx] += wp * c[2];
					}
				}

				wwx = weights[wx];
				waSum += wwx * waSums[wx];
				wcSum += wwx * wcSums[wx];
				bSum += wwx * bSums[wx];
				gSum += wwx * gSums[wx];
				rSum += wwx * rSums[wx];
			}
		}

		wcSum >>= 8;

		if (waSum == 0 || wcSum == 0)
		{
			*d++ = 255 - (255 - *s++) * (255 - lookup[0]) / 255;
			*d++ = 255 - (255 - *s++) * (255 - lookup[0]) / 255;
			*d++ = 255 - (255 - *s++) * (255 - lookup[0]) / 255;
		}
		else
		{
			*d++ = 255 - (255 - *s++) * (255 - lookup[CLAMP255(bSum / wcSum)]) / 255;
			*d++ = 255 - (255 - *s++) * (255 - lookup[CLAMP255(gSum / wcSum)]) / 255;
			*d++ = 255 - (255 - *s++) * (255 - lookup[CLAMP255(rSum / wcSum)]) / 255;
		}

		for (x = 1; x < w; ++x)
		{
			for (i = 0; i < wlen - 1; ++i)
			{
				waSums[i] = waSums[i + 1];
				wcSums[i] = wcSums[i + 1];
				bSums[i] = bSums[i + 1];
				gSums[i] = gSums[i + 1];
				rSums[i] = rSums[i + 1];
			}

			waSum = 0;
			wcSum = 0;
			bSum = 0;
			gSum = 0;
			rSum = 0;

			for (wx = 0; wx < wlen - 1; ++wx)
			{
				wwxx = (int64_t)weights[wx];
				waSum += wwxx * waSums[wx];
				wcSum += wwxx * wcSums[wx];
				bSum += wwxx * bSums[wx];
				gSum += wwxx * gSums[wx];
				rSum += wwxx * rSums[wx];
			}

			wx = wlen - 1;

			waSums[wx] = 0;
			wcSums[wx] = 0;
			bSums[wx] = 0;
			gSums[wx] = 0;
			rSums[wx] = 0;

			srcX = x + wx - r;

			if (srcX >= 0 && srcX < w)
			{
				for (wy = 0; wy < wlen; ++wy)
				{
					srcY = y + wy - r;

					if (srcY >= 0 && srcY < h)
					{
						c = src + srcY * p + 3 * srcX;
						wp = weights[wy];

						waSums[wx] += wp;
						wp *= 256;
						wcSums[wx] += wp;
						wp >>= 8;

						bSums[wx] += wp * (int64_t)c[0];
						gSums[wx] += wp * (int64_t)c[1];
						rSums[wx] += wp * (int64_t)c[2];
					}
				}

				wr = weights[wx];
				waSum += (int64_t)wr * waSums[wx];
				wcSum += (int64_t)wr * wcSums[wx];
				bSum += (int64_t)wr * bSums[wx];
				gSum += (int64_t)wr * gSums[wx];
				rSum += (int64_t)wr * rSums[wx];
			}

			wcSum >>= 8;

			if (waSum == 0 || wcSum == 0)
			{
				*d++ = 255 - (255 - *s++) * (255 - lookup[0]) / 255;
				*d++ = 255 - (255 - *s++) * (255 - lookup[0]) / 255;
				*d++ = 255 - (255 - *s++) * (255 - lookup[0]) / 255;
			}
			else
			{
				*d++ = 255 - (255 - *s++) * (255 - lookup[CLAMP255(bSum / wcSum)]) / 255;
				*d++ = 255 - (255 - *s++) * (255 - lookup[CLAMP255(gSum / wcSum)]) / 255;
				*d++ = 255 - (255 - *s++) * (255 - lookup[CLAMP255(rSum / wcSum)]) / 255;
			}
		}

	}

	return 2;
}

int ImageEffect::marble(Sheet* srcImage, Sheet* dstImage, MarbleBlob* blob, double telorance, double scale,
	int blockLeft, int blockTop, int blockRight, int blockBottom) {
	VALIDATE_IMAGES();
	DECLARE_VARIABLES();
	CLAMP_BLOCK();

	if (!blob || blob->init(telorance, scale))
		return IMAGE_EFFECT_RESULT_ERROR;

	float xScale, yScale, rx0, rx1, ry0, ry1, * q, a, b,
		ssx, ssy, t, u, v, f, fx, fy, cx, cy;
	int i, bx0, bx1, by0, by1, b00, b10, b01, b11, j;
	int displacement, srcX, srcY;
	float xWeight, yWeight, m0, m1;
	pyte nw, sw, se, ne;

	auto out = blob->out;
	auto sinTable = blob->sinTable;
	auto cosTable = blob->cosTable;
	auto pp = blob->pp;
	auto g3 = blob->g3;
	auto g2 = blob->g2;
	auto g1 = blob->g1;
	xScale = yScale = scale;

	for (y = blockTop; y < blockBottom; ++y)
	{
		s = src + y * p + blockLeft * 3;
		d = dst + y * p + blockLeft * 3;
		fy = y / yScale;

		for (x = blockLeft; x < blockRight; ++x)
		{
			fx = x / xScale;

			t = fx + 4096;
			bx0 = ((int)t) & 255;
			bx1 = (bx0 + 1) & 255;
			rx0 = t - (int)t;
			rx1 = rx0 - 1.0f;
			t = fy + 4096;
			by0 = ((int)t) & 255;
			by1 = (by0 + 1) & 255;
			ry0 = t - (int)t;
			ry1 = ry0 - 1.0f;
			i = pp[bx0];
			j = pp[bx1];
			b00 = pp[i + by0];
			b10 = pp[j + by0];
			b01 = pp[i + by1];
			b11 = pp[j + by1];
			ssx = rx0 * rx0 * (3.0f - 2.0f * rx0);
			ssy = ry0 * ry0 * (3.0f - 2.0f * ry0);
			q = g2[b00]; u = rx0 * q[0] + ry0 * q[1];
			q = g2[b10]; v = rx1 * q[0] + ry0 * q[1];
			a = u + ssx * (v - u);
			q = g2[b01]; u = rx0 * q[0] + ry1 * q[1];
			q = g2[b11]; v = rx1 * q[0] + ry1 * q[1];
			b = u + ssx * (v - u);

			f = 1.5f * (a + ssy * (b - a));

			displacement = CLAMP255((int)(127 * (1 + f)));
			out[0] = x + cosTable[displacement];
			out[1] = y + sinTable[displacement];

			srcX = (int)floor(out[0]);
			srcY = (int)floor(out[1]);
			xWeight = out[0] - srcX;
			yWeight = out[1] - srcY;

			if (srcX >= 0 && srcX < right && srcY >= 0 && srcY < bottom)
			{
				i = p * srcY + 3 * srcX;
				nw = src + i;
				ne = src + i + 3;
				sw = src + i + p;
				se = src + i + p + 3;
			}
			else
			{
				nw = src + CLAMP3(0, srcY, bottom) * p + 3 * CLAMP3(0, srcX, right);
				ne = src + CLAMP3(0, srcY, bottom) * p + 3 * CLAMP3(0, srcX + 1, right);
				sw = src + CLAMP3(0, srcY + 1, bottom) * p + 3 * CLAMP3(0, srcX, right);
				se = src + CLAMP3(0, srcY + 1, bottom) * p + 3 * CLAMP3(0, srcX + 1, right);
			}

			cx = 1.0f - xWeight;
			cy = 1.0f - yWeight;

			m0 = cx * nw[0] + xWeight * ne[0];
			m1 = cx * sw[0] + xWeight * se[0];
			*d++ = (byte)(cy * m0 + yWeight * m1);

			m0 = cx * nw[1] + xWeight * ne[1];
			m1 = cx * sw[1] + xWeight * se[1];
			*d++ = (byte)(cy * m0 + yWeight * m1);

			m0 = cx * nw[2] + xWeight * ne[2];
			m1 = cx * sw[2] + xWeight * se[2];
			*d++ = (byte)(cy * m0 + yWeight * m1);
		}
	}

	return IMAGE_EFFECT_RESULT_OK;
}

int ImageEffect::median(Sheet* srcImage, Sheet* dstImage, MedianBlob* blob, size_t radius, size_t percentile,
	int blockLeft, int blockTop, int blockRight, int blockBottom) {
	VALIDATE_IMAGES();
	DECLARE_VARIABLES();
	CLAMP_BLOCK();

	if (!blob || blob->init(radius, percentile))
		return 1;

	auto hSize = sizeof(blob->ha);
	int u, v, left, area, top, minCount1, minCount2, bCount, gCount, rCount, b1, b2, g1, g2, r1, r2;
	int minCount, g, r, b;

	auto rad = radius;
	auto precent = percentile;
	auto cutoff = ((rad * 2 + 1) * (rad * 2 + 1) + 2) / 4;
	auto leadingEdgeX = blob->leadingEdgeX;
	auto ha = blob->ha;
	auto hr = blob->hr;
	auto hg = blob->hg;
	auto hb = blob->hb;

	LR psamp, px, ps, pd;

	for (y = 0; y < h; y++)
	{
		memset(hb, 0, hSize);
		memset(hg, 0, hSize);
		memset(hr, 0, hSize);

		area = 0;

		ps = (LR)(src + y * p);
		pd = (LR)(dst + y * p);

		top = -Min(rad, y);
		bottom = Min(rad, h - 1 - y);
		left = -Min(rad, 0);
		right = Min(rad, w - 1);

		for (v = top; v <= bottom; ++v)
		{
			psamp = (LR)(src + (y + v) * p + 3 * left);

			for (u = left; u <= right; ++u)
			{
				if ((u * u + v * v) <= cutoff)
				{
					++area;
					++hb[psamp->B];
					++hg[psamp->G];
					++hr[psamp->R];
				}

				++psamp;
			}
		}

		for (x = 0; x < w; x++)
		{
			int minCount = area * precent / 100;

			int b = 0;
			int bCount = 0;

			while (b < 255 && hb[b] == 0)
			{
				++b;
			}

			while (b < 255 && bCount < minCount)
			{
				bCount += hb[b];
				++b;
			}

			int g = 0;
			int gCount = 0;

			while (g < 255 && hg[g] == 0)
			{
				++g;
			}

			while (g < 255 && gCount < minCount)
			{
				gCount += hg[g];
				++g;
			}

			int r = 0;
			int rCount = 0;

			while (r < 255 && hr[r] == 0)
			{
				++r;
			}

			while (r < 255 && rCount < minCount)
			{
				rCount += hr[r];
				++r;
			}

			pd->B = (byte)b;
			pd->G = (byte)g;
			pd->R = (byte)r;

			left = -Min(rad, x);
			right = Min(rad + 1, w - 1 - x);

			v = -1;
			while (v >= top)
			{
				u = leadingEdgeX[-v];
				if (-u >= left) break;
				--v;
			}

			while (v >= top)
			{
				u = leadingEdgeX[-v];
				px = (LR)(pyte(ps) + v * p - u * 3);

				--hb[px->B];
				--hg[px->G];
				--hr[px->R];
				--area;

				--v;
			}

			v = -1;
			while (v >= top)
			{
				u = leadingEdgeX[-v];
				if (u + 1 <= right) break;
				--v;
			}

			while (v >= top)
			{
				u = leadingEdgeX[-v];
				px = (LR)(pyte(ps) + v * p + (u + 1) * 3);

				++hb[px->B];
				++hg[px->G];
				++hr[px->R];
				++area;

				--v;
			}

			v = 0;
			while (v <= bottom)
			{
				u = leadingEdgeX[v];
				if (-u >= left) break;
				++v;
			}

			while (v <= bottom)
			{
				u = leadingEdgeX[v];
				px = (LR)(pyte(ps) + v * p - u * 3);

				--hb[px->B];
				--hg[px->G];
				--hr[px->R];
				--area;

				++v;
			}

			v = 0;
			while (v <= bottom) {
				u = leadingEdgeX[v];
				if (u + 1 <= right)
					break;
				++v;
			}

			while (v <= bottom)
			{
				u = leadingEdgeX[v];
				px = (LR)(pyte(ps) + v * p + (u + 1) * 3);

				++hb[px->B];
				++hg[px->G];
				++hr[px->R];
				++area;

				++v;
			}

			++ps;
			++pd;
		}
	}

	return 2;
}

int ImageEffect::motion_blur(Sheet* srcImage, Sheet* dstImage, size_t size, double angle,
	int blockLeft, int blockTop, int blockRight, int blockBottom) {
	VALIDATE_IMAGES();
	DECLARE_VARIABLES();
	CLAMP_BLOCK();

	size = CLAMP3(3, size, 2000);
	angle = CLAMP3F(-PI, angle, PI);

	double scx, scy, rx, ry, fx, fy;
	int i, sb, sg, sr, sa, ix, iy, wx, wy, xx, yy;
	int tx, ty, dx, dy, sc;

	auto distance = size;
	dx = (int)(distance * cos(angle));
	dy = (int)(distance * sin(angle));

	scx = (dx / (double)distance);
	scy = (dy / (double)distance);

	tx = (dx / 2.0 + (scx > 0 ? +0.5 : -0.5));
	ty = (dy / 2.0 + (scy > 0 ? +0.5 : -0.5));
	distance = size;

	for (y = blockTop; y < blockBottom; ++y)
	{
		d = dst + y * p + blockLeft * 3;
		for (x = blockLeft; x < blockRight; ++x)
		{
			sr = 0, sg = 0, sb = 0;

			wy = y - ty;
			wx = x - tx;

			for (i = 0; i < distance; i++)
			{
				rx = i * scx;
				ry = i * scy;

				iy = wy + ry + 0.5;
				ix = wx + rx + 0.5;

				xx = CLAMP3(0, ix, right);
				yy = CLAMP3(0, iy, bottom);

				s = src + yy * p + 3 * xx;

				sb += s[0];
				sg += s[1];
				sr += s[2];
			}

			sb = CLAMP255(sb / distance);
			sg = CLAMP255(sg / distance);
			sr = CLAMP255(sr / distance);

			*d++ = sb;
			*d++ = sg;
			*d++ = sr;
		}
	}

	return IMAGE_EFFECT_RESULT_OK;
}

int ImageEffect::old_paint(Sheet* srcImage, Sheet* dstImage, OldPaintBlob* blob, size_t brushSize, size_t coareness,
	int blockLeft, int blockTop, int blockRight, int blockBottom) {
	VALIDATE_IMAGES();
	DECLARE_VARIABLES();
	CLAMP_BLOCK();

	brushSize = CLAMP3F(1., brushSize, 100.);
	coareness = CLAMP3F(3., coareness, 255.);

	if (!blob) return IMAGE_EFFECT_RESULT_ERROR;

	int arrayLens = 1 + coareness;
	int localStoreSize = arrayLens * 5 * sizeof(int);
	int i, top, left, numInt, maxInstance, j, r1;
	byte chosenIntensity, intensity;
	pyte localStore = (pyte)blob->localStore;
	pyte pt = localStore;

	int* intensityCount = (int*)pt;
	pt += arrayLens * sizeof(int);

	uint32_t* avgRed = (uint32_t*)pt;
	pt += arrayLens * sizeof(uint32_t);

	uint32_t* avgGreen = (uint32_t*)pt;
	pt += arrayLens * sizeof(uint32_t);

	uint32_t* avgBlue = (uint32_t*)pt;
	pt += arrayLens * sizeof(uint32_t);

	uint32_t* avgAlpha = (uint32_t*)pt;
	pt += arrayLens * sizeof(uint32_t);

	byte maxIntensity = coareness;

	for (y = blockTop; y < blockBottom; ++y)
	{
		d = dst + y * p + blockLeft * 3;

		top = y - brushSize;
		bottom = y + brushSize + 1;

		if (top < 0) top = 0;
		if (bottom > h) bottom = h;

		for (x = blockLeft; x < blockRight; ++x)
		{
			memset(localStore, 0, localStoreSize);

			left = x - brushSize;
			right = x + brushSize + 1;

			if (left < 0) left = 0;
			if (right > w) right = w;

			numInt = 0;

			for (j = top; j < bottom; ++j)
			{
				s = src + j * p + 3 * left;

				for (i = left; i < right; ++i)
				{
					intensity = (7471 * s[0] + 38470 * s[1] + 19595 * s[2]) >> 16;
					r1 = intensity * maxIntensity + 0x80;
					intensity = ((r1 >> 8) + r1) >> 8;

					++intensityCount[intensity];
					++numInt;

					avgRed[intensity] += s[2];
					avgGreen[intensity] += s[1];
					avgBlue[intensity] += s[0];

					s += 3;
				}
			}

			chosenIntensity = 0;
			maxInstance = 0;

			for (i = 0; i <= maxIntensity; ++i)
			{
				if (intensityCount[i] > maxInstance)
				{
					chosenIntensity = i;
					maxInstance = intensityCount[i];
				}
			}

			*d++ = avgBlue[chosenIntensity] / maxInstance;
			*d++ = avgGreen[chosenIntensity] / maxInstance;
			*d++ = avgRed[chosenIntensity] / maxInstance;
		}
	}

	return IMAGE_EFFECT_RESULT_OK;
}

int ImageEffect::outline(Sheet* srcImage, Sheet* dstImage, OutlineBlob* blob, size_t radius, size_t intensity,
	int blockLeft, int blockTop, int blockRight, int blockBottom) {
	VALIDATE_IMAGES();
	DECLARE_VARIABLES();
	CLAMP_BLOCK();

	if (!blob || blob->init(radius, intensity))
		return 1;

	auto hSize = sizeof(blob->ha);
	int u, v, left, area, top, minCount1, minCount2, bCount, gCount, rCount, b1, b2, g1, g2, r1, r2;
	int minCount, g, r, b;

	auto rad = radius;
	auto cutoff = ((rad * 2 + 1) * (rad * 2 + 1) + 2) / 4;
	auto leadingEdgeX = blob->leadingEdgeX;
	auto ha = blob->ha;
	auto hr = blob->hr;
	auto hg = blob->hg;
	auto hb = blob->hb;

	LR psamp, px, ps, pd;

	for (y = blockTop; y < blockBottom; y++)
	{
		memset(hb, 0, hSize);
		memset(hg, 0, hSize);
		memset(hr, 0, hSize);

		area = 0;

		ps = (LR)(src + y * p);
		pd = (LR)(dst + y * p);

		top = -CLAMP3(0, rad, y);
		bottom = CLAMP3(0, rad, h - 1 - y);
		left = -CLAMP3(0, rad, w - 1);
		right = CLAMP3(0, rad, w - 1);

		for (v = top; v <= bottom; ++v)
		{
			psamp = (LR)(src + (y + v) * p + 3 * left);

			for (u = left; u <= right; ++u)
			{
				if ((u * u + v * v) <= cutoff)
				{
					++area;
					++hb[psamp->B];
					++hg[psamp->G];
					++hr[psamp->R];
				}

				++psamp;
			}
		}

		for (x = blockLeft; x < blockRight; x++)
		{
			minCount1 = area * (100 - intensity) / 200;
			minCount2 = area * (100 + intensity) / 200;

			bCount = 0;
			b1 = 0;

			while (b1 < 255 && hb[b1] == 0)
				++b1;

			while (b1 < 255 && bCount < minCount1)
			{
				bCount += hb[b1];
				++b1;
			}

			b2 = b1;
			while (b2 < 255 && bCount < minCount2)
			{
				bCount += hb[b2];
				++b2;
			}

			gCount = 0;
			g1 = 0;
			while (g1 < 255 && hg[g1] == 0)
				++g1;

			while (g1 < 255 && gCount < minCount1)
			{
				gCount += hg[g1];
				++g1;
			}

			g2 = g1;
			while (g2 < 255 && gCount < minCount2)
			{
				gCount += hg[g2];
				++g2;
			}

			rCount = 0;
			r1 = 0;
			while (r1 < 255 && hr[r1] == 0)
				++r1;

			while (r1 < 255 && rCount < minCount1)
			{
				rCount += hr[r1];
				++r1;
			}

			r2 = r1;
			while (r2 < 255 && rCount < minCount2)
			{
				rCount += hr[r2];
				++r2;
			}

			pd->B = 255 - (b2 - b1);
			pd->G = 255 - (g2 - g1);
			pd->R = 255 - (r2 - r1);

			left = -CLAMP3(0, rad, x);
			right = CLAMP3(0, rad + 1, w - 1 - x);

			v = -1;
			while (v >= top)
			{
				u = leadingEdgeX[-v];
				if (-u >= left) break;
				--v;
			}

			while (v >= top)
			{
				u = leadingEdgeX[-v];
				px = (LR)(pyte(ps) + v * p - 3 * u);

				--hb[px->B];
				--hg[px->G];
				--hr[px->R];
				--area;

				--v;
			}

			v = -1;
			while (v >= top)
			{
				u = leadingEdgeX[-v];
				if (u + 1 <= right) break;
				--v;
			}

			while (v >= top)
			{
				u = leadingEdgeX[-v];
				px = (LR)(pyte(ps) + v * p + 3 * (u + 1));

				++hb[px->B];
				++hg[px->G];
				++hr[px->R];
				++area;

				--v;
			}

			v = 0;
			while (v <= bottom)
			{
				u = leadingEdgeX[v];
				if (-u >= left) break;
				++v;
			}

			while (v <= bottom)
			{
				u = leadingEdgeX[v];
				px = (LR)(pyte(ps) + v * p - 3 * u);

				--hb[px->B];
				--hg[px->G];
				--hr[px->R];
				--area;

				++v;
			}

			v = 0;
			while (v <= bottom) {
				u = leadingEdgeX[v];
				if (u + 1 <= right)
					break;
				++v;
			}

			while (v <= bottom)
			{
				u = leadingEdgeX[v];
				px = (LR)(pyte(ps) + v * p + 3 * (u + 1));

				++hb[px->B];
				++hg[px->G];
				++hr[px->R];
				++area;

				++v;
			}

			++ps;
			++pd;
		}
	}

	return IMAGE_EFFECT_RESULT_OK;
}

int ImageEffect::pencil_sketch(Sheet* srcImage, Sheet* dstImage, PencilSketchBlob* blob, size_t brushSize, int range,
	int blockLeft, int blockTop, int blockRight, int blockBottom) {
	VALIDATE_IMAGES();
	DECLARE_VARIABLES();
	CLAMP_BLOCK();

	if (!blob || blob->init(brushSize, range))
		return IMAGE_EFFECT_RESULT_ERROR;

	auto weights = blob->weights;
	auto localStore = blob->localStore;
	auto lookup = blob->gray;

	int j, wx, wy, wp, wwx, srcX, srcY, wr;
	int64_t wwxx, waSum, wcSum, bSum, gSum, rSum, * rSums, * gSums, * bSums, * waSums, * wcSums;
	pyte c, ptr;
	int i, red, green, blue, r, wlen, localStoreSize, size;
	uint64_t arraysLength;

	r = brushSize;
	wlen = 1 + (r * 2);
	localStoreSize = sizeof(int64_t);
	ptr = localStore;

	waSums = (int64_t*)ptr;
	ptr += wlen * sizeof(int64_t);

	wcSums = (int64_t*)ptr;
	ptr += wlen * sizeof(int64_t);

	bSums = (int64_t*)ptr;
	ptr += wlen * sizeof(int64_t);

	gSums = (int64_t*)ptr;
	ptr += wlen * sizeof(int64_t);

	rSums = (int64_t*)ptr;
	ptr += wlen * sizeof(int64_t);

	arraysLength = (uint64_t)(sizeof(int64_t) * wlen);
	size = brushSize;
	int sintensity, dintensity;

#define ChannelBlend_ColorDodge(A, B) ((byte)(((B) == 255) ? (B) : Min(255, (((A) << 8 ) / (255 - (B))))))

	for (y = 0; y < h; ++y)
	{
		s = src + y * p;
		d = dst + y * p;

		memset(localStore, 0, (uint64_t)localStoreSize);

		waSum = 0;
		wcSum = 0;
		bSum = 0;
		gSum = 0;
		rSum = 0;

		for (wx = 0; wx < wlen; ++wx)
		{
			srcX = wx - r;
			waSums[wx] = 0;
			wcSums[wx] = 0;
			bSums[wx] = 0;
			gSums[wx] = 0;
			rSums[wx] = 0;

			if (srcX >= 0 && srcX < w)
			{
				for (wy = 0; wy < wlen; ++wy)
				{
					srcY = y + wy - r;

					if (srcY >= 0 && srcY < h)
					{
						c = src + srcY * p + 3 * srcX;
						wp = weights[wy];

						waSums[wx] += wp;
						wp *= 256;
						wcSums[wx] += wp;
						wp >>= 8;

						bSums[wx] += wp * c[0];
						gSums[wx] += wp * c[1];
						rSums[wx] += wp * c[2];
					}
				}

				wwx = weights[wx];
				waSum += wwx * waSums[wx];
				wcSum += wwx * wcSums[wx];
				bSum += wwx * bSums[wx];
				gSum += wwx * gSums[wx];
				rSum += wwx * rSums[wx];
			}
		}

		wcSum >>= 8;

		if (waSum == 0 || wcSum == 0)
		{
			sintensity = (7471 * s[0] + 38470 * s[1] + 19595 * s[2]) >> 16;
			dintensity = (7471 * (255 - lookup[0]) + 38470 * (255 - lookup[0]) + 19595 * (255 - lookup[0])) >> 16;
			dintensity = ChannelBlend_ColorDodge(sintensity, dintensity);
			*d++ = dintensity;
			*d++ = dintensity;
			*d++ = dintensity;
		}
		else
		{
			sintensity = (7471 * s[0] + 38470 * s[1] + 19595 * s[2]) >> 16;
			dintensity = (7471 * (255 - lookup[CLAMP255(bSum / wcSum)]) + 38470 * (255 - lookup[CLAMP255(gSum / wcSum)]) + 19595 * (255 - lookup[CLAMP255(rSum / wcSum)])) >> 16;
			dintensity = ChannelBlend_ColorDodge(sintensity, dintensity);
			*d++ = dintensity;
			*d++ = dintensity;
			*d++ = dintensity;
		}

		s += 3;

		for (x = 1; x < w; ++x)
		{
			for (i = 0; i < wlen - 1; ++i)
			{
				waSums[i] = waSums[i + 1];
				wcSums[i] = wcSums[i + 1];
				bSums[i] = bSums[i + 1];
				gSums[i] = gSums[i + 1];
				rSums[i] = rSums[i + 1];
			}

			waSum = 0;
			wcSum = 0;
			bSum = 0;
			gSum = 0;
			rSum = 0;

			for (wx = 0; wx < wlen - 1; ++wx)
			{
				wwxx = (int64_t)weights[wx];
				waSum += wwxx * waSums[wx];
				wcSum += wwxx * wcSums[wx];
				bSum += wwxx * bSums[wx];
				gSum += wwxx * gSums[wx];
				rSum += wwxx * rSums[wx];
			}

			wx = wlen - 1;

			waSums[wx] = 0;
			wcSums[wx] = 0;
			bSums[wx] = 0;
			gSums[wx] = 0;
			rSums[wx] = 0;

			srcX = x + wx - r;

			if (srcX >= 0 && srcX < w)
			{
				for (wy = 0; wy < wlen; ++wy)
				{
					srcY = y + wy - r;

					if (srcY >= 0 && srcY < h)
					{
						c = src + srcY * p + 3 * srcX;
						wp = weights[wy];

						waSums[wx] += wp;
						wp *= 256;
						wcSums[wx] += wp;
						wp >>= 8;

						bSums[wx] += wp * (int64_t)c[0];
						gSums[wx] += wp * (int64_t)c[1];
						rSums[wx] += wp * (int64_t)c[2];
					}
				}

				wr = weights[wx];
				waSum += (int64_t)wr * waSums[wx];
				wcSum += (int64_t)wr * wcSums[wx];
				bSum += (int64_t)wr * bSums[wx];
				gSum += (int64_t)wr * gSums[wx];
				rSum += (int64_t)wr * rSums[wx];
			}

			wcSum >>= 8;

			if (waSum == 0 || wcSum == 0)
			{
				sintensity = (7471 * s[0] + 38470 * s[1] + 19595 * s[2]) >> 16;
				dintensity = (7471 * (255 - lookup[0]) + 38470 * (255 - lookup[0]) + 19595 * (255 - lookup[0])) >> 16;
				dintensity = ChannelBlend_ColorDodge(sintensity, dintensity);
				*d++ = dintensity;
				*d++ = dintensity;
				*d++ = dintensity;
			}
			else
			{
				sintensity = (7471 * s[0] + 38470 * s[1] + 19595 * s[2]) >> 16;
				dintensity = (7471 * (255 - lookup[CLAMP255(bSum / wcSum)]) + 38470 * (255 - lookup[CLAMP255(gSum / wcSum)]) + 19595 * (255 - lookup[CLAMP255(rSum / wcSum)])) >> 16;
				dintensity = ChannelBlend_ColorDodge(sintensity, dintensity);
				*d++ = dintensity;
				*d++ = dintensity;
				*d++ = dintensity;
			}

			s += 3;
		}
	}

	return IMAGE_EFFECT_RESULT_WHOLE_IMAGE;
}

int ImageEffect::pixelate(Sheet* srcImage, Sheet* dstImage, size_t cellSize,
	int blockLeft, int blockTop, int blockRight, int blockBottom) {
	VALIDATE_IMAGES();
	DECLARE_VARIABLES();
	CLAMP_BLOCK();

	cellSize = CLAMP3(1, cellSize, 200);

	int amount, amount1, ix, iy, fx, fy, xEnd, yEnd, x2, y2;
	byte rs[4], bs[4], gs[4], as[4] = { 255, 255, 255, 255 }, a, r, g, b;
	int af;
	amount = cellSize;
	amount1 = amount - 1;

	for (y = blockTop; y < blockBottom; ++y)
	{
		s = src + y * p + blockLeft * 3;
		d = dst + y * p + blockLeft * 3;

		yEnd = y + 1;
		for (x = blockLeft; x < blockRight; ++x)
		{
			ix = x - (x % amount);
			iy = y - (y % amount);
			fx = ix + amount1;
			fy = iy + amount1;
			fx = fx >= w ? right : fx < 0 ? 0 : fx;
			fy = fy >= h ? bottom : fy < 0 ? 0 : fy;

			s = src + iy * p + 3 * ix;
			bs[0] = s[0]; gs[0] = s[1]; rs[0] = s[2];

			s = src + iy * p + 3 * fx;
			bs[1] = s[0]; gs[1] = s[1]; rs[1] = s[2];

			s = src + fy * p + 3 * ix;
			bs[2] = s[0]; gs[2] = s[1]; rs[2] = s[2];

			s = src + fy * p + 3 * fx;
			bs[3] = s[0]; gs[3] = s[1]; rs[3] = s[2];

			a = 255;
			b = (bs[0] + bs[1] + bs[2] + bs[3]) >> 2;
			g = (gs[0] + gs[1] + gs[2] + gs[3]) >> 2;
			r = (rs[0] + rs[1] + rs[2] + rs[3]) >> 2;

			xEnd = Min(w, fx) + 1;
			yEnd = Min(h, fy) + 1;

			for (y2 = y; y2 < yEnd; ++y2)
			{
				d = dst + y2 * p + 3 * x;

				for (x2 = x; x2 < xEnd; ++x2)
				{
					*d++ = b;
					*d++ = g;
					*d++ = r;
				}
			}
			x = xEnd - 1;
		}
		y = yEnd - 1;
	}

	return IMAGE_EFFECT_RESULT_OK;
}

int ImageEffect::radial_blur(Sheet* srcImage, Sheet* dstImage, RadialBlurMode blurMode, double amount, double pivotX, double pivotY,
	int blockLeft, int blockTop, int blockRight, int blockBottom) {
	VALIDATE_IMAGES();
	DECLARE_VARIABLES();
	CLAMP_BLOCK();

#define RADIUS_LENGTH 64

	amount = CLAMP3F(1., amount, 100.);
	pivotX = CLAMP3F(0., pivotX, 1.);
	pivotY = CLAMP3F(0., pivotY, 1.);

	int i, intensity, rad, cutoff;
	double sc;
	int sa = 0, sb = 0, sg = 0, sr = 0, u, a, v, fx, fy, length, fr, fsr, ox1, ox2, oy1, oy2, ffsr, fcx, fcy, fcxx, fcyy;
	fcx = int(pivotX * w * 65536), fcy = int(pivotY * h * 65536);
	length = amount;
	fr = (int)(amount * PI * 65536.0 / 181.0);
	fsr = fr / RADIUS_LENGTH;
	ffsr = -fsr;
	fcxx = fcx + 32768;
	fcyy = fcy + 32768;
	pyte s2;

	if (blurMode == RadialBlurMode::Spin)
	{
		sc = 2 * RADIUS_LENGTH + 1;

		for (y = blockTop; y < blockBottom; ++y)
		{
			s = src + y * p + blockLeft * 3;
			d = dst + y * p + blockLeft * 3;
			for (x = blockLeft; x < blockRight; ++x)
			{
				sb = *s++;
				sg = *s++;
				sr = *s++;

				ox1 = ox2 = fx = (x << 16) - fcx;
				oy1 = oy2 = fy = (y << 16) - fcy;

				for (i = 0; i < RADIUS_LENGTH; i++)
				{
					ox1 = ox1 - ((oy1 >> 8) * fsr >> 8) - ((ox1 >> 14) * (fsr * fsr >> 11) >> 8);
					oy1 = oy1 + ((ox1 >> 8) * fsr >> 8) - ((oy1 >> 14) * (fsr * fsr >> 11) >> 8);

					ox2 = ox2 - ((oy2 >> 8) * ffsr >> 8) - ((ox2 >> 14) * (fsr * fsr >> 11) >> 8);
					oy2 = oy2 + ((ox2 >> 8) * ffsr >> 8) - ((oy2 >> 14) * (fsr * fsr >> 11) >> 8);

					u = (ox1 + fcxx) >> 16;
					v = (oy1 + fcyy) >> 16;

					s2 = src + CLAMP3(0, v, bottom) * p + 3 * CLAMP3(0, u, right);

					sb += s2[0];
					sg += s2[1];
					sr += s2[2];

					u = (ox2 + fcxx) >> 16;
					v = (oy2 + fcyy) >> 16;

					s2 = src + CLAMP3(0, v, bottom) * p + 3 * CLAMP3(0, u, right);

					sb += s2[0];
					sg += s2[1];
					sr += s2[2];
				}

				*d++ = CLAMP255(sb / sc);
				*d++ = CLAMP255(sg / sc);
				*d++ = CLAMP255(sr / sc);
			}
		}
	}
	else
	{
		sc = RADIUS_LENGTH + 1;

		for (y = blockTop; y < blockBottom; ++y)
		{
			s = src + y * p + blockLeft * 3;
			d = dst + y * p + blockLeft * 3;
			for (x = blockLeft; x < blockRight; ++x)
			{
				sb = *s++;
				sg = *s++;
				sr = *s++;

				fx = (x << 16) - fcx;
				fy = (y << 16) - fcy;

				for (i = 0; i < RADIUS_LENGTH; i++)
				{
					fx = fx - (((fx >> 4) * length) >> 10);
					fy = fy - (((fy >> 4) * length) >> 10);

					u = (fx + fcxx) >> 16;
					v = (fy + fcyy) >> 16;

					s2 = src + CLAMP3(0, v, bottom) * p + 3 * CLAMP3(0, u, right);

					sb += s2[0];
					sg += s2[1];
					sr += s2[2];
				}

				*d++ = CLAMP255(sb / sc);
				*d++ = CLAMP255(sg / sc);
				*d++ = CLAMP255(sr / sc);
			}

		}
	}

	return IMAGE_EFFECT_RESULT_OK;
}

int ImageEffect::random_jitter(Sheet* srcImage, Sheet* dstImage, size_t amount,
	int blockLeft, int blockTop, int blockRight, int blockBottom) {
	VALIDATE_IMAGES();
	DECLARE_VARIABLES();
	CLAMP_BLOCK();

	amount = CLAMP3(1, amount, 200);
	auto hamount = amount / 2;
	int ix, iy;

	for (y = blockTop; y < blockBottom; ++y)
	{
		d = dst + y * p + blockLeft * 3;
		for (x = blockLeft; x < blockRight; ++x)
		{
			ix = x - hamount + (rand() % amount);
			iy = y - hamount + (rand() % amount);

			if (ix < 0) ix = 0;
			else if (ix > right) ix = right;
			if (iy < 0) iy = 0;
			else if (iy > bottom) iy = bottom;

			s = src + iy * p + 3 * ix;
			*d++ = *s++;
			*d++ = *s++;
			*d++ = *s;
		}
	}

	return IMAGE_EFFECT_RESULT_OK;
}

int ImageEffect::ripple(Sheet* srcImage, Sheet* dstImage, RippleMode rippleMode, double waveLength, double amplitude,
	int blockLeft, int blockTop, int blockRight, int blockBottom) {
	VALIDATE_IMAGES();
	DECLARE_VARIABLES();
	CLAMP_BLOCK();

	waveLength = CLAMP3F(1., waveLength, 200.);
	amplitude = CLAMP3F(1., amplitude, 100.);

	double un_x, un_y, fx, fy;
	int i, nSrcX, nSrcY, nSrcX_1, nSrcY_1;
	int length, amnt;
	pyte px0, px1, px2, px3;
	double m0, m1, my;

	amnt = waveLength;
	length = amplitude;

	if (rippleMode == RippleMode::Sinuous)
	{
		for (y = blockTop; y < blockBottom; ++y)
		{
			s = src + y * p + blockLeft * 3;
			d = dst + y * p + blockLeft * 3;
			for (x = blockLeft; x < blockRight; ++x)
			{
				un_x = int(x + w + amnt * sin(y * 2 * PI / length)) % w;
				un_x = fmin(un_x, right);
				un_y = y;

				nSrcX = (int)un_x,
					nSrcY = (int)un_y,
					nSrcX_1 = CLAMP3(0, nSrcX + 1, right),
					nSrcY_1 = CLAMP3(0, nSrcY + 1, bottom);

				px0 = src + nSrcY * p + 3 * nSrcX;
				px1 = src + nSrcY * p + 3 * nSrcX_1;
				px2 = src + nSrcY_1 * p + 3 * nSrcX;
				px3 = src + nSrcY_1 * p + 3 * nSrcX_1;

				fx = un_x - nSrcX;
				fy = un_y - nSrcY;

				for (i = 0; i < 3; i++)
				{
					m0 = px0[i] + fx * (px1[i] - px0[i]),
						m1 = px2[i] + fx * (px3[i] - px2[i]),
						my = m0 + fy * (m1 - m0);

					*d++ = CLAMP255(my);
				}
			}
		}
	}
	else
	{
		for (y = blockTop; y < blockBottom; ++y)
		{
			s = src + y * p + blockLeft * 3;
			d = dst + y * p + blockLeft * 3;
			for (x = blockLeft; x < blockRight; ++x)
			{
				un_x = fmod((x + w + floor(amnt * (fabs((((y % length) / (double)length) * 4) - 2) - 1))), w);
				un_x = fmin(un_x, right);
				un_y = y;

				nSrcX = (int)un_x,
					nSrcY = (int)un_y,
					nSrcX_1 = CLAMP3(0, nSrcX + 1, right),
					nSrcY_1 = CLAMP3(0, nSrcY + 1, bottom);

				px0 = src + nSrcY * p + 3 * nSrcX;
				px1 = src + nSrcY * p + 3 * nSrcX_1;
				px2 = src + nSrcY_1 * p + 3 * nSrcX;
				px3 = src + nSrcY_1 * p + 3 * nSrcX_1;

				fx = un_x - nSrcX;
				fy = un_y - nSrcY;

				for (i = 0; i < 3; i++)
				{
					m0 = px0[i] + fx * (px1[i] - px0[i]),
						m1 = px2[i] + fx * (px3[i] - px2[i]),
						my = m0 + fy * (m1 - m0);

					*d++ = CLAMP255(my);
				}
			}
		}
	}

	return IMAGE_EFFECT_RESULT_OK;
}

int ImageEffect::smart_blur(Sheet* srcImage, Sheet* dstImage, SmartBlurBlob* blob, size_t radius, size_t threshold,
	int blockLeft, int blockTop, int blockRight, int blockBottom) {
	VALIDATE_IMAGES();
	DECLARE_VARIABLES();
	CLAMP_BLOCK();

	if (!blob || blob->init(radius, threshold))
		return IMAGE_EFFECT_RESULT_ERROR;

	int i, ix, iy, rad, a1, a2, r1, r2, g1, g2, b1, b2, ir, ig, ib, ia, cols2, col;
	float* kernel, af, rf, gf, bf, r, g, b, a, f;
	
	int ra = (int)radius;
	int rows = ra * 2 + 1;
	kernel = blob->kernel;
	cols2 = rows / 2;
	pyte s2, d2;
	int ithreshold = (int)threshold;

	for (y = 0; y < h; y++)
	{
		for (x = 0; x < w; x++)
		{
			r = 0, g = 0, b = 0;

			s2 = src + y * p + 3 * x;
			r1 = s2[2];
			g1 = s2[1];
			b1 = s2[0];

			rf = 0, gf = 0, bf = 0;

			for (col = -cols2; col <= cols2; col++)
			{
				f = kernel[cols2 + col];

				if (f != 0) {
					ix = x + col;
					if (!(0 <= ix && ix < w))
						ix = x;
					s2 = src + y * p + 3 * ix;
					r2 = s2[2];
					g2 = s2[1];
					b2 = s2[0];

					int d = r1 - r2;
					if (d >= -ithreshold && d <= ithreshold) {
						r += f * r2;
						rf += f;
					}
					d = g1 - g2;
					if (d >= -ithreshold && d <= ithreshold) {
						g += f * g2;
						gf += f;
					}
					d = b1 - b2;
					if (d >= -ithreshold && d <= ithreshold) {
						b += f * b2;
						bf += f;
					}
				}
			}

			r = rf == 0 ? r1 : r / rf;
			g = gf == 0 ? g1 : g / gf;
			b = bf == 0 ? b1 : b / bf;

			d = dst + y * p + 3 * x;
			d[0] = CLAMP255(b);
			d[1] = CLAMP255(g);
			d[2] = CLAMP255(r);
		}
	}

	for (x = 0; x < w; x++)
	{
		for (y = 0; y < h; y++)
		{
			r = 0, g = 0, b = 0;
			rf = 0, gf = 0, bf = 0;

			d2 = dst + y * p + 3 * x;
			r1 = d2[2];
			g1 = d2[1];
			b1 = d2[0];

			for (col = -cols2; col <= cols2; col++)
			{
				f = kernel[cols2 + col];

				if (f != 0) {
					iy = y + col;
					if (!(0 <= iy && iy < h))
						iy = y;
					d2 = dst + iy * p + 3 * x;
					r2 = d2[2];
					g2 = d2[1];
					b2 = d2[0];

					int d = r1 - r2;
					if (d >= -ithreshold && d <= ithreshold) {
						r += f * r2;
						rf += f;
					}
					d = g1 - g2;
					if (d >= -ithreshold && d <= ithreshold) {
						g += f * g2;
						gf += f;
					}
					d = b1 - b2;
					if (d >= -ithreshold && d <= ithreshold) {
						b += f * b2;
						bf += f;
					}
				}
			}

			r = rf == 0 ? r1 : r / rf;
			g = gf == 0 ? g1 : g / gf;
			b = bf == 0 ? b1 : b / bf;

			d = dst + y * p + 3 * x;
			d[0] = CLAMP255(b);
			d[1] = CLAMP255(g);
			d[2] = CLAMP255(r);
		}
	}

	return IMAGE_EFFECT_RESULT_WHOLE_IMAGE;
}

int ImageEffect::smear(Sheet* srcImage, Sheet* dstImage, SmearMode smearMode, double amount, double mix, double density, double angle,
	int blockLeft, int blockTop, int blockRight, int blockBottom) {
	VALIDATE_IMAGES();
	DECLARE_VARIABLES();
	CLAMP_BLOCK();

	amount = CLAMP3F(1., amount, 100.);
	mix = CLAMP3F(0., mix, 1.);
	density = CLAMP3F(0., density, 1.);
	angle = CLAMP3F(-PI, angle, PI);
	int dis = amount;

	memcpy(dst, src, h * p);
	double cosAngle = cos(angle);
	double sinAngle = sin(angle);
	double f;
	int x1, y1, xx, yy, length, isx, isy, dx, dy, incrE, incrNE, ddx, ddy, x0, y0, radius, radius2;
	int i, numShapes;
	double D;

	if (smearMode == SmearMode::Crosses)
	{
		numShapes = (int)(2 * density * w * h / (dis + 1));

		for (i = 0; i < numShapes; i++)
		{
			xx = rand() % w;
			yy = rand() % h;
			length = rand() % dis + 1;
			s = src + yy * p + 3 * xx;
			for (x1 = xx - length; x1 < xx + length + 1; x1++)
			{
				if (x1 >= 0 && x1 < w)
				{
					d = dst + yy * p + 3 * x1;
					mixColors(mix, d, s, d);
				}
			}
			for (y1 = yy - length; y1 < yy + length + 1; y1++) {
				if (y1 >= 0 && y1 < h) {
					d = dst + y1 * p + 3 * xx;
					mixColors(mix, d, s, d);
				}
			}
		}
	}
	else if (smearMode == SmearMode::Lines)
	{
		numShapes = (int)(2 * density * w * h / 2);

		for (i = 0; i < numShapes; i++)
		{
			isx = rand() % w;
			isy = rand() % h;
			s = src + isy * p + 3 * isx;
			length = rand() % dis;
			dx = (int)(length * cosAngle);
			dy = (int)(length * sinAngle);

			x0 = isx - dx;
			y0 = isy - dy;
			x1 = isx + dx;
			y1 = isy + dy;

			if (x1 < x0)
				ddx = -1;
			else
				ddx = 1;
			if (y1 < y0)
				ddy = -1;
			else
				ddy = 1;
			dx = x1 - x0;
			dy = y1 - y0;
			dx = abs(dx);
			dy = abs(dy);
			x = x0;
			y = y0;

			if (x < w && x >= 0 && y < h && y >= 0)
			{
				d = dst + y * p + 3 * x;
				mixColors(mix, d, s, d);
			}
			if (abs(dx) > abs(dy))
			{
				D = 2 * dy - dx;
				incrE = 2 * dy;
				incrNE = 2 * (dy - dx);

				while (x != x1) {
					if (D <= 0)
						D += incrE;
					else {
						D += incrNE;
						y += ddy;
					}
					x += ddx;
					if (x < w && x >= 0 && y < h && y >= 0) {
						d = dst + y * p + 3 * x;
						mixColors(mix, d, s, d);
					}
				}
			}
			else
			{
				D = 2 * dx - dy;
				incrE = 2 * dx;
				incrNE = 2 * (dx - dy);

				while (y != y1) {
					if (D <= 0)
						D += incrE;
					else {
						D += incrNE;
						x += ddx;
					}
					y += ddy;
					if (x < w && x >= 0 && y < h && y >= 0) {
						d = dst + y * p + 3 * x;
						mixColors(mix, d, s, d);
					}
				}
			}
		}
	}
	else if (smearMode == SmearMode::Circles)
	{
		radius = dis + 1;
		radius2 = sqrt(radius * radius);
		numShapes = (int)(2 * density * w * h / radius);
		for (i = 0; i < numShapes; i++)
		{
			isx = rand() % w;
			isy = rand() % h;
			s = src + isy * p + 3 * isx;

			for (x = isx - radius; x < isx + radius + 1; x++) {
				for (y = isy - radius; y < isy + radius + 1; y++) {

					f = sqrt((x - isx) * (x - isx) + (y - isy) * (y - isy)) / (double)radius;
					f = f > 1.0 ? 1 - (f - 1.0) * dis : 1.0;
					f = f < 0 ? 0 : f;
					if (x >= 0 && x < w && y >= 0 && y < h) {
						d = dst + y * p + 3 * x;
						mixColors(mix * f, d, s, d);
					}
				}
			}
		}
	}
	else if (smearMode == SmearMode::Squares)
	{
		radius = dis + 1;
		numShapes = (int)(2 * density * w * h / radius);
		for (i = 0; i < numShapes; i++)
		{
			isx = rand() % w;
			isy = rand() % h;
			s = src + isy * p + 3 * isx;

			for (x = isx - radius; x < isx + radius + 1; x++) {
				for (y = isy - radius; y < isy + radius + 1; y++) {
					if (x >= 0 && x < w && y >= 0 && y < h) {
						d = dst + y * p + 3 * x;
						mixColors(mix, d, s, d);
					}
				}
			}
		}
	}

	return IMAGE_EFFECT_RESULT_WHOLE_IMAGE;
}

int ImageEffect::soft_portrait(Sheet* srcImage, Sheet* dstImage, SoftPortraitBlob* blob, size_t softness, size_t warmness, int brightness,
	int blockLeft, int blockTop, int blockRight, int blockBottom) {
	VALIDATE_IMAGES();
	DECLARE_VARIABLES();
	CLAMP_BLOCK();

	if (!blob || blob->init(softness, warmness, brightness))
		return IMAGE_EFFECT_RESULT_ERROR;

	int i, j, wx, wy, wp, wwx, srcX, srcY, wr;
	int64_t wwxx, waSum, wcSum, aSum, bSum, gSum, rSum, * rSums, * gSums, * bSums, * waSums, * aSums, * wcSums;
	pyte c, ptr;
	int alpha, red, green, blue, r, wlen, localStoreSize, size;
	uint64_t arraysLength;

	auto weights = blob->weights;
	auto localStore = blob->localStore;
	auto lookup = blob->gray;
	pyte t;
	byte gray;

	r = softness;
	size = wlen = 1 + (r * 2);
	localStoreSize = wlen * 6 * sizeof(int64_t);
	ptr = localStore;

	waSums = (int64_t*)ptr;
	ptr += wlen * sizeof(int64_t);

	wcSums = (int64_t*)ptr;
	ptr += wlen * sizeof(int64_t);

	aSums = (int64_t*)ptr;
	ptr += wlen * sizeof(int64_t);

	bSums = (int64_t*)ptr;
	ptr += wlen * sizeof(int64_t);

	gSums = (int64_t*)ptr;
	ptr += wlen * sizeof(int64_t);

	rSums = (int64_t*)ptr;
	ptr += wlen * sizeof(int64_t);

	arraysLength = (uint64_t)(sizeof(int64_t) * wlen);

	for (y = 0; y < h; ++y)
	{
		memset(localStore, 0, (uint64_t)localStoreSize);

		waSum = 0;
		wcSum = 0;
		bSum = 0;
		gSum = 0;
		rSum = 0;

		s = src + y * p;
		d = dst + y * p;

		for (wx = 0; wx < wlen; ++wx)
		{
			srcX = wx - r;
			waSums[wx] = 0;
			wcSums[wx] = 0;
			bSums[wx] = 0;
			gSums[wx] = 0;
			rSums[wx] = 0;

			if (srcX >= 0 && srcX < w)
			{
				for (wy = 0; wy < wlen; ++wy)
				{
					srcY = y + wy - r;

					if (srcY >= 0 && srcY < h)
					{
						c = src + srcY * p + 3 * srcX;
						wp = weights[wy];

						waSums[wx] += wp;
						wp *= 256;
						wcSums[wx] += wp;
						wp >>= 8;

						bSums[wx] += wp * c[0];
						gSums[wx] += wp * c[1];
						rSums[wx] += wp * c[2];
					}
				}

				wwx = weights[wx];
				waSum += wwx * waSums[wx];
				wcSum += wwx * wcSums[wx];
				bSum += wwx * bSums[wx];
				gSum += wwx * gSums[wx];
				rSum += wwx * rSums[wx];
			}
		}

		wcSum >>= 8;
		gray = (byte)((30 * s[2] + 59 * s[1] + 11 * s[0]) / 100);
		s += 3;

		if (waSum == 0 || wcSum == 0)
		{
			*d++ = overlay_blend(lookup[0], gray * (100 - warmness) / 100);
			*d++ = overlay_blend(lookup[0], gray);
			*d++ = overlay_blend(lookup[0], Min(Max((gray * (100 + warmness) / 100), 0), 255));
		}
		else
		{
			*d++ = overlay_blend(lookup[CLAMP255(bSum / wcSum)], gray * (100 - warmness) / 100);
			*d++ = overlay_blend(lookup[CLAMP255(gSum / wcSum)], gray);
			*d++ = overlay_blend(lookup[CLAMP255(rSum / wcSum)], Min(Max((gray * (100 + warmness) / 100), 0), 255));
		}

		for (x = 1; x < w; ++x)
		{
			for (i = 0; i < wlen - 1; ++i)
			{
				waSums[i] = waSums[i + 1];
				wcSums[i] = wcSums[i + 1];
				bSums[i] = bSums[i + 1];
				gSums[i] = gSums[i + 1];
				rSums[i] = rSums[i + 1];
			}

			waSum = 0;
			wcSum = 0;
			bSum = 0;
			gSum = 0;
			rSum = 0;

			for (wx = 0; wx < wlen - 1; ++wx)
			{
				wwxx = (int64_t)weights[wx];
				waSum += wwxx * waSums[wx];
				wcSum += wwxx * wcSums[wx];
				bSum += wwxx * bSums[wx];
				gSum += wwxx * gSums[wx];
				rSum += wwxx * rSums[wx];
			}

			wx = wlen - 1;

			waSums[wx] = 0;
			wcSums[wx] = 0;
			bSums[wx] = 0;
			gSums[wx] = 0;
			rSums[wx] = 0;

			srcX = x + wx - r;

			if (srcX >= 0 && srcX < w)
			{
				for (wy = 0; wy < wlen; ++wy)
				{
					srcY = y + wy - r;

					if (srcY >= 0 && srcY < h)
					{
						c = src + srcY * p + 3 * srcX;
						wp = weights[wy];

						waSums[wx] += wp;
						wp *= 256;
						wcSums[wx] += wp;
						wp >>= 8;

						bSums[wx] += wp * (int64_t)c[0];
						gSums[wx] += wp * (int64_t)c[1];
						rSums[wx] += wp * (int64_t)c[2];
					}
				}

				wr = weights[wx];
				waSum += (int64_t)wr * waSums[wx];
				wcSum += (int64_t)wr * wcSums[wx];
				bSum += (int64_t)wr * bSums[wx];
				gSum += (int64_t)wr * gSums[wx];
				rSum += (int64_t)wr * rSums[wx];
			}

			wcSum >>= 8;

			gray = (byte)((30 * s[2] + 59 * s[1] + 11 * s[0]) / 100);
			s += 3;

			if (waSum == 0 || wcSum == 0)
			{
				*d++ = overlay_blend(lookup[0], gray * (100 - warmness) / 100);
				*d++ = overlay_blend(lookup[0], gray);
				*d++ = overlay_blend(lookup[0], Min(Max((gray * (100 + warmness) / 100), 0), 255));
			}
			else
			{
				*d++ = overlay_blend(lookup[CLAMP255(bSum / wcSum)], gray * (100 - warmness) / 100);
				*d++ = overlay_blend(lookup[CLAMP255(gSum / wcSum)], gray);
				*d++ = overlay_blend(lookup[CLAMP255(rSum / wcSum)], Min(Max((gray * (100 + warmness) / 100), 0), 255));
			}
		}
	}

	return IMAGE_EFFECT_RESULT_WHOLE_IMAGE;
}

int ImageEffect::stamp(Sheet* srcImage, Sheet* dstImage, StampBlob* blob, size_t radius, double threshold,
	int blockLeft, int blockTop, int blockRight, int blockBottom) {
	VALIDATE_IMAGES();
	DECLARE_VARIABLES();
	CLAMP_BLOCK();

	if (!blob || blob->init(radius, threshold))
		return IMAGE_EFFECT_RESULT_ERROR;

	auto weights = blob->weights;
	auto localStore = blob->localStore;

	int j, wx, wy, wp, wwx, srcX, srcY, wr;
	int64_t wwxx, waSum, wcSum, aSum, bSum, gSum, rSum, * rSums, * gSums, * bSums, * waSums, * aSums, * wcSums;
	pyte c, ptr;
	int alpha, red, green, blue, r, wlen, localStoreSize, size;
	uint64_t arraysLength;

	int i, l, upperThreshold3, lowerThreshold3, dv;
	float f;
	byte blb, blg, blr, whb, whg, whr, wha, bla;
	bla = blb = blg = blr = 0;
	wha = whb = whg = whr = 255;

	r = radius;
	size = wlen = 1 + (r * 2);
	localStoreSize = sizeof(int64_t);
	ptr = localStore;

	waSums = (int64_t*)ptr;
	ptr += wlen * sizeof(int64_t);

	wcSums = (int64_t*)ptr;
	ptr += wlen * sizeof(int64_t);

	aSums = (int64_t*)ptr;
	ptr += wlen * sizeof(int64_t);

	bSums = (int64_t*)ptr;
	ptr += wlen * sizeof(int64_t);

	gSums = (int64_t*)ptr;
	ptr += wlen * sizeof(int64_t);

	rSums = (int64_t*)ptr;
	ptr += wlen * sizeof(int64_t);

	arraysLength = (uint64_t)(sizeof(int64_t) * wlen);


	threshold /= 100.0;

	lowerThreshold3 = 255 * 3 * (threshold - radius * 0.005f);
	upperThreshold3 = 255 * 3 * (threshold + radius * 0.005f);

	if (upperThreshold3 == lowerThreshold3)
		upperThreshold3 = lowerThreshold3 + 1;

	dv = (upperThreshold3 - lowerThreshold3);


	for (y = 0; y < h; ++y)
	{
		d = dst + y * p;

		memset(localStore, 0, (uint64_t)localStoreSize);

		waSum = 0;
		wcSum = 0;
		bSum = 0;
		gSum = 0;
		rSum = 0;

		d = dst + y * p;

		for (wx = 0; wx < wlen; ++wx)
		{
			srcX = wx - r;
			waSums[wx] = 0;
			wcSums[wx] = 0;
			bSums[wx] = 0;
			gSums[wx] = 0;
			rSums[wx] = 0;

			if (srcX >= 0 && srcX < w)
			{
				for (wy = 0; wy < wlen; ++wy)
				{
					srcY = y + wy - r;

					if (srcY >= 0 && srcY < h)
					{
						c = src + srcY * p + 3 * srcX;
						wp = weights[wy];

						waSums[wx] += wp;
						wp *= 256;
						wcSums[wx] += wp;
						wp >>= 8;

						bSums[wx] += wp * c[0];
						gSums[wx] += wp * c[1];
						rSums[wx] += wp * c[2];
					}
				}

				wwx = weights[wx];
				waSum += wwx * waSums[wx];
				wcSum += wwx * wcSums[wx];
				bSum += wwx * bSums[wx];
				gSum += wwx * gSums[wx];
				rSum += wwx * rSums[wx];
			}
		}

		wcSum >>= 8;

		if (waSum == 0 || wcSum == 0) l = 0;
		else l = CLAMP255(bSum / wcSum) + CLAMP255(gSum / wcSum) + CLAMP255(rSum / wcSum);

		if (l < lowerThreshold3)
			f = 0;
		if (l >= upperThreshold3)
			f = 1;
		l = (l - lowerThreshold3) / dv;

		f = l * l * (3 - 2 * l);

		*d++ = CLAMP255(blb + f * (whb - blb));
		*d++ = CLAMP255(blg + f * (whg - blg));
		*d++ = CLAMP255(blr + f * (whr - blr));

		for (x = 1; x < w; ++x)
		{
			for (i = 0; i < wlen - 1; ++i)
			{
				waSums[i] = waSums[i + 1];
				wcSums[i] = wcSums[i + 1];
				bSums[i] = bSums[i + 1];
				gSums[i] = gSums[i + 1];
				rSums[i] = rSums[i + 1];
			}

			waSum = 0;
			wcSum = 0;
			bSum = 0;
			gSum = 0;
			rSum = 0;

			for (wx = 0; wx < wlen - 1; ++wx)
			{
				wwxx = (int64_t)weights[wx];
				waSum += wwxx * waSums[wx];
				wcSum += wwxx * wcSums[wx];
				bSum += wwxx * bSums[wx];
				gSum += wwxx * gSums[wx];
				rSum += wwxx * rSums[wx];
			}

			wx = wlen - 1;

			waSums[wx] = 0;
			wcSums[wx] = 0;
			bSums[wx] = 0;
			gSums[wx] = 0;
			rSums[wx] = 0;

			srcX = x + wx - r;

			if (srcX >= 0 && srcX < w)
			{
				for (wy = 0; wy < wlen; ++wy)
				{
					srcY = y + wy - r;

					if (srcY >= 0 && srcY < h)
					{
						c = src + srcY * p + 3 * srcX;
						wp = weights[wy];

						waSums[wx] += wp;
						wp *= 256;
						wcSums[wx] += wp;
						wp >>= 8;

						bSums[wx] += wp * (int64_t)c[0];
						gSums[wx] += wp * (int64_t)c[1];
						rSums[wx] += wp * (int64_t)c[2];
					}
				}

				wr = weights[wx];
				waSum += (int64_t)wr * waSums[wx];
				wcSum += (int64_t)wr * wcSums[wx];
				bSum += (int64_t)wr * bSums[wx];
				gSum += (int64_t)wr * gSums[wx];
				rSum += (int64_t)wr * rSums[wx];
			}

			wcSum >>= 8;

			if (waSum == 0 || wcSum == 0) l = 0;
			else l = CLAMP255(bSum / wcSum) + CLAMP255(gSum / wcSum) + CLAMP255(rSum / wcSum);

			if (l < lowerThreshold3)
				f = 0;
			if (l >= upperThreshold3)
				f = 1;
			l = (l - lowerThreshold3) / dv;

			f = l * l * (3 - 2 * l);

			*d++ = CLAMP255(blb + f * (whb - blb));
			*d++ = CLAMP255(blg + f * (whg - blg));
			*d++ = CLAMP255(blr + f * (whr - blr));
		}
	}

	return IMAGE_EFFECT_RESULT_WHOLE_IMAGE;
}

int ImageEffect::surface_blur(Sheet* srcImage, Sheet* dstImage, SurfaceBlurBlob* blob, double radiusf, double levelf,
	int blockLeft, int blockTop, int blockRight, int blockBottom) {
	VALIDATE_IMAGES();
	DECLARE_VARIABLES();
	CLAMP_BLOCK();

	radiusf = CLAMP3F(1., radiusf, 100.);
	levelf = CLAMP3F(2., levelf, 255.);

	if (!blob)
		return IMAGE_EFFECT_RESULT_ERROR;

	int i;
	auto mat = blob->mat;
	auto imat = blob->imat;

	int level = levelf;
	int radius = radiusf;
	int numrad = radius;
	int    dx;
	float sd, c1, c2;
	int dix;
	int mlevel = -levelf;
	byte* src_db, * src_dg, * src_dr;
	uint32_t red_sum, grn_sum, blu_sum;
	uint32_t red_fact, grn_fact, blu_fact;
	int offset;
	byte* src_b, * src_g, * src_r;
	unsigned long red_rowsum, grn_rowsum, blu_rowsum;
	unsigned long red_rowfact, grn_rowfact, blu_rowfact;
	int tmp;
	int mnumrad = 1 - numrad;
	float fsum, fscale;
	int j;

	sd = radius / 3.329042969;
	c1 = 1.0 / sqrt(2.0 * PI * sd);
	c2 = -2.0 * (sd * sd);

	for (dx = 0; dx < numrad; dx++)
		mat[dx] = c1 * exp((dx * dx) / c2);

	fsum = 0.0;
	for (y = 1 - numrad; y < numrad; y++)
		fsum += mat[abs(y)];

	fscale = 0x1000 / fsum;
	for (y = 0; y < numrad; y++)
		imat[numrad - y] = imat[numrad + y] = mat[y] * fscale;
	double D;

	for (y = blockTop; y < blockBottom; y++)
	{
		for (x = blockLeft; x < blockRight; x++)
		{
			dix = y * p + 3 * x;

			src_db = src + dix;
			src_dg = src + dix + 1;
			src_dr = src + dix + 2;

			red_sum = 0;
			grn_sum = 0;
			blu_sum = 0;

			red_fact = 0;
			grn_fact = 0;
			blu_fact = 0;

			offset = p * (y - numrad) + 3 * (x - numrad);

			for (j = mnumrad; j < numrad; j++)
			{
				red_rowsum = 0;
				grn_rowsum = 0;
				blu_rowsum = 0;
				red_rowfact = 0;
				grn_rowfact = 0;
				blu_rowfact = 0;

				offset += p;
				if (y + j < 0 || y + j >= h)
					continue;

				src_b = src + offset;
				src_g = src + offset + 1;
				src_r = src + offset + 2;

				for (i = mnumrad; i < numrad; i++)
				{
					src_b += 3;
					src_g += 3;
					src_r += 3;

					if (x + i < 0 || x + i >= w)
						continue;

					D = imat[numrad + i];

					tmp = *src_db - *src_b;
					if (tmp < level && tmp > mlevel)
					{
						blu_rowsum += D * *src_b;
						blu_rowfact += D;
					}

					tmp = *src_dg - *src_g;
					if (tmp < level && tmp > mlevel)
					{
						grn_rowsum += D * *src_g;
						grn_rowfact += D;
					}

					tmp = *src_dr - *src_r;
					if (tmp < level && tmp > mlevel)
					{
						red_rowsum += D * *src_r;
						red_rowfact += D;
					}
				}

				D = imat[numrad + j];
				blu_sum += D * blu_rowsum;
				grn_sum += D * grn_rowsum;
				red_sum += D * red_rowsum;
				blu_fact += D * blu_rowfact;
				grn_fact += D * grn_rowfact;
				red_fact += D * red_rowfact;
			}

			d = dst + dix;

			if (blu_fact == 0) d[0] = *src_db;
			else d[0] = blu_sum / blu_fact;

			if (grn_fact == 0) d[1] = *src_dg;
			else d[1] = grn_sum / grn_fact;

			if (red_fact == 0) d[2] = *src_dr;
			else d[2] = red_sum / red_fact;
		}

	}

	return IMAGE_EFFECT_RESULT_OK;
}

int ImageEffect::swirl(Sheet* srcImage, Sheet* dstImage, double angle, bool shouldStretch, double pivotX, double pivotY,
	int blockLeft, int blockTop, int blockRight, int blockBottom) {
	VALIDATE_IMAGES();
	DECLARE_VARIABLES();
	CLAMP_BLOCK();

	angle = CLAMP3F(-PI / 4, angle, PI / 4);
	pivotX = CLAMP3F(0., pivotX, 1.);
	pivotY = CLAMP3F(0., pivotY, 1.);

	int n = -angle * 180;
	auto twist = n * n * ((n > 0) ? 1 : -1);

	double un_x, un_y, size;
	int i, nSrcX, nSrcY, nSrcX_1, nSrcY_1;
	double u, v, theta, t, r, fx, fy;
	pyte px0, px1, px2, px3;
	double m0, m1, my;
	size = .9;
	double sc = w > h ? pivotX / pivotY : pivotY / pivotX;
	pivotX = pivotX * w;
	pivotY = pivotY * h;
	double invmaxrad = 1.0 / (pivotY < pivotX ? pivotY : pivotX);

	if (shouldStretch)
	{
		if (pivotX > pivotY)
		{
			for (y = 0; y < h; y++)
			{
				for (x = 0; x < w; x++)
				{
					u = (x - pivotX) / sc;
					v = (y - pivotY);

					r = sqrt(u * u + v * v);
					theta = atan2(v, u);

					t = 1 - ((r * size) * invmaxrad);
					t = (t < 0) ? 0 : (t * t * t);
					theta += (t * twist) / 100.0;

					un_x = fmin(pivotX + r * cos(theta) * sc, right);
					un_y = fmin(pivotY + r * sin(theta), bottom);

					nSrcX = (int)un_x,
						nSrcY = (int)un_y,
						nSrcX_1 = Min(nSrcX + 1, right),
						nSrcY_1 = Min(nSrcY + 1, bottom);

					px0 = src + nSrcY * p + 3 * nSrcX;
					px1 = src + nSrcY * p + 3 * nSrcX_1;
					px2 = src + nSrcY_1 * p + 3 * nSrcX;
					px3 = src + nSrcY_1 * p + 3 * nSrcX_1;

					fx = un_x - nSrcX;
					fy = un_y - nSrcY;

					d = dst + y * p + 3 * x;

					for (i = 0; i < 3; i++)
					{
						m0 = px0[i] + fx * (px1[i] - px0[i]),
							m1 = px2[i] + fx * (px3[i] - px2[i]),
							my = m0 + fy * (m1 - m0);

						*d++ = CLAMP255(my);
					}
				}
			}

		}
		else
		{
			for (y = blockTop; y < blockBottom; y++)
			{
				d = dst + y * p + 3 * blockLeft;
				for (x = blockLeft; x < blockRight; x++)
				{
					u = (x - pivotX);
					v = (y - pivotY) / sc;

					r = sqrt(u * u + v * v);
					theta = atan2(v, u);

					t = 1 - ((r * size) * invmaxrad);
					t = (t < 0) ? 0 : (t * t * t);
					theta += (t * twist) / 100.0;

					un_x = fmin(pivotX + r * cos(theta), right);
					un_y = fmin(pivotY + r * sin(theta) * sc, bottom);

					nSrcX = (int)un_x,
						nSrcY = (int)un_y,
						nSrcX_1 = CLAMP3(0, nSrcX + 1, right),
						nSrcY_1 = CLAMP3(0, nSrcY + 1, bottom);

					px0 = src + nSrcY * p + 3 * nSrcX;
					px1 = src + nSrcY * p + 3 * nSrcX_1;
					px2 = src + nSrcY_1 * p + 3 * nSrcX;
					px3 = src + nSrcY_1 * p + 3 * nSrcX_1;

					fx = un_x - nSrcX;
					fy = un_y - nSrcY;

					d = dst + y * p + 3 * x;

					for (i = 0; i < 3; i++)
					{
						m0 = px0[i] + fx * (px1[i] - px0[i]),
							m1 = px2[i] + fx * (px3[i] - px2[i]),
							my = m0 + fy * (m1 - m0);

						*d++ = CLAMP255(my);
					}
				}
			}
		}
	}
	else
	{
		for (y = blockTop; y < blockBottom; y++)
		{
			d = dst + y * p + 3 * blockLeft;
			for (x = blockLeft; x < blockRight; x++)
			{
				u = (x - pivotX);
				v = (y - pivotY);

				r = sqrt(u * u + v * v);
				theta = atan2(v, u);

				t = 1 - ((r * size) * invmaxrad);
				t = (t < 0) ? 0 : (t * t * t);
				theta += (t * twist) / 100.0;

				un_x = pivotX + r * cos(theta);
				un_y = pivotY + r * sin(theta);

				nSrcX = CLAMP3(0, (int)un_x, right),
					nSrcY = CLAMP3(0, (int)un_y, bottom),
					nSrcX_1 = CLAMP3(0, nSrcX + 1, right),
					nSrcY_1 = CLAMP3(0, nSrcY + 1, bottom);

				px0 = src + nSrcY * p + 3 * nSrcX;
				px1 = src + nSrcY * p + 3 * nSrcX_1;
				px2 = src + nSrcY_1 * p + 3 * nSrcX;
				px3 = src + nSrcY_1 * p + 3 * nSrcX_1;

				fx = un_x - nSrcX;
				fy = un_y - nSrcY;

				for (i = 0; i < 3; i++)
				{
					m0 = px0[i] + fx * (px1[i] - px0[i]),
						m1 = px2[i] + fx * (px3[i] - px2[i]),
						my = m0 + fy * (m1 - m0);

					*d++ = CLAMP255(my);
				}
			}
		}
	}

	return IMAGE_EFFECT_RESULT_OK;
}
