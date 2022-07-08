#include "UIDCurves.h"
#include "ImageEffect.h"
#include "UISeparator.h"
#include <oui_button.h>

void UIDCurves::measure_size(int* width, int* height) {
	if (width) *width = 320;
	if (height) *height = 380;
}

void UIDCurves::reset_image() {
	cCurve.set_image(document != NULL ? document->get_image() : 0);
}

void UIDCurves::on_destroy() {
	if (blob) delete blob;
	blob = NULL;
}

void UIDCurves::on_init() {
	set_title(L"Curves");

	blob = new CurvesBlob();

	selPreset.create(this);
	selChannel.create(this);
	grpModes.create(this);
	gradLeft.create(this);
	gradBottom.create(this);
	cCurve.create(this);

	int rdW, rdH, rdM;
	rdM = 5;
	rdW = 25; rdH = 30;
	rdCurve.create(0, 0, rdW, rdH, &grpModes);
	rdPoint.create(rdW + rdM, 0, rdW, rdH, &grpModes);

	gradLeft.set_gradient(gradient_func_black_white, true, true);
	gradBottom.set_gradient(gradient_func_black_white, false, false);

	selPreset.set_title(L"Preset", true);
	selChannel.set_title(L"Channel", true);

	int optH = 30, sepH = 10;
	selPreset.add_option<UIButton>(L"Default", optH);
	selPreset.add_option<UISeparator>(L"", sepH, true);
	selPreset.add_option<UIButton>(L"Color Negative (RGB)", optH);
	selPreset.add_option<UIButton>(L"Cross Process (RGB)", optH);
	selPreset.add_option<UIButton>(L"Darker (RGB)", optH);
	selPreset.add_option<UIButton>(L"Increase Contrast (RGB)", optH);
	selPreset.add_option<UIButton>(L"Lighter (RGB)", optH);
	selPreset.add_option<UIButton>(L"Linear Contrast (RGB)", optH);
	selPreset.add_option<UIButton>(L"Medium Contrast (RGB)", optH);
	selPreset.add_option<UIButton>(L"Negative (RGB)", optH);
	selPreset.add_option<UIButton>(L"Strong Contrast (RGB)", optH);
	selPreset.add_option<UISeparator>(L"", sepH, true);
	selPreset.add_option<UIButton>(L"Custom", optH);

	selChannel.add_option<UIButton>(L"RGB", optH);
	selChannel.add_option<UISeparator>(L"", sepH, true);
	selChannel.add_option<UIButton>(L"Red", optH);
	selChannel.add_option<UIButton>(L"Green", optH);
	selChannel.add_option<UIButton>(L"Blue", optH);

	selPreset.select_option(0);
	selChannel.select_option(0);

	rdCurve.set_text(LR"(
		<svg width="24px" height="24px" viewBox="0 0 24 24" stroke="currentColor">
			  <path d="M17 20C17 21.1046 17.8954 22 19 22C20.1046 22 21 21.1046 21 20C21 18.8954 20.1046 18 19 18C17.8954 18 17 18.8954 17 20ZM17 20H15" stroke-width="1.5" stroke-linecap="round" stroke-linejoin="round"/>
			  <path d="M7 4C7 5.10457 6.10457 6 5 6C3.89543 6 3 5.10457 3 4C3 2.89543 3.89543 2 5 2C6.10457 2 7 2.89543 7 4ZM7 4L9 4" stroke-width="1.5" stroke-linecap="round" stroke-linejoin="round"/>
			  <path d="M14 4L12 4" stroke-width="1.5" stroke-linecap="round" stroke-linejoin="round"/>
			  <path d="M12 20H10" stroke-width="1.5" stroke-linecap="round" stroke-linejoin="round"/>
			  <path d="M3 20C11 20 13 4 21 4" stroke-width="1.5" stroke-linecap="round" stroke-linejoin="round"/>
		</svg>
	)");

	rdPoint.set_text(LR"(
		<svg width="24" height="24" viewBox="0 0 24 24" fill="currentColor">
			<path d="M3 19h18v2H3zm3-3c1.1 0 2-.9 2-2 0-.5-.2-1-.5-1.3L8.8 10H9c.5 0 1-.2 1.3-.5l2.7 1.4v.1c0 1.1.9 2 2 2s2-.9 2-2c0-.5-.2-.9-.5-1.3L17.8 7h.2c1.1 0 2-.9 2-2s-.9-2-2-2-2 .9-2 2c0 .5.2 1 .5 1.3L15.2 9H15c-.5 0-1 .2-1.3.5L11 8.2V8c0-1.1-.9-2-2-2s-2 .9-2 2c0 .5.2 1 .5 1.3L6.2 12H6c-1.1 0-2 .9-2 2s.9 2 2 2z"/>
		</svg>
	)");

	rdCurve.set_type(UIRadioType::Button);
	rdPoint.set_type(UIRadioType::Button);
	rdCurve.padding.set(4);
	rdPoint.padding.set(4);

	if (bCurve = true) {
		rdCurve.select(true);
		cCurve.set_type(CurveType::Spline);
	}
	else {
		rdPoint.select(true);
		cCurve.set_type(CurveType::Point);
	}

	calc_lookup_tables();
}

