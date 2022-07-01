#include "UIDContBright.h"
#include "BrightnessData.h"
#include "ContrastData.h"

void UIDContBright::measure_size(int* width, int* height) {
	if (width) *width = 360;
	if (height) *height = 190;
}

void UIDContBright::on_init() {
	scBright.create(0, 0, 10, 10, this);
	scContst.create(0, 0, 10, 10, this);
	chkLegacy.create(0, 0, 10, 10, this);

	scBright.set_text(L"Brightness");
	scBright.config(0, 1, -150, 150);

	scContst.set_text(L"Contrast");
	scContst.config(0, 1, -100, 100);

	chkLegacy.set_text(L"Legacy");
	chkLegacy.select(bLegacy = true);
}

void UIDContBright::on_resize(int width, int height) {
	UIDEffect::on_resize(width, height);

	int w, h, l, t, botttomMargin;
	w = contentArea.width,
		h = 50;
	l = 0;
	t = 0;
	botttomMargin = 5;
	scBright.move(l, t, w, h); t += h + botttomMargin;
	scContst.move(l, t, w, h); t += h + botttomMargin + 5;

	h = 30;
	chkLegacy.move(l, t, 80, h);
}

void UIDContBright::process_event(OUI* element, uint32_t message, uint64_t param, bool bubbleUp) {
	if (element == &scBright) {
		bInvalidate = true;
	}
	else if (element == &scContst) {
		bInvalidate = true;
	}
	else if (element == &chkLegacy) {
		if (message == Event::Select || message == Event::Deselect) {
			auto prevState = bLegacy;
			chkLegacy.select(message == Event::Select);
			bLegacy = chkLegacy.bSelected;
			if (bLegacy) {
				scBright.config(scBright.get_value(), 1, -100, 100);
				scContst.config(scContst.get_value(), 1, -100, 100);
			}
			else {
				scBright.config(scBright.get_value(), 1, -150, 150);
				scContst.config(scContst.get_value(), 1, -50, 100);
			}

			bInvalidate = bLegacy != prevState;
		}
	}
	else {
		UIDEffect::process_event(element, message, param, bubbleUp);
	}

	while (bInvalidate) {
	}

	parent->process_event(this, Event::Update, 0, true);
}

void UIDContBright::apply() {

}

void UIDContBright::render() {
	if (!document) return;

	auto image = document->get_image();
	auto frame = document->get_frame();

	unsigned char blookup[256] = { 0 };
	unsigned char clookup[256] = { 0 };

	int brightness = scBright.get_value();
	int contrast = scContst.get_value();
	int x, y, w, h, p, n, i;
	double slope;
	pyte d, s;

	if (bLegacy)
	{
		x = y = 0;

		if (contrast >= 0) {
			x = (abs(contrast) / 100.0) * 127.5;
			n = (2 * (127.5 - x));
			slope = 255.0 / n;
			n = x + n;

			for (i = x; i <= n; i++) {
				y = (i - x) * slope + 0.5;
				clookup[i] = y;
			}
			for (i = 0; i < x; i++) clookup[i] = 0;
			for (i = n + 1; i < 256; i++) clookup[i] = 255;
			for (i = 0; i < 256; i++) blookup[i] = CLAMP255(i + brightness);

		}
		else {
			y = (abs(contrast) / 100.0) * 127.5;
			slope = (2.0 * (127 - y)) / 255.0;
			for (i = 0; i < 256; i++) blookup[i] = y + (i * slope + 0.5);
			for (i = 0; i < 256; i++) clookup[i] = CLAMP255(i + brightness);
		}

	}
	else
	{
		memcpy(blookup, BrightnessData[brightness + 150], 256);
		memcpy(clookup, ContrastData[contrast + 50], 256);
	}

	if (image && frame) {
		w = image->w;
		h = image->h;
		p = image->pitch;

		for (y = 0; y < h; ++y) {
			s = image->data + y * p;
			d = frame->data + y * p;
			for (x = 0; x < w; ++x) {
				*d++ = clookup[blookup[*s++]];
				*d++ = clookup[blookup[*s++]];
				*d++ = clookup[blookup[*s++]];
			}
		}
	}
}


