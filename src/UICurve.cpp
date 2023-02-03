#include "UICurve.h"

#define MAX_WIDTH   260
#define MIN_WIDTH   180
#define DOT_RADIUS  16

void LineTo(double* data, int x1, int y1, int x2, int y2, int w, int h)
{
	double an = atan2((double)y2 - y1, (double)x2 - x1);
	double d = _hypot((double)x2 - x1, (double)y2 - y1);
	int ix, iy, lx, ly, nx, ny;
	double i = 0;
	lx = x1;
	ly = y1;

	if (d > 1)
		int r = 0;

	if (x1 >= 0 && y1 >= 0 && x1 < w && y1 < h)
		data[x1] = y1;
	if (x2 >= 0 && y2 >= 0 && x2 < w && y2 < h)
		data[x2] = y2;

	for (i = 0; i < d; i += 0.1)
	{
		nx = x1 + int(((double)i + 0.1) * cos(an) + 0.5);
		ny = y1 + int(((double)i + 0.1) * sin(an) + 0.5);

		if (abs(nx - lx) < 2 && abs(ny - ly) < 2) continue;

		lx = ix = x1 + int((double)i * cos(an) + 0.5);
		ly = iy = y1 + int((double)i * sin(an) + 0.5);

		if (ix >= 0 && iy >= 0 && ix < w && iy < h)
		{
			data[ix] = iy;
		}
	}
}

Curve* UICurve::GetCmpCurve(int idx) {
	if (idx == 1)
		return &red_curve;
	if (idx == 2)
		return &green_curve;
	if (idx == 3)
		return &blue_curve;
	return &val_curve;
}

void UICurve::fill_image() {
	img.clear(0xff, 0xff, 0xff);
	if (srcImage == NULL || histo == NULL) return;

	int x, y, h1, p, yLevel, lookupIndex, lookupValue;
	int divV = img.h / 4;
	int divH = img.w / 4;
	int h = img.h;
	int w = img.w;

	byte* d, * data, backR, backG, backB, fillR, fillG, fillB;

	backR = 0xff;
	backG = 0xff;
	backB = 0xff;

	fillR = crHisto.r;
	fillG = crHisto.g;
	fillB = crHisto.b;

	data = img.data;
	w = img.w;
	h = img.h;
	p = img.pitch;
	h1 = h - 1;

	for (x = 0; x < w; ++x) {
		lookupIndex = x * 256 / w;
		d = data + x * 3;
		lookupValue = histogramBlob.grayHisto[lookupIndex];
		yLevel = h1 - (lookupValue * h / 256);

		for (y = 0; y < h; ++y, d += p) {
			if (y < yLevel) {
				d[0] = backB;
				d[1] = backG;
				d[2] = backR;
			}
			else {
				d[0] = fillB;
				d[1] = fillG;
				d[2] = fillR;
			}
		}
	}

	// draw grid lines
	Canvas can(0, &img);
	Color stroke(210, 210, 210);
	can.art.strokeColor.set(230, 230, 230);

	can.draw_vertical_line(divH, 0, h, 1);
	can.draw_vertical_line(divH * 2, 0, h, 1);
	can.draw_vertical_line(divH * 3, 0, h, 1);

	can.draw_horizontal_line(divV, 0, w, 1);
	can.draw_horizontal_line(divV * 2, 0, w, 1);
	can.draw_horizontal_line(divV * 3, 0, w, 1);



	static Curve* cur[3] =
	{
		&red_curve,
		&green_curve,
		&blue_curve
	}, * curv;

	static Color crs[3] =
	{
		Color(255, 0, 0),
		Color(0, 255, 0),
		Color(0, 0, 255)
	}, cr;

	if (channel == ColorChannel::RGB || channel == ColorChannel::None)
	{
		int i, q;
		int lx, ly, y;
		Canvas can(0, &img);
		can.art.thickness = 1;

		if (bSmooth)
		{
			int maxx, minx;
			int ra = range - 1;

			for (q = 0; q < 3; q++)
			{
				curv = cur[q];
				if (!curv->IsIntact()) continue;
				minx = (int)curv->xs[0];
				maxx = (int)curv->xs[(int)curv->nCount - 1];
				cr = crs[q];

				can.art.strokeColor.set(cr);
				lx = 2;
				ly = (int)curv->ys[0] + 2;

				for (i = minx; i <= maxx; i++)
				{
					y = Min(Max(curv->evalSpline(i), 0), ra);
					can.draw_line(lx, ly, i + 2, y + 2);
					lx = i + 2;
					ly = y + 2;
				}

				can.draw_line(lx, ly, range + 2, (int)curv->ys[curv->nCount - 1] + 2);
			}
		}
		else
		{
			for (q = 0; q < 3; q++)
			{
				curv = cur[q];
				if (!curv->IsIntact())
					continue;

				cr = crs[q];

				can.art.strokeColor.set(cr);
				lx = 2;
				ly = (int)curv->points_lookup[0] + 2;
				for (i = 0; i < range; i++)
				{
					y = int(curv->points_lookup[i] + 2.5);
					can.draw_line(lx, ly, i + 2, y);
					lx = i + 2;
					ly = y;
				}
			}
		}
	}
}