void UIDCurves::on_resize(int width, int height) {
	UIDEffect::on_resize(width, height);

	int w, h, l, t, margin, groupW;
	w = contentArea.width,
		h = 30;
	l = 0;
	t = 0;
	margin = 5;
	groupW = 65;

	selPreset.move(l, t, w, h); t += h + margin;
	selChannel.move(l, t, w - groupW - margin, h);
	grpModes.move(w - groupW, t, groupW, h); t += h + margin;

	int gradM = 2;
	int gradW = 5;
	h = 240;
	cCurve.move(l + gradW + gradM, t, h, h);
	gradLeft.move(l, t, gradW, h);
	gradBottom.move(l + gradW + gradM, t + h + gradM, h, gradW);
}

void UIDCurves::process_event(OUI* element, uint32_t message, uint64_t param, bool bubbleUp) {
	if (element == &cCurve) {
		calc_lookup_tables();
		selPreset.select_option(10);
		bInvalidate = true;
	}
	else if (element == &selPreset) {
		if (message == Event::Update) {
			int presetId = selPreset.get_selected_option_index();
			load_preset((size_t)presetId);
			calc_lookup_tables();
			bInvalidate = true;
		}
	}
	else if (element == &selChannel) {

		ColorChannel c = ColorChannel::RGB;
		auto selIndex = selChannel.get_selected_option_index();
		switch (selIndex) {
		case 1:
			c = ColorChannel::Red;
			break;
		case 2:
			c = ColorChannel::Green;
			break;
		case 3:
			c = ColorChannel::Blue;
			break;
		}

		cCurve.set_channel(c);
		bInvalidate = true;
	}
	else if (element == &rdCurve || element == &rdPoint) {
		if (message == Event::Select) {
			auto prevState = rdCurve.bSelected;
			element->select(true);
			bCurve = rdCurve.bSelected;
			cCurve.set_type(rdCurve.bSelected ? CurveType::Spline : CurveType::Point);
			bInvalidate = bCurve != prevState;
		}
	}
	else {
		UIDEffect::process_event(element, message, param, bubbleUp);
	}
}

void UIDCurves::calc_lookup_tables() {
	auto valueLookup = cCurve.GetData(0);
	auto redLookup = cCurve.GetData(1);
	auto greenLookup = cCurve.GetData(2);
	auto blueLookup = cCurve.GetData(3);

	memcpy(blob->gray, valueLookup, 256);
	memcpy(blob->red, redLookup, 256);
	memcpy(blob->green, greenLookup, 256);
	memcpy(blob->blue, blueLookup, 256);

	bInvalidate = true;
}

int UIDCurves::render(Sheet* srcImage, Sheet* dstImage, int blockLeft, int blockTop, int blockRight, int blockBottom)
{
	return ImageEffect::curves(srcImage, dstImage, blob, blockLeft, blockTop, blockRight, blockBottom);
}