//#define SC_BRIGHT (EC + 1)
//#define SC_CONTST (EC + 2)
//#define CC_LEGACY (EC + 3)
//#define MAX_WIDTH  273
//#define MAX_HEIGHT 216
//
//BOOL UIDContBright::OnInitDialog()
//{
//	bLegacy = 0;
//	lbright = bright = 0;
//	lcontrast = contrast = 0;
//
//	int cx = MAX_WIDTH - 39;
//	scBright.DestroyWindow();
//	scContst.DestroyWindow();
//	chkLegacy.DestroyWindow();
//
//	chkLegacy.Create(L"Legacy", LEFT, TOP + 98, 60, 15, this, CC_LEGACY);
//	scBright.Create(L"Brightness:", LEFT, TOP, cx, 40, this, SC_BRIGHT);
//	scContst.Create(L"Contrast:", LEFT, TOP + 50, cx, 40, this, SC_CONTST);
//
//	scBright.SetRange(-150, 150);
//	scContst.SetRange(-50, 100);
//	scBright.SetPos(bright);
//	scContst.SetPos(contrast);
//
//	blookup = new byte[256];
//	clookup = new byte[256];
//
//	SetWindowText(L"Brightness / Contrast");
//	bOnceRender = 1;
//	bInvalidate = 1;
//	hThread = CreateThread(0, 0, ApplyEffect, this, 0, 0);
//	return TRUE;
//
//}
//
//void UIDContBright::Apply()
//{
//	hThread = CreateThread(0, 0, ApplyCompleteEffect, this, 0, 0);
//}
//
//void UIDContBright::StopThread()
//{
//	DWORD dwExit = 0;
//	GetExitCodeThread(hThread, &dwExit);
//	TerminateThread(hThread, dwExit);
//	CloseHandle(hThread);
//	EndProcess(0);
//}
//
//void UIDContBright::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
//{
//	UINT id = pScrollBar->GetDlgCtrlID();
//	int val = int(nPos);
//
//	if (id == SC_BRIGHT)
//	{
//		if (val == lbright && (nSBCode != 8)) return;
//		lbright = bright = val;
//	}
//	else if (id == SC_CONTST)
//	{
//		if (val == lcontrast && (nSBCode != 8)) return;
//		lcontrast = contrast = val;
//	}
//
//	if (nSBCode != 8) bInvalidate = 1;
//
//	if (nSBCode == 8)
//	{
//		if (!bEndRendering) bInvalidate = 1;
//		bOnceRender = 1;
//	}
//	else if (!bThread && bEndRendering)
//	{
//		bOnceRender = 0;
//		bInvalidate = 1;
//		hThread = CreateThread(0, 0, ApplyEffect, this, 0, 0);
//	}
//}
//
//void UIDContBright::OnBtnLegacy()
//{
//	bLegacy = !chkLegacy.Get_Check();
//	chkLegacy.Set_Check(bLegacy);
//
//	if (bLegacy)
//	{
//		scBright.SetRange(-100, 100);
//		scContst.SetRange(-100, 100);
//	}
//	else
//	{
//		scBright.SetRange(-150, 150);
//		scContst.SetRange(-50, 100);
//	}
//
//	lbright = bright = 0;
//	lcontrast = contrast = 0;
//
//	bInvalidate = 1;
//	if (!bThread && bEndRendering)
//	{
//		DWORD dwExit = 0;
//		GetExitCodeThread(hThread, &dwExit);
//		TerminateThread(hThread, dwExit);
//		if (hThread) CloseHandle(hThread);
//
//		bOnceRender = 1;
//		bInvalidate = 1;
//		hThread = CreateThread(0, 0, ApplyEffect, this, 0, 0);
//	}
//}
//
//DWORD WINAPI UIDContBright::ApplyEffect(LPVOID lParam)
//{
//	int us, o, x, y, sx, sy, ex, ey, p, i, nb, in;
//	UINT iw, ih, nx, ny, xpart, ypart;
//	UIDContBright* app = (UIDContBright*)lParam;
//	app->bThread = 1; app->bEndRendering = 0;
//	CWnd* pParent = app->GetParent();
//	BOOL* bInvalidate = &app->bInvalidate;
//	pyte disData = app->disData, srcData = app->srcData;
//
//	iw = app->iw;
//	ih = app->ih;
//	p = app->p;
//	nb = app->nbpp;
//	nx = app->nx;
//	ny = app->ny;
//	in = nx * ny;
//	xpart = app->xpart;
//	ypart = app->ypart;
//
//	int ihmax = ih - 1;
//	pyte src, dst, clookup = app->clookup, blookup = app->blookup;
//	BOOL bLegacy;
//	double slope;
//	int n, contrast, bright;
//
//label:
//
//	while (!*bInvalidate)
//		Sleep(100);
//
//	app->bEndRendering = 0;
//	*bInvalidate = 0;
//
//	bLegacy = app->bLegacy;
//	bright = app->bright;
//	contrast = app->contrast;
//
//	if (bLegacy)
//	{
//		x = y = 0;
//
//		if (contrast >= 0) {
//			x = (abs(contrast) / 100.0) * 127.5;
//			n = (2 * (127.5 - x));
//			slope = 255.0 / n;
//			n = x + n;
//
//			for (i = x; i <= n; i++) {
//				y = (i - x) * slope + 0.5;
//				clookup[i] = y;
//			}
//			if (*bInvalidate) goto label;
//			for (i = 0; i < x; i++) clookup[i] = 0;
//			for (i = n + 1; i < 256; i++) clookup[i] = 255;
//			if (*bInvalidate) goto label;
//			for (i = 0; i < 256; i++) blookup[i] = clamp0255(i + bright);
//
//		}
//		else {
//			y = (abs(contrast) / 100.0) * 127.5;
//			slope = (2.0 * (127 - y)) / 255.0;
//			for (i = 0; i < 256; i++) blookup[i] = y + (i * slope + 0.5);
//			for (i = 0; i < 256; i++) clookup[i] = clamp0255(i + bright);
//		}
//
//	}
//	else
//	{
//		memcpy(blookup, BrightnessData[bright + 150], 256);
//		if (*bInvalidate) goto label;
//		memcpy(clookup, ContrastData[contrast + 50], 256);
//		if (*bInvalidate) goto label;
//	}
//
//	if (nb == 1)
//	{
//		for (o = 0; o < in; o++)
//		{
//			if (*bInvalidate) goto label;
//
//			us = o;// num[o];
//
//			sy = (us / nx) * ypart;
//			sx = (us % nx) * xpart;
//			ex = min(sx + xpart, iw);
//			ey = min(sy + ypart, ih);
//
//			for (y = sy; y < ey; y++)
//			{
//				for (x = sx; x < ex; x++)
//				{
//					disData[y * p + x] = clookup[blookup[srcData[y * p + x]]];
//				}
//			}
//
//			pParent->SendMessage(UPDATEIMG);
//			if (*bInvalidate) goto label;
//		}
//	}
//	else if (nb == 3)
//	{
//		for (o = 0; o < in; o++)
//		{
//			if (*bInvalidate) goto label;
//
//			us = o;// num[o];
//
//			sy = (us / nx) * ypart;
//			sx = (us % nx) * xpart;
//			ex = min(sx + xpart, iw);
//			ey = min(sy + ypart, ih);
//
//			for (y = sy; y < ey; y++)
//			{
//				for (x = sx; x < ex; x++)
//				{
//					src = srcData + y * p + 3 * x;
//					dst = disData + y * p + 3 * x;
//
//					dst[0] = clookup[blookup[src[0]]];
//					dst[1] = clookup[blookup[src[1]]];
//					dst[2] = clookup[blookup[src[2]]];
//				}
//			}
//
//			pParent->SendMessage(UPDATEIMG);
//			if (*bInvalidate) goto label;
//		}
//	}
//	else if (nb == 4)
//	{
//		for (o = 0; o < in; o++)
//		{
//			if (*bInvalidate) goto label;
//
//			us = o;// num[o];
//
//			sy = (us / nx) * ypart;
//			sx = (us % nx) * xpart;
//			ex = min(sx + xpart, iw);
//			ey = min(sy + ypart, ih);
//
//			for (y = sy; y < ey; y++)
//			{
//				for (x = sx; x < ex; x++)
//				{
//					src = srcData + y * p + 4 * x;
//					dst = disData + y * p + 4 * x;
//
//					dst[0] = clookup[blookup[src[0]]];
//					dst[1] = clookup[blookup[src[1]]];
//					dst[2] = clookup[blookup[src[2]]];
//					dst[3] = src[3];
//				}
//			}
//
//			pParent->SendMessage(UPDATEIMG);
//			if (*bInvalidate) goto label;
//		}
//	}
//
//	app->bEndRendering = 1;
//	if (app->bOnceRender)
//	{
//		app->bOnceRender = 0;
//		goto end;
//	}
//
//	goto label;
//
//end:
//	app->bThread = 0;
//	return 0;
//}
//
//DWORD WINAPI UIDContBright::ApplyCompleteEffect(LPVOID lParam)
//{
//	LONG x, y, p, i, nb, ylevel, iw, ih;
//	UIDContBright* app = (UIDContBright*)lParam;
//	pyte disData = app->disData, srcData = app->srcData;
//
//	iw = app->iw;
//	ih = app->ih;
//	p = app->p;
//	nb = app->nbpp;
//	ylevel = app->ylevel;
//
//	double all = ih - 1;
//	int ihmax = ih - 1;
//	pyte src, dst, clookup = app->clookup, blookup = app->blookup;
//	BOOL bLegacy;
//	double slope;
//	int n, contrast, bright;
//
//	bLegacy = app->bLegacy;
//	bright = app->bright;
//	contrast = app->contrast;
//
//	if (bLegacy)
//	{
//		x = y = 0;
//
//		if (contrast >= 0) {
//			x = (abs(contrast) / 100.0) * 127.5;
//			n = (2 * (127.5 - x));
//			slope = 255.0 / n;
//			n = x + n;
//
//			for (i = x; i <= n; i++) {
//				y = (i - x) * slope + 0.5;
//				clookup[i] = y;
//			}
//
//			for (i = 0; i < x; i++) clookup[i] = 0;
//			for (i = n + 1; i < 256; i++) clookup[i] = 255;
//			for (i = 0; i < 256; i++) blookup[i] = clamp0255(i + bright);
//
//		}
//		else {
//			y = (abs(contrast) / 100.0) * 127.5;
//			slope = (2.0 * (127 - y)) / 255.0;
//			for (i = 0; i < 256; i++) blookup[i] = y + (i * slope + 0.5);
//			for (i = 0; i < 256; i++) clookup[i] = clamp0255(i + bright);
//		}
//
//	}
//	else
//	{
//		memcpy(blookup, BrightnessData[bright + 150], 256);
//		memcpy(clookup, ContrastData[contrast + 50], 256);
//	}
//
//	if (nb == 1)
//	{
//		for (y = 0; y < ih; y++)
//		{
//			for (x = 0; x < iw; x++)
//				disData[y * p + x] = clookup[blookup[srcData[y * p + x]]];
//
//			if (!(y % ylevel) || y == ihmax) app->SetProgressLevel(int((y / all) * PRECENT_BASE + 0.5));
//		}
//	}
//	else if (nb == 3)
//	{
//		for (y = 0; y < ih; y++)
//		{
//			for (x = 0; x < iw; x++)
//			{
//				src = srcData + y * p + 3 * x;
//				dst = disData + y * p + 3 * x;
//
//				dst[0] = clookup[blookup[src[0]]];
//				dst[1] = clookup[blookup[src[1]]];
//				dst[2] = clookup[blookup[src[2]]];
//			}
//
//			if (!(y % ylevel) || y == ihmax) app->SetProgressLevel(int((y / all) * PRECENT_BASE + 0.5));
//		}
//	}
//	else if (nb == 4)
//	{
//		for (y = 0; y < ih; y++)
//		{
//			for (x = 0; x < iw; x++)
//			{
//				src = srcData + y * p + 4 * x;
//				dst = disData + y * p + 4 * x;
//
//				dst[0] = clookup[blookup[src[0]]];
//				dst[1] = clookup[blookup[src[1]]];
//				dst[2] = clookup[blookup[src[2]]];
//				dst[3] = src[3];
//			}
//
//			if (!(y % ylevel) || y == ihmax) app->SetProgressLevel(int((y / all) * PRECENT_BASE + 0.5));
//		}
//	}
//
//	app->EndProcess(1);
//	return 0;
//}




























































