void UICurve::set_channel(ColorChannel channel) {
	this->channel = channel;

	crLine.set(100, 100, 100);
	crHisto.set(230, 230, 230);

	switch (channel) {
	case ColorChannel::Red:
		histo = histogramBlob.bHisto;
		crLine.set(255, 0, 0);
		crHisto.set(255, 210, 210);
		curve = GetCmpCurve(1);
		break;
	case ColorChannel::Green:
		histo = histogramBlob.gHisto;
		crLine.set(0, 255, 0);
		crHisto.set(210, 255, 210);
		curve = GetCmpCurve(2);
		break;
	case ColorChannel::Blue:
		histo = histogramBlob.rHisto;
		crLine.set(0, 0, 255);
		crHisto.set(210, 236, 255);
		curve = GetCmpCurve(3);
		break;
	default:
		histo = histogramBlob.grayHisto;
		crLine.set(100, 100, 100);
		crHisto.set(230, 230, 230);
		curve = GetCmpCurve(0);
		break;
	}

	iLastSel = -1;
	fill_image();
	invalidate();
}

void UICurve::on_resize(int width, int height) {
	UIHistogram::on_resize(width, height);

	w = area.width; h = area.height;
	range = Max(200, w - 4);

	val_curve.Stretch(range);
	red_curve.Stretch(range);
	green_curve.Stretch(range);
	blue_curve.Stretch(range);

	fill_image();
	invalidate();
}

void UICurve::on_dbl_click(int x, int y, uint32_t flags) {
	on_mouse_down(x, y, flags);
}

void UICurve::on_mouse_down(int x, int y, uint32_t flags) {
	
	iLastSel = -1;

	if (iHover != -2)
	{
		if (iHover == -1 && bNew) 
			process_event(this, Event::Update, 1, true);

		set_capture(this);
		isPressed = true;

		bMoving = 0;
		lastPressedX = x;
		lastPressedY = y;

		if (iHover == -1)
		{
			iHover = curve->Add(x - 2, y - 2);
			curve->FillData();
		}

		iLastSel = iHover;
		curve->points_lookup[x] = y;

		int dx, dy, in, out, ra = curve->range - 1;

		if (iHover == -3)
		{
			dx = Min(Max(x, 2), ra + 2);
			dy = Min(Max(y, 0), ra + 2);
			in = int(double(dx - 2) * 255.0 / (double)ra + 0.5);
			out = (int)curve->value_lookup[in];
		}
		else
		{
			if (iHover < 0 || iHover > (int)curve->nCount)
				int r = 0;

			dx = (int)curve->xs[iHover];
			dy = (int)curve->ys[iHover];
			in = (int)((double)dx * 255. / (double)ra + 0.5);
			out = (int)curve->value_lookup[in];
		}
	}

	invalidate();
}

