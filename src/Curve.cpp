#include "UICurve.h"

void Curve::Stretch(int newrange)
{
	double s = newrange / (double)range;
	range = int(range * s + 0.5);
	int ra = range - 1;

	for (int i = nCount - 1; i >= 0; i--)
	{
		xs[i] = Min(Max(int(xs[i] * s + 0.5), 0), ra);
		ys[i] = Min(Max(int(ys[i] * s + 0.5), 0), ra);
	}

	FillData();
}
void Curve::Init(int w, int max)
{
	Add(0, w - 1);
	Add(w - 1, 0);
	range = w;
	bSmooth = 1;
	bIntact = 0;

	AA = new double*[256];

	for (int i = 0; i<256; i++)
		AA[i] = new double[256];
}
void Curve::FillData()
{
	int i;
	for (i = 0; i < nCount; i++)
		ks[i] = 1.0;

	Sort();
	getNaturalKs();

	FillLookUp();
}
void Curve::ConvertTo(int mode)
{
	if (mode)
	{
		int i, interval = 40;

		if (bIntact)
		{
			int val, d1, d2, lx, ly;
			erase(1, nCount - 2);
			lx = 0;
			xs[0] = 0;
			xs[1] = range - 1.0;
			ly = ys[0] = points_lookup[0];
			ys[1] = points_lookup[range - 1];

			for (i = 3; i < range - 12; i += 2)
			{
				d1 = (points_lookup[i] - ly) / (double)(i - lx) * 100;
				d2 = (points_lookup[i + 10] - points_lookup[i]) * 10;
				d1 = abs(d2 - d1);

				if (d1 > 10)
				{
					Add(lx = i, ly = int(points_lookup[i]));
					i += 14;
				}
			}
		}

		memset(points_lookup, 0, sizeof(points_lookup));
		bIntact = 0;
	}
	else
	{
		int i, minx, maxx, ra;
		double t, y;
		ra = range - 1;

		minx = xs[0];
		maxx = xs[nCount - 1];

		memset(points_lookup, 0, sizeof(points_lookup));

		for (i = minx; i <= maxx; i++)
		{
			t = i;
			points_lookup[i] = Min(Max(evalSpline(t), 0), ra);
		}

		y = Min(Max(ys[0], 0), 255);

		for (i = 0; i < minx; i++)
			points_lookup[i] = y;

		y = Min(Max(ys[nCount - 1], 0), ra);
		for (i = maxx + 1; i < 256; i++)
			points_lookup[i] = y;
	}

	FillData();
}
void Curve::FillLookUp()
{
	if (bSmooth)
	{
		int i, y, minx, maxx, ra;
		ra = range - 1;

		double s = (range - 1) / 255.0,
			s2 = 255.0 / ra, t;

		minx = xs[0] * s2;
		maxx = xs[nCount - 1] * s2;

		memset(value_lookup, 0, 256);

		for (i = minx; i < maxx; i++)
		{
			t = int(i * s + 0.5);
			value_lookup[i] = 255 - Min(int(Max(Min(evalSpline(t), ra), 0) * s2 + 0.5), 255);
		}

		y = 255 - Min(int(Max(Min(ys[0], ra), 0) * s2 + 0.5), 255);
		for (i = 0; i < minx; i++)
			value_lookup[i] = y;

		y = 255 - Min(int(Max(Min(ys[nCount - 1], ra), 0) * s2 + 0.5), 255);
		for (i = maxx; i < 256; i++)
			value_lookup[i] = y;
	}
	else
	{
		int i, y, minx, maxx, ra, t;
		ra = range - 1;
		if (ra > 255) return;

		double s = ra / 255.0,
			s2 = 255.0 / ra;

		memset(value_lookup, 0, 256);

		for (i = 0; i < 256; i++)
		{
			t = i * s;
			value_lookup[i] = 255 - Min(int(Max(Min(points_lookup[t], ra), 0) * s2 + 0.5), 255);
		}
	}
}
double** Curve::zerosMat(int r, int c)
{
	int i, j;

	for (i = 0; i < r; i++)
	{
		for (j = 0; j < c; j++)
			AA[i][j] = 0;
	}

	return AA;
}
void Curve::swapRows(double **m, int k, int l)
{
	double *p = m[k];
	m[k] = m[l];
	m[l] = p;
}

