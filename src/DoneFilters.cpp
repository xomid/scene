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


struct YPoint
{
	int index;
	float x, y;
	float dx, dy;
	float cubeX, cubeY;
	float distance;
};

struct Crystal
{
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

#define B  0x100
#define BM  0xff
#define N  0x1000

	int p[B + B + 2];
	float g3[B + B + 2][3];
	float g2[B + B + 2][2];
	float g1[B + B + 2];
	bool start;
	YPoint results[3];

	float noise2(float x, float y)
	{
		int bx0, bx1, by0, by1, b00, b10, b01, b11;
		float rx0, rx1, ry0, ry1, * q, sx, sy, a, b, t, u, v;
		int i, j;
		if (start) {
			start = false;
			int i, j, k;
			float* v, s;

			for (i = 0; i < B; i++)
			{
				p[i] = i;
				g1[i] = (float)(((rand() % 0x7fffffff) % (B + B)) - B) / B;
				for (j = 0; j < 2; j++)
					g2[i][j] = (float)(((rand() % 0x7fffffff) % (B + B)) - B) / B;

				v = g2[i];
				s = (float)sqrt(v[0] * v[0] + v[1] * v[1]);
				v[0] = v[0] / s;
				v[1] = v[1] / s;

				for (j = 0; j < 3; j++)
					g3[i][j] = (float)(((rand() % 0x7fffffff) % (B + B)) - B) / B;

				v = g3[i];
				s = (float)sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
				v[0] = v[0] / s;
				v[1] = v[1] / s;
				v[2] = v[2] / s;
			}

			for (i = B - 1; i >= 0; i--)
			{
				k = p[i];
				p[i] = p[j = (rand() % 0x7fffffff) % B];
				p[j] = k;
			}

			for (i = 0; i < B + 2; i++)
			{
				p[B + i] = p[i];
				g1[B + i] = g1[i];
				for (j = 0; j < 2; j++)
					g2[B + i][j] = g2[i][j];
				for (j = 0; j < 3; j++)
					g3[B + i][j] = g3[i][j];
			}
		}
		t = x + N;
		bx0 = ((int)t) & BM;
		bx1 = (bx0 + 1) & BM;
		rx0 = t - (int)t;
		rx1 = rx0 - 1.0f;
		t = y + N;
		by0 = ((int)t) & BM;
		by1 = (by0 + 1) & BM;
		ry0 = t - (int)t;
		ry1 = ry0 - 1.0f;
		i = p[bx0];
		j = p[bx1];
		b00 = p[i + by0];
		b10 = p[j + by0];
		b01 = p[i + by1];
		b11 = p[j + by1];
		sx = rx0 * rx0 * (3.0f - 2.0f * rx0);
		sy = ry0 * ry0 * (3.0f - 2.0f * ry0);
		q = g2[b00]; u = rx0 * q[0] + ry0 * q[1];
		q = g2[b10]; v = rx1 * q[0] + ry0 * q[1];
		a = u + sx * (v - u);
		q = g2[b01]; u = rx0 * q[0] + ry1 * q[1];
		q = g2[b11]; v = rx1 * q[0] + ry1 * q[1];
		b = u + sx * (v - u);

		return 1.5f * (a + sy * (b - a));
	}
	float checkCube(float x, float y, int cubeX, int cubeY)
	{
		int numPoints, i;
		float px, py, dx, dy, d, weight;
		YPoint p, * pp;

		srand(571 * cubeX + 23 * cubeY);

		switch (mode)
		{
		case CrystalizeMode::Random:
		default:
			numPoints = probabilities[rand() & 0x1fff];
			break;
		case CrystalizeMode::Square:
			numPoints = 1;
			break;
		case CrystalizeMode::Hexagonal:
			numPoints = 1;
			break;
		case CrystalizeMode::Octagonal:
			numPoints = 2;
			break;
		case CrystalizeMode::Triangular:
			numPoints = 2;
			break;
		}

		for (i = 0; i < numPoints; i++) {
			px = 0; py = 0;
			weight = 1.0f;
			switch (mode) {
			case CrystalizeMode::Random:
				px = (float)rand() / RAND_MAX;
				py = (float)rand() / RAND_MAX;
				break;
			case CrystalizeMode::Square:
				px = py = 0.5f;
				if (randomness != 0) {
					px = px + randomness * (((float)rand() / RAND_MAX) - 0.5f);
					py = py + randomness * (((float)rand() / RAND_MAX) - 0.5f);
				}
				break;
			case CrystalizeMode::Hexagonal:
				if ((cubeX & 1) == 0) {
					px = 0.75f; py = 0;
				}
				else {
					px = 0.75f; py = 0.5f;
				}
				if (randomness != 0) {
					px = px + randomness * noise2(271 * (cubeX + px), 271 * (cubeY + py));
					py = py + randomness * noise2(271 * (cubeX + px) + 89, 271 * (cubeY + py) + 137);
				}
				break;
			case CrystalizeMode::Octagonal:
				switch (i) {
				case 0: px = 0.207f; py = 0.207f; break;
				case 1: px = 0.707f; py = 0.707f; weight = 1.6f; break;
				}
				if (randomness != 0) {
					px += randomness * noise2(271 * (cubeX + px), 271 * (cubeY + py));
					py += randomness * noise2(271 * (cubeX + px) + 89, 271 * (cubeY + py) + 137);
				}
				break;
			case CrystalizeMode::Triangular:
				if ((cubeY & 1) == 0) {
					if (i == 0) {
						px = 0.25f; py = 0.35f;
					}
					else {
						px = 0.75f; py = 0.65f;
					}
				}
				else {
					if (i == 0) {
						px = 0.75f; py = 0.35f;
					}
					else {
						px = 0.25f; py = 0.65f;
					}
				}
				if (randomness != 0) {
					px += randomness * noise2(271 * (cubeX + px), 271 * (cubeY + py));
					py += randomness * noise2(271 * (cubeX + px) + 89, 271 * (cubeY + py) + 137);
				}
				break;
			}
			dx = (float)fabs(x - px);
			dy = (float)fabs(y - py);
			d;
			dx *= weight;
			dy *= weight;
			if (distancePower == 1.0f)
				d = dx + dy;
			else if (distancePower == 2.0f)
				d = (float)sqrt(dx * dx + dy * dy);
			else
				d = (float)pow((float)pow(dx, distancePower) + (float)pow(dy, distancePower), 1 / distancePower);

			// Insertion sort the long way round to speed it up a bit
			if (d < results[0].distance) {
				p = results[2];
				results[2] = results[1];
				results[1] = results[0];
				results[0] = p;
				pp = &results[0];
				pp->distance = d;
				pp->dx = dx;
				pp->dy = dy;
				pp->x = cubeX + px;
				pp->y = cubeY + py;
			}
			else if (d < results[1].distance) {
				p = results[2];
				results[2] = results[1];
				results[1] = p;
				pp = &results[1];
				pp->distance = d;
				pp->dx = dx;
				pp->dy = dy;
				pp->x = cubeX + px;
				pp->y = cubeY + py;
			}
			else if (d < results[2].distance) {
				pp = &results[2];
				pp->distance = d;
				pp->dx = dx;
				pp->dy = dy;
				pp->x = cubeX + px;
				pp->y = cubeY + py;
			}
		}

		return results[2].distance;
	}
};


