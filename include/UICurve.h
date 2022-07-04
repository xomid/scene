#pragma once
#include "ImageEffect.h"
#include "UIHistogram.h"

class Curve
{
public:
	Curve()
	{
		value_lookup = new byte[256];
		points_lookup = new double[256];
	}

public:
	std::vector<double> xs;
	std::vector<double> ys;
	std::vector<double> ks;
	byte* value_lookup;
	double* points_lookup;
	int range, nCount;
	bool bSmooth, bIntact;
	double** AA;

public:
	void Stretch(int newrange);
	void Init(int w, int max);
	void FillData();
	void ConvertTo(int mode);
	void FillLookUp();
	double** zerosMat(int r, int c);
	void swapRows(double** m, int k, int l);
	void solve(double** A, int len);
	void getNaturalKs();
	int evalSpline(int x);
	double evalSpline(double x);
	void Sort();
	int Add(int x, int y);
	void Remove(int nIndex);
	void erase(int iIndex, int iCount);
	void Reset();
	void Insert(int nIndex, int x, int y);
	bool IsIntact();
	void Modify(int nIndex, double x, double y);
};

enum class CurveType {
	Spline,
	Point
};

class UICurve : public UIHistogram {
public:
	Curve val_curve, red_curve, green_curve, blue_curve;
protected:
	Curve *curve;
	bool bCanChangeCursor, bSuspend, bMoving, bCreated, bSmooth, bNew;
	int w, h, iHover, iCursor, range, iLastSel, lastPressedX, lastPressedY;

	Color crHisto, crLine;
	ColorChannel channel; //0: gray, 1-3: r-g-b
	unsigned long* histo;

	void fill_image() override;
public:

	int get_range() const;
	void clear();
	void set_type(CurveType curveType);
	Curve* GetCmpCurve(int idx);
	pyte GetData(int idx);
	int DotsHover(int x, int y);


	void on_mouse_leave(OUI* next) override;
	void on_init() override;
	void set_channel(ColorChannel channel);
	void on_resize(int width, int height) override;
	void on_update() override;
	void on_mouse_move(int x, int y, uint32_t flags) override;
	void on_mouse_down(int x, int y, uint32_t flags) override;
	void on_dbl_click(int x, int y, uint32_t flags) override;
	void on_mouse_up(int x, int y, uint32_t flags) override;
};



