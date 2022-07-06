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


#define MAX_WANIMATE 50

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


static float biasf(float a, float b)
{
	//		return (float)Math.pow(a, Math.log(b) / Math.log(0.5));
	return a / ((1.0f / b - 2) * (1.0f - a) + 1);
}

static float gainf(float a, float b) {
	/*
	float p = (float)Math.log(1.0 - b) / (float)Math.log(0.5);

	if (a < .001)
	return 0.0f;
	else if (a > .999)
	return 1.0f;
	if (a < 0.5)
	return (float)Math.pow(2 * a, p) / 2;
	else
	return 1.0f - (float)Math.pow(2 * (1. - a), p) / 2;
	*/
	float c = (1.0f / b - 2.0f) * (1.0f - 2.0f * a);
	if (a < 0.5)
		return a / (c + 1.0f);
	else
		return (c - a) / (c - 1.0f);
}

int GainBlob::init(double gain, double bias) {
	gain = CLAMP3F(0., gain, 1.);
	bias = CLAMP3F(0., bias, 1.);

	if (!bSet) {
		float f;
		for (int i = 0; i < 256; i++)
		{
			f = i / 255.0;
			f = gainf(f, gain);
			f = biasf(f, bias);
			gray[i] = f * 255.0;
		}
		bSet = true;
	}
	return 0;
}


int ImageEffect::gain(Sheet* srcImage, Sheet* dstImage, GainBlob* blob, double gain, double bias,
	int blockLeft, int blockTop, int blockRight, int blockBottom) {

	if (!blob || blob->init(gain, bias))
		return 1;

	return apply_lookup_gray(srcImage, dstImage, blob, blockLeft, blockTop, blockRight, blockBottom);
}