void UICurve::on_mouse_move(int x, int y, uint32_t flags) {
	if (isPressed)
	{
		if (iHover > -1 && iHover < curve->nCount)
		{
			int dx = x - lastPressedX, dy = y - lastPressedY;
			bMoving = 1;

			if (!dx && !dy) return;
			if (bNew)
				process_event(this, Event::Update, 0, true);

			if (!bSuspend)
			{
				curve->xs[iHover] += dx;
				curve->ys[iHover] += dy;

				if (iHover == 0)
				{
					if (curve->xs[0] >= curve->xs[1])
					{
						dx -= int((curve->xs[0] - curve->xs[1]) + 1);
						curve->xs[0] = curve->xs[1] - 1.;
					}
					else if (curve->xs[0] < 0)
					{
						dx -= (int)curve->xs[0];
						curve->xs[0] = 0;
					}

					if (curve->ys[0] > range - 1)
					{
						dy -= (int)curve->ys[0] - (range - 1);
						curve->ys[0] = (double)(range - 1);
					}
					else if (curve->ys[0] < 0)
					{
						dy -= (int)curve->ys[0];
						curve->ys[0] = 0;
					}
				}
				else if (iHover == curve->nCount - 1)
				{
					if (curve->xs[iHover] <= curve->xs[iHover - 1])
					{
						dx -= (int)(curve->xs[iHover] - curve->xs[iHover - 1] - 1);
						curve->xs[iHover] = curve->xs[iHover - 1] + 1;
					}
					else if (curve->xs[iHover] > range - 1)
					{
						dx -= (int)curve->xs[iHover] - (range - 1);
						curve->xs[iHover] = (double)(range - 1);
					}
					if (curve->ys[iHover] > range - 1)
					{
						dy -= (int)curve->ys[iHover] - (range - 1);
						curve->ys[iHover] = (double)(range - 1);
					}
					else if (curve->ys[iHover] < 0)
					{
						dy -= (int)curve->ys[iHover];
						curve->ys[iHover] = 0;
					}
				}
				else if (curve->xs[iHover] >= curve->xs[iHover + 1])
				{
					bSuspend = 1;
					lastPressedX = (int)curve->xs[iHover - 1] + 1;
					lastPressedY = (int)curve->ys[iHover];
					curve->Remove(iHover);
				}
				else if (curve->xs[iHover] <= curve->xs[iHover - 1])
				{
					bSuspend = 1;
					lastPressedX = (int)curve->xs[iHover + 1] - 1;
					lastPressedY = (int)curve->ys[iHover];
					curve->Remove(iHover);
				}
				lastPressedX += dx;
				lastPressedY += dy;

				if (iHover && iHover < (int)curve->nCount - 1 && (int)curve->ys[iHover] > (range - 1) || (int)curve->ys[iHover] < 0)
				{
					bSuspend = 1;
					lastPressedX = (int)curve->xs[iHover];
					lastPressedY = (int)curve->ys[iHover];
					curve->Remove(iHover);
				}

				curve->FillData();
			}
			else
			{
				lastPressedX += dx;
				lastPressedY += dy;
				if (lastPressedX > (int)curve->xs[iHover - 1] && lastPressedX < (int)curve->xs[iHover] && lastPressedY >= 0 && lastPressedY < range)
				{
					curve->Insert(iHover, lastPressedX, lastPressedY);
					curve->FillData();
					bSuspend = 0;
				}
			}
		}
		else if (iHover == -3)
		{
			int ra = range - 1;
			if (bNew)
				process_event(this, Event::Update, 0, true);

			LineTo(curve->points_lookup, Min(Max(lastPressedX - 2, 0), ra), 
				Min(Max(lastPressedY - 2, 0), ra), Min(Max(x - 2, 0), ra), Min(Max(y - 2, 0), ra), range, range);

			lastPressedX = x;
			lastPressedY = y;

			curve->bIntact = 1;
			curve->FillLookUp();
		}

		if (!bSuspend)
		{
			int dx, dy, in, out, ra = curve->range - 1;

			if (iHover == -3)
			{
				dx = Min(Max(x, 2), ra + 2);
				dy = Min(Max(y, 0), ra + 2);
				in = (int)(double(dx - 2) * 255.0 / (double)ra + 0.5);
				out = (int)curve->value_lookup[in];
			}
			else
			{
				dx = (int)curve->xs[iHover];
				dy = (int)curve->ys[iHover];
				in = (int)((double)dx * 255.0 / (double)ra + 0.5);
				out = (int)curve->value_lookup[in];
			}
		}

		process_event(this, Event::Update, 2, true);
	}
	else
	{
		iHover = DotsHover(x, y);

		if (iHover == -1)
		{
			iCursor = 1;
		}
		else if (iHover > -1)
		{
			iCursor = 2;
		}
		else if (iHover == -2)
		{
			iCursor = 0;
			bCanChangeCursor = 1;
		}
	}


	invalidate();
}

void UICurve::on_mouse_up(int x, int y, uint32_t flags) {
	if (get_capture() == this)
		release_capture();
	isPressed = bSuspend = false;
	iHover = -2;
	bMoving = 0;
	//wndInfo->ShowWindow(SW_HIDE);
	invalidate();
	process_event(this, Event::Update, 8, true);
}

void UICurve::clear() {
	bNew = true;
	val_curve.Reset();
	red_curve.Reset();
	green_curve.Reset();
	blue_curve.Reset();
}

int UICurve::get_range() const {
	return range;
}