int PosterizeBlob::init(byte threshold) {
	if (!bSet) {
		for (int i = 0; i < 256; i++)
			gray[i] = 255 * (threshold * i / 256) / (threshold - 1);
		bSet = true;
	}
	return 0;
}


// adjustments

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

	return 0;
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

	return 0;
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

	return 0;
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

	return 0;
}

int ImageEffect::posterize(Sheet* srcImage, Sheet* dstImage, PosterizeBlob* blob, byte levels,
	int blockLeft, int blockTop, int blockRight, int blockBottom)
{
	if (blob == NULL || blob->init(levels)) return 1;
	return apply_lookup_gray(srcImage, dstImage, blob, blockLeft, blockTop, blockRight, blockBottom);
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

	return 0;
}



// filters

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

	return 0;
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

	return 0;
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
				s = src + y * p + blockLeft * 3;
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
						un_x = fmin(u * rscale2 * sc + pivotX, right);
						un_y = fmin(v * rscale2 + pivotY, bottom);
					}
					else
					{
						un_x = x;
						un_y = y;
					}

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
					u = x - pivotX;
					v = (y - pivotY) / sc;
					r = sqrt(u * u + v * v);
					rscale1 = 1.0 - (r / maxrad);

					if (rscale1 > 0)
					{
						rscale2 = 1.0 - amount * rscale1 * rscale1;
						un_x = fmin(u * rscale2 + pivotX, right);
						un_y = fmin(v * rscale2 * sc + pivotY, bottom);
					}
					else
					{
						un_x = x;
						un_y = y;
					}

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
			s = src + y * p + blockLeft * 3;
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
					un_x = fmin(u * rscale2 + pivotX, right);
					un_y = fmin(v * rscale2 + pivotY, bottom);
				}
				else
				{
					un_x = x;
					un_y = y;
				}

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

	return 0;
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