/*
void UIDContBright::ApplyBrightnessLeg(int val)
{
	int i, n = iw * ih * nbpp;

	for (i = 0; i < n; i++)
	{
		disData[i] = min(max(srcData[i] + val, 0), 255);
	}
}

void UIDContBright::ApplyContrastLeg(int val)
{
	int i, n = iw * ih * nbpp;
	int y, x; x = y = 0;
	double slope;

	if (val >= 0)
	{
		x = (abs(val) / 100.0) * 127.5;
		int n = (2 * (127.5 - x));
		slope = 255.0 / n;
		int i;
		n = x + n;

		for (i = x; i <= n; i++)
		{
			y = (i - x) * slope + 0.5;
			contrast_lookup[i] = y;
		}

		for (i = 0; i < x; i++)
			contrast_lookup[i] = 0;

		for (i = n + 1; i < 256; i++)
			contrast_lookup[i] = 255;
	}
	else
	{
		y = (abs(val) / 100.0) * 127.5;
		slope = (2.0 * (127 - y)) / 255.0;

		for (int i = 0; i < 256; i++)
		{
			contrast_lookup[i] = y + (i * slope + 0.5);
		}
	}

	for (i = 0; i < n; i++)
	{
		disData[i] = contrast_lookup[srcData[i]];
	}
}

void UIDContBright::ApplyBrightness(int val)
{
	int i, n = iw * ih * nbpp;
	pyte data = BrightnessData[val];

	for (i = 0; i < n; i++)
	{
		disData[i] = data[srcData[i]];
	}
}

void UIDContBright::ApplyContrast(int val)
{
	int i, n = iw * ih * nbpp;
	pyte data = ContrastData[val];

	for (i = 0; i < n; i++)
	{
		disData[i] = data[srcData[i]];
	}
}

void UIDContBright::ApplyBrightnessLegBuff(int val)
{
	if (lContrastVal >= 0)
	{
		int i, n = iw * ih * nbpp;

		for (i = 0; i < n; i++)
		{
			disData[i] = contrast_lookup[min(max(srcData[i] + lBrightnessVal, 0), 255)];
		}
	}
	else
	{
		int i, n = iw * ih * nbpp;

		for (i = 0; i < n; i++)
		{
			disData[i] = min(max(contrast_lookup[srcData[i]] + lBrightnessVal, 0), 255);
		}
	}
}

void UIDContBright::ApplyContrastLegBuff(int val)
{
	int i, n = iw * ih * nbpp;
	int y, x; x = y = 0;
	double slope;

	if (val >= 0)
	{
		x = (abs(val) / 100.0) * 127.5;
		int n = (2 * (127.5 - x));
		slope = 255.0 / n;
		int i;
		n = x + n;

		for (i = x; i <= n; i++)
		{
			y = (i - x) * slope + 0.5;
			contrast_lookup[i] = y;
		}

		for (i = 0; i < x; i++)
			contrast_lookup[i] = 0;

		for (i = n + 1; i < 256; i++)
			contrast_lookup[i] = 255;
	}
	else
	{
		y = (abs(val) / 100.0) * 127.5;
		slope = (2.0 * (127 - y)) / 255.0;

		for (int i = 0; i < 256; i++)
		{
			contrast_lookup[i] = y + (i * slope + 0.5);
		}
	}

	ApplyBrightnessLegBuff(lBrightnessVal);
}

void UIDContBright::ApplyBrightnessBuff(int val)
{
	int i, n = iw * ih * nbpp;
	pyte brig = BrightnessData[val];
	pyte cont = ContrastData[lContrastVal];

	for (i = 0; i < n; i++)
	{
		disData[i] = cont[brig[srcData[i]]];
	}
}

void UIDContBright::ApplyContrastBuff(int val)
{
	ApplyBrightnessBuff(lBrightnessVal);
}*/