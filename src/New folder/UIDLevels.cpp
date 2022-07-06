#include "UIDLevels.h"
#include "ImageEffect.h"
#include "UISeparator.h"

void UIDLevels::measure_size(int* width, int* height) {
	if (width) *width = 320;
	if (height) *height = 390;
}

void UIDLevels::reset_image() {
	cHisto.set_image(document ? document->get_image() : 0);
}

void UIDLevels::on_init() {
	set_title(L"Levels");

	selPreset.create(this);
	selChannel.create(this);
	cHisto.create(this);
	numLow.create(this);
	numMid.create(this);
	numHigh.create(this);
	numMin.create(this);
	numMax.create(this);
	slRange.create(this);
	slOutput.create(this);

	lblOutput.create(this);
	lblOutput.set_text(L"Output Levels");

	RangedFloat::debugMode = true;

	slRange.set_bound_range(0., 1., .01);
	slRange.set_range(0., 255.);
	slRange.set_value(0, 0.);
	slRange.set_value(2, 255.);
	slRange.set_value(1, .5);

	slOutput.set_range(0., 255.);
	slOutput.set_value(0, 0.);
	slOutput.set_value(1, 255.);
	slOutput.set_gradient_func(gradient_func_black_white);
	slOutput.set_handle_color(0, Colors::black);
	slOutput.set_handle_color(1, Colors::white);

	numLow.set_range(0., 1., 0., 255.);
	numMid.set_range("1.", "0.01", "0.01", "9.99");
	numHigh.set_range(255., 1., 0., 255.);
	numMin.set_range(0., 1., 0., 255.);
	numMax.set_range(255., 1., 0., 255.);

	slOutput.set_gradient_func(gradient_func_black_white);

	selPreset.set_title(L"Preset", true);
	selChannel.set_title(L"Channel", true);

	int optH = 30, sepH = 10;
	selPreset.add_option<UIButton>(L"Default", optH);
	selPreset.add_option<UISeparator>(L"", sepH, true);
	selPreset.add_option<UIButton>(L"Darker", optH);
	selPreset.add_option<UIButton>(L"Increase Contrast 1", optH);
	selPreset.add_option<UIButton>(L"Increase Contrast 2", optH);
	selPreset.add_option<UIButton>(L"Increase Contrast 3", optH);
	selPreset.add_option<UIButton>(L"Lighten Shadows", optH);
	selPreset.add_option<UIButton>(L"Lighter", optH);
	selPreset.add_option<UIButton>(L"Midtones Brighter", optH);
	selPreset.add_option<UIButton>(L"Midtones Darker", optH);
	selPreset.add_option<UISeparator>(L"", sepH, true);
	selPreset.add_option<UIButton>(L"Custom", optH);

	selChannel.add_option<UIButton>(L"RGB", optH);
	selChannel.add_option<UISeparator>(L"", sepH, true);
	selChannel.add_option<UIButton>(L"Red", optH);
	selChannel.add_option<UIButton>(L"Green", optH);
	selChannel.add_option<UIButton>(L"Blue", optH);

	selPreset.select_option(0);
	selChannel.select_option(0);

	Border bor;
	bor.set(1, backgroundColor.bright(-30));
	numLow.border = bor;
	numMid.border = bor;
	numHigh.border = bor;
	numMin.border = bor;
	numMax.border = bor;
}

void UIDLevels::on_resize(int width, int height) {
	UIDEffect::on_resize(width, height);

	int w, h, l, t, botttomMargin;
	w = contentArea.width,
		h = 30;
	l = 0;
	t = 0;
	botttomMargin = 5;
	int selChannelW = 100;

	selPreset.move(l, t, w, h); t += h + botttomMargin;
	selChannel.move(l, t, selChannelW, h); t += h + botttomMargin;

	h = 130;
	cHisto.move(l, t, w, h); t += h + botttomMargin;

	int numW = 70,
		numH = 30;

	numLow.move(l, t, numW, numH);
	numMid.move((w - numW + 1) / 2, t, numW, numH);
	numHigh.move(w - numW, t, numW, numH);

	t += numH + botttomMargin;
	h = 20;
	slRange.move(l, t, w, h); t += h + botttomMargin;

	int labelW = 120;
	int margin = 5;
	lblOutput.move(l, t, labelW, numH);
	numMin.move(w - numW * 2 - margin, t, numW, numH);
	numMax.move(w - numW, t, numW, numH); t += numH + botttomMargin;

	slOutput.move(l, t, w, h);
}

void UIDLevels::process_event(OUI* element, uint32_t message, uint64_t param, bool bubbleUp) {
	if (message == Event::Update) {
		if (element == &numLow) {
			slRange.set_value(0, atof(numLow.get_number().c_str()));
		}
		else if (element == &numMid) {
			auto f = atof(numMid.get_number().c_str());
			auto t = 1. - pow((f - 0.01) / 9.98, 0.299998);
			slRange.set_value(1, t);
		}
		else if (element == &numHigh) {
			slRange.set_value(1, atof(numHigh.get_number().c_str()));
		}
		else if (element == &numMin) {
			slOutput.set_value(0, atof(numMin.get_number().c_str()));
		}
		else if (element == &numMax) {
			slOutput.set_value(1, atof(numMax.get_number().c_str()));
		}
		else if (element == &slRange) {
			numLow.set_value(slRange.get_value(0));
			numHigh.set_value(slRange.get_value(2));
			auto t = slRange.get_value(1); // [0-1]
			// convert to [.01-9.99]
			auto f = pow(1. - t, 1 / 0.299998) * 9.98 + 0.01;
			numMid.set_value(f);
		}
		else if (element == &slOutput) {
			numMin.set_value(slOutput.get_value(0));
			numMax.set_value(slOutput.get_value(1));
		}
		bInvalidate = true;
	}
	else {
		UIDEffect::process_event(element, message, param, bubbleUp);
	}
}

void UIDLevels::render(Sheet* srcImage, Sheet* dstImage, int blockLeft, int blockTop, int blockRight, int blockBottom)
{
	/*ImageEffect::levels(srcImage, dstImage, &blob, low, mid, high, min, max,
		blockLeft, blockTop, blockRight, blockBottom);*/
}