void Curve::solve(double **A, int len)
{
	int m = len;

	for (int k = 0; k<m; k++)
	{
		int i_max = 0;
		int vali = 0;
		bool bFirst = true;

		for (int i = k; i<m; i++)
		if (A[i][k]>vali || bFirst)
		{
			bFirst = false;
			i_max = i;
			vali = A[i][k];
		}

		swapRows(A, k, i_max);
		constexpr int NEGATIVE_INFINITY = -std::numeric_limits<int>::infinity();

		// for all rows below pivot
		for (int i = k + 1; i<m; i++)
		{
			for (int j = k + 1; j < m + 1; j++)
			{
				double c = A[k][k] ? A[i][j] - A[k][j] * (A[i][k] / A[k][k]) : NEGATIVE_INFINITY;
				A[i][j] = c;
			}

			A[i][k] = 0;
		}
	}

	for (int i = m - 1; i >= 0; i--)	// rows = columns
	{
		double v = A[i][m] / (double)A[i][i];
		ks[i] = v;
		for (int j = i - 1; j >= 0; j--)	// rows
		{
			A[j][m] -= A[j][i] * v;
			A[j][i] = 0;
		}
	}
}
void Curve::getNaturalKs()	// in x values, in y values, out k values
{
	if (nCount < 1) return;
	int n = nCount - 1;
	double **A = zerosMat(n + 1, n + 2);

	for (int i = 1; i<n; i++)	// rows
	{
		A[i][i - 1] = 1.0 / (xs[i] - xs[i - 1]);

		A[i][i] = 2.0 * (1.0 / (xs[i] - xs[i - 1]) + 1.0 / (xs[i + 1] - xs[i]));

		A[i][i + 1] = 1.0 / (xs[i + 1] - xs[i]);

		A[i][n + 1] = 3.0 * ((ys[i] - ys[i - 1]) / ((xs[i] - xs[i - 1])*(xs[i] - xs[i - 1])) + (ys[i + 1] - ys[i]) / ((xs[i + 1] - xs[i])*(xs[i + 1] - xs[i])));
	}

	A[0][0] = 2.0 / (xs[1] - xs[0]);
	A[0][1] = 1.0 / (xs[1] - xs[0]);
	A[0][n + 1] = 3.0 * (ys[1] - ys[0]) / ((xs[1] - xs[0])*(xs[1] - xs[0]));

	A[n][n - 1] = 1.0 / (xs[n] - xs[n - 1]);
	A[n][n] = 2.0 / (xs[n] - xs[n - 1]);
	A[n][n + 1] = 3.0 * (ys[n] - ys[n - 1]) / ((xs[n] - xs[n - 1])*(xs[n] - xs[n - 1]));

	solve(A, (n + 1));
}
int  Curve::evalSpline(int x)
{
	int i = 1;
	while (xs[i]<x) i++;

	double t = (double)(x - xs[i - 1]) / (xs[i] - xs[i - 1]);

	double a = ks[i - 1] * (xs[i] - xs[i - 1]) - (ys[i] - ys[i - 1]);
	double b = -ks[i] * (xs[i] - xs[i - 1]) + (ys[i] - ys[i - 1]);

	double q = (1 - t)*ys[i - 1] + t*ys[i] + t*(1 - t)*(a*(1 - t) + b*t);
	return q + 0.5;
}
double Curve::evalSpline(double x)
{
	int i = 1;
	while (xs[i]<x) i++;

	double t = (double)(x - xs[i - 1]) / (xs[i] - xs[i - 1]);

	double a = ks[i - 1] * (xs[i] - xs[i - 1]) - (ys[i] - ys[i - 1]);
	double b = -ks[i] * (xs[i] - xs[i - 1]) + (ys[i] - ys[i - 1]);

	double q = (1 - t)*ys[i - 1] + t*ys[i] + t*(1 - t)*(a*(1 - t) + b*t);
	return q;
}
void Curve::Sort()
{
	double t = 0;
	int i, j, s;
	s = 1;

	for (i = 0; i < nCount; i++)
	{
		for (j = s; j < nCount; j++)
		{
			if (xs[j] < xs[j - 1])
			{
				t = xs[j];
				xs[j] = xs[j - 1];
				xs[j - 1] = t;

				t = ys[j];
				ys[j] = ys[j - 1];
				ys[j - 1] = t;
			}
		}
	}
}
int  Curve::Add(int x, int y)
{
	ks.push_back(1.0);
	ys.push_back(y);
	xs.push_back(x);
	nCount = ks.size();
	Sort();

	for (int i = 0; i < nCount; i++)
	if (xs[i] == x) return i;
}
void Curve::Remove(int nIndex)
{
	xs.erase(xs.begin() + nIndex, xs.begin() + nIndex + 1);
	ys.erase(ys.begin() + nIndex, ys.begin() + nIndex + 1);
	ks.erase(ks.begin() + nIndex, ks.begin() + nIndex + 1);
	nCount = xs.size();
	Sort();
}
void Curve::erase(int iIndex, int iCount)
{
	int s, e;
	if (iCount < 1) return;

	s = iIndex != -1 ? iIndex : 0;
	e = iCount != -1 ? s + iCount : nCount - 1;
	e = e > nCount ? nCount - 1 : e;

	xs.erase(xs.begin() + s, xs.begin() + e);
	ys.erase(ys.begin() + s, ys.begin() + e);
	ks.erase(ks.begin() + s, ks.begin() + e);

	nCount = ks.size();
}
void Curve::Reset()
{
	ks.clear();
	xs.clear();
	ys.clear();

	ks.push_back(1); ks.push_back(1);
	xs.push_back(0); xs.push_back(range - 1);
	ys.push_back(range - 1); ys.push_back(0);

	bSmooth = 1;
	bIntact = 0;
	nCount = ks.size();

	Sort();
	FillData();
}
void Curve::Insert(int nIndex, int x, int y)
{
	ks.insert(ks.begin() + nIndex, 1.0000000);
	xs.insert(xs.begin() + nIndex, (double)x);
	ys.insert(ys.begin() + nIndex, (double)y);

	nCount = xs.size();
	Sort();
}

bool Curve::IsIntact()
{
	return (bIntact || nCount > 2 || xs[0] != 0 || ys[0] != range - 1 || xs[1] != range - 1 || ys[1] != 0);
}

void Curve::Modify(int nIndex, double x, double y)
{
	if (nIndex < 0 || nIndex >= nCount) return;
	int ra = range - 1;
	xs[nIndex] = int(x / 255.0 * ra + 0.5);
	ys[nIndex] = int(y / 255.0 * ra + 0.5);
}