void UIDCurves::load_preset(size_t presetId)
{
	if (presetId > 9) return;

	cCurve.clear();

	switch (presetId)
	{
	case 1:
		cCurve.red_curve.Add(0, 0);
		cCurve.green_curve.Add(0, 0);
		cCurve.blue_curve.Add(0, 0);

		cCurve.red_curve.Modify(0, 33, 0);
		cCurve.red_curve.Modify(1, 119, 128);
		cCurve.red_curve.Modify(2, 185, 255);
		cCurve.red_curve.FillData();

		cCurve.green_curve.Modify(0, 28, 0);
		cCurve.green_curve.Modify(1, 77, 128);
		cCurve.green_curve.Modify(2, 132, 255);
		cCurve.green_curve.FillData();

		cCurve.blue_curve.Modify(0, 25, 0);
		cCurve.blue_curve.Modify(1, 60, 128);
		cCurve.blue_curve.Modify(2, 108, 255);
		cCurve.blue_curve.FillData();
		break;

	case 2:
		cCurve.red_curve.Add(0, 0); cCurve.red_curve.Add(0, 0); cCurve.red_curve.Add(0, 0);
		cCurve.green_curve.Add(0, 0); cCurve.green_curve.Add(0, 0); cCurve.green_curve.Add(0, 0);
		cCurve.blue_curve.Add(0, 0); cCurve.blue_curve.Add(0, 0);

		cCurve.red_curve.Modify(1, 64, 215);
		cCurve.red_curve.Modify(2, 128, 130);
		cCurve.red_curve.Modify(3, 175, 65);
		cCurve.red_curve.FillData();

		cCurve.green_curve.Modify(1, 64, 207);
		cCurve.green_curve.Modify(2, 97, 127);
		cCurve.green_curve.Modify(3, 190, 47);
		cCurve.green_curve.Modify(4, 255, 47);
		cCurve.green_curve.FillData();

		cCurve.blue_curve.Modify(1, 59, 231);
		cCurve.blue_curve.Modify(2, 181, 32);
		cCurve.blue_curve.FillData();
		break;
	case 3:
		cCurve.val_curve.Add(0, 0);
		cCurve.val_curve.Modify(1, 130, 154);
		cCurve.val_curve.FillData();

		break;

	case 4:
		cCurve.val_curve.Add(0, 0); cCurve.val_curve.Add(0, 0); cCurve.val_curve.Add(0, 0);

		cCurve.val_curve.Modify(1, 38, 238);
		cCurve.val_curve.Modify(2, 212, 24);
		cCurve.val_curve.Modify(3, 231, 5);
		cCurve.val_curve.FillData();

		break;

	case 5:
		cCurve.val_curve.Add(0, 0);
		cCurve.val_curve.Modify(1, 103, 130);
		cCurve.val_curve.FillData();

		break;

	case 6:
		cCurve.val_curve.Add(0, 0); cCurve.val_curve.Add(0, 0);

		cCurve.val_curve.Modify(1, 78, 182);
		cCurve.val_curve.Modify(2, 177, 73);
		cCurve.val_curve.FillData();

		break;

	case 7:
		cCurve.val_curve.Add(0, 0); cCurve.val_curve.Add(0, 0);

		cCurve.val_curve.Modify(1, 73, 199);
		cCurve.val_curve.Modify(2, 163, 91);
		cCurve.val_curve.FillData();

		break;

	case 8:
		cCurve.val_curve.Modify(0, 0, 0);
		cCurve.val_curve.Modify(1, 255, 255);
		cCurve.val_curve.FillData();
		break;

	case 9:
		cCurve.val_curve.Add(0, 0); cCurve.val_curve.Add(0, 0); cCurve.val_curve.Add(0, 0);

		cCurve.val_curve.Modify(1, 77, 205);
		cCurve.val_curve.Modify(2, 151, 102);
		cCurve.val_curve.Modify(3, 175, 67);
		cCurve.val_curve.FillData();

		break;


	default:
		break;
	}

	cCurve.set_type(CurveType::Spline);
	bInvalidate = true;
}