void UICurve::on_update() {
	OUI::on_update();
	canvas.bit_blt(img, contentArea.left, contentArea.top, contentArea.width, contentArea.height, 0, 0, true);

	int l, i, x, y;
	int cr = 255;

	Border boxBorder;
	boxBorder.set(1, Colors::purple);

	Rect rc, rcBox; get_content_area(rc);
	Color crBorder(210, 210, 210),
		crBack(cr, cr, cr),
		cr1(185, 185, 185),
		cr2(202, 202, 202),
		cr3(248, 248, 248),
		cr4(215, 215, 215),
		crb1(60, 60, 60),
		crb2(120, 120, 120);

	pixfmt_bgr pf(canvas.sheet, &area);
	agg::path_storage path;
	agg::conv_stroke<agg::path_storage> stroke(path);
	stroke.width(1);
	stroke.line_join(agg::line_join_e::bevel_join);
	stroke.line_cap(agg::line_cap_e::square_cap);
	stroke.inner_join(agg::inner_join_e::inner_bevel);

	canvas.art.strokeColor.set(crLine);

	if (iLastSel >= 0 && iLastSel < curve->nCount)
	{
		if (bMoving && !bSuspend)
		{
			x = (int)curve->xs[iLastSel] + 2;
			y = (int)curve->ys[iLastSel] + 2;

			path.remove_all();
			canvas.reset();

			path.move_to(1, y);
			path.line_to(w - 1, y);
			path.move_to(x, 1);
			path.line_to(x, h - 1);

			canvas.add_path<agg::conv_stroke<agg::path_storage>>(stroke);
			canvas.render(Color(220, 220, 220));
		}

		x = (int)curve->xs[iLastSel], y = (int)curve->ys[iLastSel];
		rcBox.set(x, y, 5, 5);
		canvas.draw_box(rcBox, boxBorder, 0xff);
	}

	int maxx, minx, ra = range - 1;
	curve->nCount = curve->xs.size();
	minx = (int)curve->xs[0];
	maxx = (int)curve->xs[(int)curve->nCount - 1];

	if (bSmooth)
	{
		path.remove_all();
		canvas.reset();

		path.move_to(2., curve->ys[0] + 2.);
		for (i = minx; i <= maxx; i++) {
			y = Min(Max(curve->evalSpline(i), 0), ra);
			path.line_to((double)(i + 2), double(y + 2));
		}
		path.line_to((double)(range + 2), curve->ys[(int)curve->nCount - 1] + 2.);

		canvas.add_path<agg::conv_stroke<agg::path_storage>>(stroke);
		canvas.render(crLine);

		l = (int)curve->nCount;

		for (i = 0; i < l; i++)
		{
			if (i == iLastSel) continue;
			x = (int)curve->xs[i], y = (int)curve->ys[i];
			rcBox.set(x, y, 5, 5);
			canvas.draw_box(rcBox, boxBorder, 0xff);
		}
	}
	else
	{
		path.remove_all();
		canvas.reset();

		for (i = 0; i < range; i++)
		{
			y = (int)(curve->points_lookup[i] + 2.5);
			path.line_to((double)(i + 2), (double)y);
		}

		canvas.add_path<agg::conv_stroke<agg::path_storage>>(stroke);
		canvas.render(crLine);
	}
}

void UICurve::on_init() {
	UIHistogram::on_init();
	curve = NULL;
	channel = ColorChannel::None;
	bCanChangeCursor = 1;
	iCursor = 0;
	bSuspend = 0;
	bMoving = 0;
	bCreated = 0;
	bNew = 1;

	h = w = contentArea.width;
	range = 200;

	val_curve.Init(range, MAX_WIDTH - 4);
	red_curve.Init(range, MAX_WIDTH - 4);
	green_curve.Init(range, MAX_WIDTH - 4);
	blue_curve.Init(range, MAX_WIDTH - 4);

	val_curve.FillData();
	red_curve.FillData();
	green_curve.FillData();
	blue_curve.FillData();

	set_channel(ColorChannel::RGB);
	set_type(CurveType::Spline);
}

void UICurve::set_type(CurveType curveType)
{
	bool bSmoothType = curveType == CurveType::Spline;
	bSmooth = val_curve.bSmooth = red_curve.bSmooth = green_curve.bSmooth = blue_curve.bSmooth = bSmoothType;
	iLastSel = -1;

	val_curve.ConvertTo(bSmoothType);
	red_curve.ConvertTo(bSmoothType);
	green_curve.ConvertTo(bSmoothType);
	blue_curve.ConvertTo(bSmoothType);

	fill_image();
	invalidate();
}

void UICurve::on_mouse_leave(OUI* next)
{
	//SetCursor(AfxGetApp()->LoadStandardCursor(IDC_ARROW));
	bCanChangeCursor = 1;
}

int UICurve::DotsHover(int x, int y)
{
	int i, d;

	if (!bSmooth) return -3;

	for (i = 0; i < curve->nCount; i++)
	{
		d = (int)hypot(x - (int)curve->xs[i], y - (int)curve->ys[i]);
		if (d < DOT_RADIUS)
			return i;
	}

	if (x - 1 > curve->xs[curve->nCount - 1])
		return (int)curve->nCount - 1;
	if (x - 3 < curve->xs[0])
		return 0;

	x = Min(Max(x - 2, 0), range - 1);
	y = Min(Max(y - 2, 0), range - 1);
	y = Min(Max(curve->evalSpline(x - 2), 0), range - 1);

	if (y > y - DOT_RADIUS && y < y + DOT_RADIUS)
		return -1;

	return -2;
}

pyte UICurve::GetData(int idx)
{
	return &GetCmpCurve(idx)->value_lookup[0];
}