int ImageEffect::crystalize(Sheet* srcImage, Sheet* dstImage, CrystalizeMode crystalizeMode, bool shouldFadeEdges,
	double edgeThickness, double randomness, double distancePower, double scale,
	int blockLeft, int blockTop, int blockRight, int blockBottom) {

	VALIDATE_IMAGES();
	DECLARE_VARIABLES();
	//CLAMP_BLOCK();

	edgeThickness = CLAMP3F(0., edgeThickness, 1.);
	randomness = CLAMP3F(0., randomness, 1.);
	distancePower = CLAMP3F(0.01, distancePower, 10.);
	scale = CLAMP3F(0.01, scale, 256.);

	float nx, ny, f, f1, f2, fx, fy, a, angle, factorial, total, probability, D,
		angleCoefficient, gradientCoefficient;
	int i, ix, iy, bx, by, en, st, j;

	Crystal cry;
	auto* results = cry.results;
	byte v[4], v2[4];
	byte edgeColor[4] = { 0, 0, 0, 0 };
	cry.start = 1;
	factorial = 1;
	total = 0;

	cry.mode = crystalizeMode;
	cry.edgeThickness = edgeThickness;
	cry.randomness = randomness;
	cry.distancePower = distancePower;
	cry.scale = scale;

	for (i = 0; i < 10; i++)
	{
		if (i > 1)
			factorial *= i;
		probability = (float)pow(2.5f, i) * (float)exp(-2.5f) / factorial;
		st = (int)(total * 8192);
		total += probability;
		en = (int)(total * 8192);
		for (j = st; j < en; j++) cry.probabilities[j] = (byte)i;
	}

	angleCoefficient = cry.angleCoefficient;
	gradientCoefficient = cry.gradientCoefficient;

	for (y = 0; y < h; ++y)
	{
		s = src + y * p;
		d = dst + y * p ;
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

			D = cry.checkCube(fx, fy, bx, by);
			if (D > fy)
				D = cry.checkCube(fx, fy + 1, bx, by - 1);
			if (D > 1 - fy)
				D = cry.checkCube(fx, fy - 1, bx, by + 1);
			if (D > fx) {
				cry.checkCube(fx + 1, fy, bx - 1, by);
				if (D > fy)
					D = cry.checkCube(fx + 1, fy + 1, bx - 1, by - 1);
				if (D > 1 - fy)
					D = cry.checkCube(fx + 1, fy - 1, bx - 1, by + 1);
			}
			if (D > 1 - fx) {
				D = cry.checkCube(fx - 1, fy, bx + 1, by);
				if (D > fy)
					D = cry.checkCube(fx - 1, fy + 1, bx + 1, by - 1);
				if (D > 1 - fy)
					D = cry.checkCube(fx - 1, fy - 1, bx + 1, by + 1);
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

int ImageEffect::despeckle(Sheet* srcImage, Sheet* dstImage, size_t iteration,
	int blockLeft, int blockTop, int blockRight, int blockBottom) {

	VALIDATE_IMAGES();
	DECLARE_VARIABLES();
	CLAMP_BLOCK();

	iteration = CLAMP3(1, iteration, 20);

	pyte t, m, b;
	byte ob, og, or ;
	bool xIn, yIn;
	long ti = 0;

	memcpy(dst, src, h * p);

	while (iteration-- >= 0)
	{
		for (y = blockTop; y < blockBottom; ++y)
		{
			s = src + y * p + blockLeft * 3;
			d = dst + y * p + blockLeft * 3;
			yIn = y > 0 && y < bottom;
			for (x = blockLeft; x < blockRight; ++x)
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

	return 0;
}


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

int ImageEffect::gussian_blur(Sheet* srcImage, Sheet* dstImage, size_t radius,
	int blockLeft, int blockTop, int blockRight, int blockBottom) {

	VALIDATE_IMAGES();
	DECLARE_VARIABLES();
	CLAMP_BLOCK();

	radius = CLAMP3(0, radius, 254);
	static unsigned char stack[256];
	memcpy(dst, src, h * p);

	uint32_t xp, yp, i;
	uint32_t sp;
	uint32_t stack_start;
	byte* stack_ptr;

	byte* src_ptr;
	byte* dst_ptr;

	unsigned long sum_r;
	unsigned long sum_g;
	unsigned long sum_b;
	unsigned long sum_a;
	unsigned long sum_in_r;
	unsigned long sum_in_g;
	unsigned long sum_in_b;
	unsigned long sum_in_a;
	unsigned long sum_out_r;
	unsigned long sum_out_g;
	unsigned long sum_out_b;
	unsigned long sum_out_a;

	uint32_t wm = w - 1;
	uint32_t hm = h - 1;
	uint32_t w4 = w * 4;
	uint32_t w3 = w * 3;
	uint32_t div = (radius * 2) + 1;
	uint32_t mul_sum = stackblur_mul[radius];
	byte shr_sum = stackblur_shr[radius];

	int u = 0;

	for (y = 0; y < h; y++)
	{
		sum_r = sum_g = sum_b =
			sum_in_r = sum_in_g = sum_in_b =
			sum_out_r = sum_out_g = sum_out_b = 0;

		src_ptr = dst + w3 * y; // start of line (0,y)

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
		src_ptr = dst + 3 * (xp + y * w); //   img.pix_ptr(xp, y);
		dst_ptr = dst + y * w3; // img.pix_ptr(0, y);
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
			if (i <= hm) src_ptr += w3; // +stride
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
		src_ptr = dst + 3 * (x + yp * w);
		dst_ptr = dst + 3 * x;
		for (y = 0; y < h; y++)
		{
			dst_ptr[0] = byte((sum_r * mul_sum) >> shr_sum);
			dst_ptr[1] = byte((sum_g * mul_sum) >> shr_sum);
			dst_ptr[2] = byte((sum_b * mul_sum) >> shr_sum);
			dst_ptr += w3;

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
				src_ptr += w3; // stride
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

	return 0;
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

	return 0;
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

	return 0;
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

	return 0;
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

				xSample = Min(xSample, right);
				ySample = Min(ySample, bottom);

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

	return 0;
}

int ImageEffect::unsharp(Sheet* srcImage, Sheet* dstImage,
	int blockLeft, int blockTop, int blockRight, int blockBottom) {
	return convolution(srcImage, dstImage, blurMatrix, 3, 3, blockLeft, blockTop, blockRight, blockBottom);
}

int ImageEffect::wave(Sheet* srcImage, Sheet* dstImage, double waveLength, double amplitude, double pivotX, double pivotY,
	int blockLeft, int blockTop, int blockRight, int blockBottom) {

	DECLARE_VARIABLES()

	waveLength = CLAMP3F(2., waveLength, 200.);
	amplitude = CLAMP3F(1., waveLength, 50.);
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
			nSrcX = Min((int)un_x, right);
			nSrcY = Min((int)un_y, bottom);

			nSrcX_1 = Min(nSrcX + 1, right);
			nSrcY_1 = Min(nSrcY + 1, bottom);

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

	return 0;
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
				nw = src + Min(srcY, bottom) * p + 3 * Min(srcX, right);
				ne = src + Min(srcY, bottom) * p + 3 * Min(srcX + 1, right);
				sw = src + Min(srcY + 1, bottom) * p + 3 * Min(srcX, right);
				se = src + Min(srcY + 1, bottom) * p + 3 * Min(srcX + 1, right);
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

	return 0;
}