int ImageEffect::glow(Sheet* srcImage, Sheet* dstImage, int softness, int brightness, int contrast,
	int blockLeft, int blockTop, int blockRight, int blockBottom) {
	VALIDATE_IMAGES();
	DECLARE_VARIABLES();
	CLAMP_BLOCK();

	softness = CLAMP3(1, softness, 16);
	contrast = CLAMP3(-100, contrast, 100);
	brightness = CLAMP3(-100, brightness, 100);

	int j, wx, wy, wp, wwx, srcX, srcY, wr;
	int64_t wwxx, waSum, wcSum, aSum, bSum, gSum, rSum, * rSums, * gSums, * bSums, * waSums, * aSums, * wcSums;
	pyte c, ptr;
	int alpha, red, green, blue, r, wlen, localStoreSize, size;
	uint64_t arraysLength;

	int weights[1000 * 2 + 1];
	byte localStore[192096];
	byte lookup[256];
	byte stack[99];

	r = softness;
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

	int i;
	for (i = 0; i <= r; ++i)
	{
		weights[i] = 16 * (i + 1);
		weights[size - i - 1] = weights[i];
	}

	double D = (100 + contrast) / 100.0;

	for (i = 0; i < 256; i++)
		lookup[i] = CLAMP255((i - 128) * D + (brightness + 128) + 0.5);

	for (y = 0; y < h; ++y)
	{
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

	return 0;
}


int ImageEffect::marble(Sheet* srcImage, Sheet* dstImage, double telorance, double scale,
	int blockLeft, int blockTop, int blockRight, int blockBottom) {
	VALIDATE_IMAGES();
	DECLARE_VARIABLES();
	CLAMP_BLOCK();

	telorance = CLAMP3F(0., telorance, 100.);
	scale = CLAMP3F(0., scale, 100.);

	float sinTable[256], cosTable[256], turbulence, xScale, yScale, angle, rx0, rx1, ry0, ry1, * q, a, b, ssx, ssy, t, u, v, f, fx, fy, cx, cy;
	int bx0, bx1, by0, by1, b00, b10, b01, b11, j;
	int displacement, srcX, srcY, k;
	float xWeight, yWeight, m0, m1;
	float* vv;

	int pp[514];
	float g3[514][3];
	float g2[514][2];
	float g1[514];
	int i;

	yScale = xScale = scale;
	turbulence = telorance / 100.0;

	for (i = 0; i < 256; i++)
	{
		angle = PI * i / 256.0 * turbulence;
		sinTable[i] = (float)(-yScale * sin(angle));
		cosTable[i] = (float)(yScale * cos(angle));
	}

	pyte nw, sw, se, ne;
	float out[2];

	for (i = 0; i < 256; i++)
	{
		double s;
		pp[i] = i;
		g1[i] = (float)(((rand() % 0x7fffffff) % 512) - 256) / 256;
		for (j = 0; j < 2; j++)
			g2[i][j] = (float)(((rand() % 0x7fffffff) % 512) - 256) / 256;

		vv = g2[i];
		s = (float)sqrt(vv[0] * vv[0] + vv[1] * vv[1]);
		vv[0] = vv[0] / s;
		vv[1] = vv[1] / s;

		for (j = 0; j < 3; j++)
			g3[i][j] = (float)(((rand() % 0x7fffffff) % 512) - 256) / 256;

		vv = g3[i];
		s = (float)sqrt(vv[0] * vv[0] + vv[1] * vv[1] + vv[2] * vv[2]);
		vv[0] = vv[0] / s;
		vv[1] = vv[1] / s;
		vv[2] = vv[2] / s;
	}

	for (i = 255; i >= 0; i--)
	{
		k = pp[i];
		pp[i] = pp[j = rand() % 256];
		pp[j] = k;
	}

	for (i = 0; i < 258; i++)
	{
		pp[256 + i] = pp[i];
		g1[256 + i] = g1[i];
		for (j = 0; j < 2; j++)
			g2[256 + i][j] = g2[i][j];
		for (j = 0; j < 3; j++)
			g3[256 + i][j] = g3[i][j];
	}


	for (y = blockTop; y < blockBottom; ++y)
	{
		s = src + y * p + blockLeft * 3;
		d = dst + y * p + blockLeft * 3;
		fy = y / xScale;

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


int ImageEffect::median(Sheet* srcImage, Sheet* dstImage, size_t radius, size_t percentile,
	int blockLeft, int blockTop, int blockRight, int blockBottom) {
	VALIDATE_IMAGES();
	DECLARE_VARIABLES();
	CLAMP_BLOCK();

	radius = CLAMP3F(0., radius, 200.);
	percentile = CLAMP3F(0., percentile, 100.);

	int leadingEdgeX[256], ha[256], hr[256], hg[256], hb[256];

	uint32_t hSize = (uint32_t)(sizeof(int) * 256);
	int u, v, left, area, top, minCount1, minCount2, bCount, gCount, rCount, aCount, a1, a2, b1, b2, g1, g2, r1, r2;
	int minCount, g, r, b, a;

	int rad = radius;
	int precent = percentile;
	memset(leadingEdgeX, 0, (rad + 1) * sizeof(int));
	int cutoff = ((rad * 2 + 1) * (rad * 2 + 1) + 2) / 4;

	for (v = 0; v <= rad; ++v)
		for (u = 0; u <= rad; ++u)
			if (u * u + v * v <= cutoff)
				leadingEdgeX[v] = u;

	LR psamp, px, ps, pd;

	for (y = blockTop; y < blockBottom; ++y)
	{
		s = src + y * p + blockLeft * 3;
		d = dst + y * p + blockLeft * 3;

		memset(hb, 0, hSize);
		memset(hg, 0, hSize);
		memset(hr, 0, hSize);

		area = 0;
		ps = (LR)s;
		pd = (LR)d;

		top = -fmin(rad, y);
		bottom = fmin(rad, bottom - y);
		left = -fmin(rad, 0);
		right = fmin(rad, right);

		for (v = top; v <= bottom; ++v)
		{
			psamp = (LR)(src + (y + v) * w + left);

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

		for (x = blockLeft; x < blockRight; ++x)
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

			left = -fmin(rad, x);
			right = fmin(rad + 1, right - x);

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
				px = (ps + (v * w)) - u;

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
				px = (ps + (v * w)) + u + 1;

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
				px = ps + v * w - u;

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
				px = ps + v * w + u + 1;

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

	return 0;
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
	int tx, ty, dx, dy, sc, distance;

	dx = (int)(distance * cos(angle / 180.0 * PI));
	dy = (int)(distance * sin(angle / 180.0 * PI));

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

				xx = Max(Min(ix, right), 0);
				yy = Max(Min(iy, bottom), 0);

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

	return 0;
}


int ImageEffect::old_paint(Sheet* srcImage, Sheet* dstImage, size_t brushSize, size_t coareness,
	int blockLeft, int blockTop, int blockRight, int blockBottom) {
	VALIDATE_IMAGES();
	DECLARE_VARIABLES();
	CLAMP_BLOCK();

	brushSize = CLAMP3F(1., brushSize, 100.);
	coareness = CLAMP3F(3., coareness, 255.);

	int arrayLens = 1 + coareness;
	int localStoreSize = arrayLens * 5 * sizeof(int);
	int i, top, left, numInt, maxInstance, j, r1;
	byte chosenIntensity, intensity;
	static int plocalStore[20480];
	pyte localStore = (pyte)plocalStore;
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

	return 0;
}


int ImageEffect::outline(Sheet* srcImage, Sheet* dstImage, size_t radius, size_t intensity,
	int blockLeft, int blockTop, int blockRight, int blockBottom) {
	VALIDATE_IMAGES();
	DECLARE_VARIABLES();
	CLAMP_BLOCK();

	radius = CLAMP3F(1., radius, 200.);
	intensity = CLAMP3F(0., intensity, 100.);

	int leadingEdgeX[256], ha[256], hr[256], hg[256], hb[256];
	uint32_t hSize = (uint32_t)(sizeof(int) * 256);
	int u, v, left, area, top, minCount1, minCount2, bCount, gCount, rCount, aCount, a1, a2, b1, b2, g1, g2, r1, r2;
	int rad = radius;
	memset(leadingEdgeX, 0, (rad + 1) * sizeof(int));
	int cutoff = ((rad * 2 + 1) * (rad * 2 + 1) + 2) / 4;

	for (v = 0; v <= rad; ++v)
		for (u = 0; u <= rad; ++u)
			if (u * u + v * v <= cutoff)
				leadingEdgeX[v] = u;

	LR psamp, px, ps, pd;

	for (y = blockTop; y < blockBottom; ++y)
	{
		s = src + y * p + blockLeft * 3;
		d = dst + y * p + blockLeft * 3;
		
		memset(hb, 0, hSize);
		memset(hg, 0, hSize);
		memset(hr, 0, hSize);

		area = 0;

		ps = (LR)s;
		pd = (LR)d;

		top = -fmin(rad, y);
		bottom = fmin(rad, bottom - y);
		left = -fmin(rad, 0);
		right = fmin(rad, right);

		for (v = top; v <= bottom; ++v)
		{
			psamp = (LR)(src + (y + v) * w + left);

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

		for (x = blockLeft; x < blockRight; ++x)
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

			left = -fmin(rad, x);
			right = fmin(rad + 1, right - x);

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
				px = (ps + (v * w)) - u;

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
				px = (ps + (v * w)) + u + 1;

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
				px = ps + v * w - u;

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
				px = ps + v * w + u + 1;

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

	return 0;
}


int ImageEffect::pencil_sketch(Sheet* srcImage, Sheet* dstImage, size_t brushSize, size_t range,
	int blockLeft, int blockTop, int blockRight, int blockBottom) {
	VALIDATE_IMAGES();
	DECLARE_VARIABLES();
	CLAMP_BLOCK();

	brushSize = CLAMP3F(1., brushSize, 50.);
	range = CLAMP3F(-20., range, 20.);

	auto weights = new int[100 * 2 + 1];
	auto localStore = new byte[19296];
	auto lookup = new byte[256];

	int j, wx, wy, wp, wwx, srcX, srcY, wr;
	int64_t wwxx, waSum, wcSum, aSum, bSum, gSum, rSum, * rSums, * gSums, * bSums, * waSums, * aSums, * wcSums;
	pyte c, ptr;
	int i, alpha, red, green, blue, r, wlen, localStoreSize, size;
	uint64_t arraysLength;

	r = brushSize;
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

	for (i = 0; i <= r; ++i)
	{
		weights[i] = 16 * (i + 1);
		weights[size - i - 1] = weights[i];
	}

	size = brushSize;
	int sintensity, dintensity;

	for (i = 0; i < 256; i++)
	{
		double d = (100 - range) / 100.0;
		lookup[i] = CLAMP255((i - 128) * d + (range + 128) + 0.5);
	}

#define ChannelBlend_ColorDodge(A, B) ((byte)(((B) == 255) ? (B) : Min(255, (((A) << 8 ) / (255 - (B))))))

	for (y = blockTop; y < blockBottom; ++y)
	{
		s = src + y * p + blockLeft * 3;
		d = dst + y * p + blockLeft * 3;

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

	return 0;
}


int ImageEffect::pixelate(Sheet* srcImage, Sheet* dstImage, size_t cellSize,
	int blockLeft, int blockTop, int blockRight, int blockBottom) {
	VALIDATE_IMAGES();
	DECLARE_VARIABLES();
	CLAMP_BLOCK();

	cellSize = CLAMP3F(1., cellSize, 200.);

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

	return 0;
}


int ImageEffect::radial_blur(Sheet* srcImage, Sheet* dstImage, BlurMode blurMode, double amount, double pivotX, double pivotY,
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
	fcx = pivotX, fcy = pivotY;
	length = amount;
	fr = (int)(amount * PI * 65536.0 / 181.0);
	fsr = fr / RADIUS_LENGTH;
	ffsr = -fsr;
	fcxx = fcx + 32768;
	fcyy = fcy + 32768;
	pyte s2;

	if (blurMode == BlurMode::Spin)
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

					s2 = src + Min(v, bottom) * p + 3 * Min(u, right);

					sb += s2[0];
					sg += s2[1];
					sr += s2[2];

					u = (ox2 + fcxx) >> 16;
					v = (oy2 + fcyy) >> 16;

					s2 = src + Min(v, bottom) * p + 3 * Min(u, right);

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

					s2 = src + Min(v, bottom) * p + 3 * Min(u, right);

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

	return 0;
}



int ImageEffect::random_jitter(Sheet* srcImage, Sheet* dstImage, size_t amount,
	int blockLeft, int blockTop, int blockRight, int blockBottom) {
	VALIDATE_IMAGES();
	DECLARE_VARIABLES();
	CLAMP_BLOCK();

	amount = CLAMP3F(1., amount, 200.);
	auto hamount = amount / 2;
	int ix, iy;

	for (y = blockTop; y < blockBottom; ++y)
	{
		s = src + y * p + blockLeft * 3;
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

	return 0;
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
				un_x = fmod((x + w + floor(amnt * (fabs((((y % length) / (double)length) * 4) - 2) - 1))), w);
				un_x = fmin(un_x, right);
				un_y = y;

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



int ImageEffect::smart_blur(Sheet* srcImage, Sheet* dstImage, size_t radius, size_t threshold,
	int blockLeft, int blockTop, int blockRight, int blockBottom) {
	VALIDATE_IMAGES();
	DECLARE_VARIABLES();
	CLAMP_BLOCK();

	radius = CLAMP3F(3., radius, 100.);
	threshold = CLAMP3F(0., threshold, 255.);

	int i, ix, iy, rad, a1, a2, r1, r2, g1, g2, b1, b2, ir, ig, ib, ia, ra, rows, row, index, cols2, col;
	float* kernel, af, rf, gf, bf, r, g, b, a, f, sigma, sigma22, sigmaPi2, sqrtSigmaPi2, radius2, total;
	kernel = (float*)malloc(1);

	free(kernel);

	ra = (int)ceil(radius);
	rows = ra * 2 + 1;
	kernel = new float[rows];
	sigma = radius / 3;
	sigma22 = 2 * sigma * sigma;
	sigmaPi2 = 2 * PI * sigma;
	sqrtSigmaPi2 = (float)sqrt(sigmaPi2);
	radius2 = radius * radius;
	total = 0;
	index = 0;

	for (row = -ra; row <= ra; row++)
	{
		int d = row * row;
		if (d > radius2) kernel[index] = 0;
		else kernel[index] = (float)exp(-(d) / sigma22) / sqrtSigmaPi2;
		total += kernel[index];
		index++;
	}

	for (i = 0; i < rows; i++)
		kernel[i] /= total;

	cols2 = rows / 2;
	pyte s2;

	for (y = blockTop; y < blockBottom; ++y)
	{
		s = src + y * p + blockLeft * 3;
		d = dst + y * p + blockLeft * 3;
		for (x = blockLeft; x < blockRight; ++x)
		{
			r = 0, g = 0, b = 0;

			r1 = *s++;
			g1 = *s++;
			b1 = *s++;

			rf = 0, gf = 0, bf = 0;

			for (col = -cols2; col <= cols2; col++)
			{
				f = kernel[cols2 + col];

				if (f != 0) {
					ix = x + col;
					if (!(0 <= ix && ix < w))
						ix = x;
					s2 = src + y * p + 3 * ix;
					r2 = *s2++;
					g2 = *s2++;
					b2 = *s2++;

					double d = r1 - r2;
					if (d >= -threshold && d <= threshold) {
						r += f * r2;
						rf += f;
					}
					d = g1 - g2;
					if (d >= -threshold && d <= threshold) {
						g += f * g2;
						gf += f;
					}
					d = b1 - b2;
					if (d >= -threshold && d <= threshold) {
						b += f * b2;
						bf += f;
					}
				}
			}

			r = rf == 0 ? r1 : r / rf;
			g = gf == 0 ? g1 : g / gf;
			b = bf == 0 ? b1 : b / bf;

			*d++ = CLAMP255(b);
			*d++ = CLAMP255(g);
			*d++ = CLAMP255(r);
		}
	}

	for (y = blockTop; y < blockBottom; ++y)
	{
		s = src + y * p + blockLeft * 3;
		d = dst + y * p + blockLeft * 3;
		for (x = blockLeft; x < blockRight; ++x)
		{
			r = 0, g = 0, b = 0;
			rf = 0, gf = 0, bf = 0;

			r1 = src[2];
			g1 = src[1];
			b1 = src[0];

			for (col = -cols2; col <= cols2; col++)
			{
				f = kernel[cols2 + col];

				if (f != 0) {
					iy = y + col;
					if (!(0 <= iy && iy < h))
						iy = y;
					s2 = dst + iy * p + 3 * x;
					r2 = *s2++;
					g2 = *s2++;
					b2 = *s2++;

					double d = r1 - r2;
					if (d >= -threshold && d <= threshold) {
						r += f * r2;
						rf += f;
					}
					d = g1 - g2;
					if (d >= -threshold && d <= threshold) {
						g += f * g2;
						gf += f;
					}
					d = b1 - b2;
					if (d >= -threshold && d <= threshold) {
						b += f * b2;
						bf += f;
					}
				}
			}

			r = rf == 0 ? r1 : r / rf;
			g = gf == 0 ? g1 : g / gf;
			b = bf == 0 ? b1 : b / bf;

			*d++ = CLAMP255(b);
			*d++ = CLAMP255(g);
			*d++ = CLAMP255(r);
		}

	}

	return 0;
}


int ImageEffect::smear(Sheet* srcImage, Sheet* dstImage, SmearMode smearMode, double amount, double mix, double density, double angle,
	int blockLeft, int blockTop, int blockRight, int blockBottom) {
	VALIDATE_IMAGES();
	DECLARE_VARIABLES();
	CLAMP_BLOCK();

	amount = CLAMP3F(1., amount, 100.);
	mix = CLAMP3F(0., mix, 1.);
	density = CLAMP3F(0., density, 1.);
	int dis = amount;

	memcpy(dst, src, h * p);
	double cosAngle = cos(angle / 180.0 * PI);
	double sinAngle = sin(angle / 180.0 * PI);
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

	return 0;
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

int ImageEffect::soft_portrait(Sheet* srcImage, Sheet* dstImage, size_t softness, size_t warmness, int brightness,
	int blockLeft, int blockTop, int blockRight, int blockBottom) {
	VALIDATE_IMAGES();
	DECLARE_VARIABLES();
	CLAMP_BLOCK();

	softness = CLAMP3F(0., softness, 10.);
	warmness = CLAMP3F(0., warmness, 40.);
	brightness = CLAMP3F(-100., brightness, 100.);

	int i, j, wx, wy, wp, wwx, srcX, srcY, wr;
	int64_t wwxx, waSum, wcSum, aSum, bSum, gSum, rSum, * rSums, * gSums, * bSums, * waSums, * aSums, * wcSums;
	pyte c, ptr;
	int alpha, red, green, blue, r, wlen, localStoreSize, size;
	uint64_t arraysLength;

	auto weights = new int[41];
	auto localStore = new byte[41 * 6 * 16];
	byte lookup[256];
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

	for (i = 0; i <= r; ++i)
	{
		weights[i] = 16 * (i + 1);
		weights[size - i - 1] = weights[i];
	}

	for (i = 0; i < 256; i++)
		lookup[i] = Min(Max(i + brightness, 0), 255);

	for (y = blockTop; y < blockBottom; ++y)
	{
		s = src + y * p + blockLeft * 3;
		d = dst + y * p + blockLeft * 3;

		memset(localStore, 0, (uint64_t)localStoreSize);

		waSum = 0;
		wcSum = 0;
		bSum = 0;
		gSum = 0;
		rSum = 0;

		s = src + y * p;
		dst = dst + y * p;

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

	return 0;
}



int ImageEffect::stamp(Sheet* srcImage, Sheet* dstImage, size_t radius, double threshold,
	int blockLeft, int blockTop, int blockRight, int blockBottom) {
	VALIDATE_IMAGES();
	DECLARE_VARIABLES();
	CLAMP_BLOCK();

	radius = CLAMP3F(0., radius, 100.);
	threshold = CLAMP3F(0., threshold, 100.);

	int weights[201];
	byte localStore[201 * 6 * 16];

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

	for (i = 0; i <= r; ++i)
	{
		weights[i] = 16 * (i + 1);
		weights[size - i - 1] = weights[i];
	}

	threshold /= 100.0;

	lowerThreshold3 = 255 * 3 * (threshold - radius * 0.005f);
	upperThreshold3 = 255 * 3 * (threshold + radius * 0.005f);

	if (upperThreshold3 == lowerThreshold3)
		upperThreshold3 = lowerThreshold3 + 1;

	dv = (upperThreshold3 - lowerThreshold3);


	for (y = blockTop; y < blockBottom; ++y)
	{
		d = dst + y * p + blockLeft * 3;

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

	return 0;
}

int ImageEffect::surface_blur(Sheet* srcImage, Sheet* dstImage, double radiusf, double levelf,
	int blockLeft, int blockTop, int blockRight, int blockBottom) {
	VALIDATE_IMAGES();
	DECLARE_VARIABLES();
	CLAMP_BLOCK();

	radiusf = CLAMP3F(1., radiusf, 100.);
	levelf = CLAMP3F(2., levelf, 255.);

	int i;
	auto mat = (float*)malloc(101 * sizeof(float));
	auto imat = (unsigned short*)malloc(202 * sizeof(unsigned short));

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

	for (y = 0; y < h; y++)
	{
		for (x = 0; x < w; x++)
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

			if (blu_fact == 0) dst[0] = *src_db;
			else dst[0] = blu_sum / blu_fact;

			if (grn_fact == 0) dst[1] = *src_dg;
			else dst[1] = grn_sum / grn_fact;

			if (red_fact == 0) dst[2] = *src_dr;
			else dst[2] = red_sum / red_fact;
		}

	}

	return 0;
}

int ImageEffect::swirl(Sheet* srcImage, Sheet* dstImage, double angle, bool shouldStretch, double pivotX, double pivotY,
	int blockLeft, int blockTop, int blockRight, int blockBottom) {
	VALIDATE_IMAGES();
	DECLARE_VARIABLES();
	CLAMP_BLOCK();

	angle = CLAMP3F(-PI / 4, angle, PI / 4);
	pivotX = CLAMP3F(0., pivotX, 1.);
	pivotY = CLAMP3F(0., pivotY, 1.);

	int n = -angle; // fix maybe from 180 to pi
	auto twist = n * n * ((n > 0) ? 1 : -1);

	double un_x, un_y, size;
	int i, nSrcX, nSrcY, nSrcX_1, nSrcY_1;
	double u, v, theta, t, r, fx, fy;
	pyte px0, px1, px2, px3;
	double m0, m1, my;
	size = .9;
	double sc = w > h ? pivotX / pivotY : pivotY / pivotX;
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
			for (y = 0; y < h; y++)
			{
				for (x = 0; x < w; x++)
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
	}
	else
	{
		for (y = 0; y < h; y++)
		{
			for (x = 0; x < w; x++)
			{
				u = (x - pivotX);
				v = (y - pivotY);

				r = sqrt(u * u + v * v);
				theta = atan2(v, u);

				t = 1 - ((r * size) * invmaxrad);
				t = (t < 0) ? 0 : (t * t * t);
				theta += (t * twist) / 100.0;

				un_x = fmin(pivotX + r * cos(theta), right);
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

	return 0;
}
