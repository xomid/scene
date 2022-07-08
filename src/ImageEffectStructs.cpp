#include "ImageEffect.h"

#define CLAMP3F(minValue, value, maxValue) fmin(fmax(value, minValue), maxValue)

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
	threshold = CLAMP3(2, threshold, 255);
	if (!isSet || this->threshold != threshold) {
		for (int i = 0; i < 256; i++)
			gray[i] = 255 * (threshold * i / 256) / (threshold - 1);
		isSet = true;
		this->threshold == threshold;
	}

	return IMAGE_EFFECT_RESULT_OK;
}

int SmartBlurBlob::init(size_t& radius, size_t& threshold) {
	radius = CLAMP3(3, radius, 100);
	threshold = CLAMP3(0, threshold, 255);

	if (!isSet || this->radius != radius || this->threshold != threshold) {
		int i, index, ra, row, rows, sigma, sigma22, sigmaPi2, sqrtSigmaPi2, radius2;

		index = 0;
		ra = (int)ceil(radius);
		rows = ra * 2 + 1; 
		sigma = radius / 3.;
		sigma22 = 2 * sigma * sigma;
		sigmaPi2 = 2 * PI * sigma;
		sqrtSigmaPi2 = (float)sqrt(sigmaPi2);
		radius2 = radius * radius;
		float total = 0;

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

		isSet = true;
		this->radius == radius;
		this->threshold == threshold;
	}

	return IMAGE_EFFECT_RESULT_OK;
}

int StampBlob::init(size_t& radius, double& threshold) {
	radius = CLAMP3(0, radius, 100);
	threshold = CLAMP3F(0., threshold, 100.);

	if (!isSet || this->radius != radius || are_not_equal(this->threshold, threshold)) {
		int r = radius;
		int i, size = 1 + (r * 2);
		
		for (i = 0; i <= r; ++i) {
			weights[i] = 16 * (i + 1);
			weights[size - i - 1] = weights[i];
		}

		isSet = true;
		this->radius == radius;
		this->threshold == threshold;
	}

	return IMAGE_EFFECT_RESULT_OK;
}

int SoftPortraitBlob::init(size_t& softness, size_t& warmness, int& brightness) {
	softness = CLAMP3F(0., softness, 10.);
	warmness = CLAMP3F(0., warmness, 40.);
	brightness = CLAMP3F(-100., brightness, 100.);

	if (!isSet || this->softness != softness || this->warmness != warmness || this->brightness != brightness) {
		int i, r, size;
		
		r = softness;
		size = 1 + (r * 2);

		for (i = 0; i <= r; ++i)
		{
			weights[i] = 16 * (i + 1);
			weights[size - i - 1] = weights[i];
		}

		for (i = 0; i < 256; i++)
			gray[i] = CLAMP255(i + brightness);

		isSet = true;
		this->softness == softness;
		this->warmness == warmness;
		this->brightness == brightness;
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

int MedianBlob::init(size_t& radius, size_t& intensity) {

	radius = CLAMP3(1, radius, 200);
	intensity = CLAMP3(0, intensity, 100);
	if (!isSet || this->radius != radius) {
		auto rad = radius;
		memset(leadingEdgeX, 0, (rad + 1) * sizeof(int));
		auto cutoff = ((rad * 2 + 1) * (rad * 2 + 1) + 2) / 4;
		int u, v;

		for (v = 0; v <= rad; ++v)
			for (u = 0; u <= rad; ++u)
				if (u * u + v * v <= cutoff)
					leadingEdgeX[v] = u;

		isSet = true;
		this->radius == radius;
	}

	return IMAGE_EFFECT_RESULT_OK;
}

int CrystalizeBlob::init() {
	if (!isSet) {
		int i, j, en, st;
		float factorial, total, probability;
		factorial = 1;
		total = 0;
		for (i = 0; i < 10; i++)
		{
			if (i > 1)
				factorial *= i;
			probability = (float)pow(2.5f, i) * (float)exp(-2.5f) / factorial;
			st = (int)(total * 8192);
			total += probability;
			en = (int)(total * 8192);
			for (j = st; j < en; j++)
				probabilities[j] = (byte)i;
		}
		isSet = true;
	}
	return IMAGE_EFFECT_RESULT_OK;
}

float CrystalizeBlob::noise2(float x, float y)
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

float CrystalizeBlob::checkCube(float x, float y, int cubeX, int cubeY)
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

int MarbleBlob::init(double& telorance, double& scale) {

	telorance = CLAMP3F(0., telorance, 100.);
	scale = CLAMP3F(0., scale, 100.);

	if (!isSet || are_not_equal(this->telorance, telorance) || are_not_equal(this->scale, scale)) {
		int i, j, k;
		float* vv, turbulence, angle;
		turbulence = telorance / 100.0;

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

		for (i = 0; i < 256; i++)
		{
			angle = PI * i / 256.0 * turbulence;
			sinTable[i] = (float)(-scale * sin(angle));
			cosTable[i] = (float)(scale * cos(angle));
		}

		isSet = true;
		this->telorance = telorance;
		this->scale = scale;
	}
	return IMAGE_EFFECT_RESULT_OK;
}

int PencilSketchBlob::init(size_t& brushSize, int& range) {
	brushSize = CLAMP3(1, brushSize, 50);
	range = CLAMP3(-20, range, 20);

	if (!isSet || this->brushSize != brushSize || this->range != range) {

		int i, r, size;
		r = brushSize;
		size = 1 + (r * 2);

		for (i = 0; i <= r; ++i)
		{
			weights[i] = 16 * (i + 1);
			weights[size - i - 1] = weights[i];
		}

		for (i = 0; i < 256; i++)
		{
			double d = (100 - range) / 100.0;
			gray[i] = CLAMP255((i - 128) * d + (range + 128) + 0.5);
		}

		isSet = true;
		this->brushSize = brushSize;
		this->range = range;
	}

	return IMAGE_EFFECT_RESULT_OK;
}

int ThresholdBlob::init(byte threshold, bool isMonochromatic) {
	if (!isSet || this->threshold != threshold || this->isMonochromatic != isMonochromatic)
	{
		int i;
		if (isMonochromatic)
		{
			for (i = 0; i < threshold; i++)
				gray[i] = 0;
			for (i = threshold; i < 256; i++)
				gray[i] = 255;
		}
		else for (i = 0; i < 256; i++) 
			gray[i] = i < threshold ? 255 - i : i;

		isSet = true;
		this->threshold = threshold;
		this->isMonochromatic = isMonochromatic;
	}

	return IMAGE_EFFECT_RESULT_OK;
}