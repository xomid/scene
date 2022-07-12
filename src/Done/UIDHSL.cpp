#include "UIDHSL.h"
#include "ImageEffect.h"
#include "UISeparator.h"

void gradient_func_hsl(void* param, double t, Color& res) {
	hsb_to_rgb(int(t * 360. + .5), 100, 100, res.r, res.g, res.b);
}

void gradient_func_gray_blue(void* param, double t, Color& res) {
	res.b = 127 + int(t * 128 + .5);
	res.r = res.g = 127 - int(t * 127 + .5);
}

void UIDHSL::HSL::reset() {
	hue = 0;
	saturation = 0;
	lightness = 0;
}

void UIDHSL::measure_size(int* width, int* height) {
	if (width) *width = 320;
	if (height) *height = 290;
}

void UIDHSL::on_init() {
	set_title(L"Hue/Saturation");
	
	selPreset.create(this);
	cHue.create(this);
	cSaturation.create(this);
	cLightness.create(this);
	chkColorize.create(this);

	cHue.set_text(L"Hue");
	cSaturation.set_text(L"Saturation");
	cLightness.set_text(L"Lightness");

	cHue.set_gradient(gradient_func_hsl);
	cSaturation.set_gradient(gradient_func_gray_blue);
	cLightness.set_gradient(gradient_func_black_white);

	hsl.reset();
	colorizeHsl.reset();
	normalHsl.reset();

	cHue.config(0., 1, -180, 180, 60);
	cSaturation.config(0., 1, -100, 100, 60);
	cLightness.config(0., 1, -100, 100, 60);

	selPreset.set_title(L"Preset");
	chkColorize.set_text(L"Colorize");
	chkColorize.select(shouldColorize = true);

	int optH = 30, sepH = 10;
	selPreset.add_option<UIButton>(L"Default", optH);
	selPreset.add_option<UISeparator>(L"", sepH, true);
	selPreset.add_option<UIButton>(L"Cyanotype", optH);
	selPreset.add_option<UIButton>(L"Increase Saturation More", optH);
	selPreset.add_option<UIButton>(L"Increase Saturation", optH);
	selPreset.add_option<UIButton>(L"Old Style", optH);
	selPreset.add_option<UIButton>(L"Red Boost", optH);
	selPreset.add_option<UIButton>(L"Sepia", optH);
	selPreset.add_option<UIButton>(L"Strong Saturation", optH);
	selPreset.add_option<UIButton>(L"Yellow Boost", optH);
	selPreset.add_option<UISeparator>(L"", sepH, true);
	selPreset.add_option<UIButton>(L"Custom", optH);

	selPreset.select_option(0);
	load_preset(0);
}

void UIDHSL::on_resize(int width, int height) {
	UIDEffect::on_resize(width, height);

	int w, h, l, t, botttomMargin;
	w = contentArea.width,
		h = 30;
	l = 0;
	t = 0;
	botttomMargin = 5;

	selPreset.move(l, t, w, h); t += h + botttomMargin + 5;

	h = 50;
	cHue.move(l, t, w, h); t += h + botttomMargin;
	cSaturation.move(l, t, w, h); t += h + botttomMargin;
	cLightness.move(l, t, w, h); t += h + botttomMargin + 5;

	h = 30;
	chkColorize.move(l, t, 80, h);
}

void UIDHSL::process_event(OUI* element, uint32_t message, uint64_t param, bool bubbleUp) {
	if (element == &selPreset) {
		if (message == Event::Update) {
			int presetId = selPreset.get_selected_option_index();
			if (presetId >= 1) {
				load_preset((size_t)presetId);
			}
		}
	}
	else {
		bool presetCustomized = true;

		if (element == &cHue) {
			hsl.hue = cHue.get_value();
			shouldInvalidate = true;
		}
		else if (element == &cSaturation) {
			hsl.saturation = cSaturation.get_value();
			shouldInvalidate = true;
		}
		else if (element == &cLightness) {
			hsl.lightness = cLightness.get_value();
			shouldInvalidate = true;
		}
		else if (element == &chkColorize) {
			if (message == Event::Select || message == Event::Deselect) {
				auto prevState = shouldColorize;
				chkColorize.select(message == Event::Select);
				shouldColorize = chkColorize.bSelected;
				colorize();
				shouldInvalidate = shouldColorize != prevState;
			}
		}
		else {
			presetCustomized = false;
			UIDEffect::process_event(element, message, param, bubbleUp);
		}

		if (presetCustomized)
			load_preset(9);
	}

}

int UIDHSL::render(Sheet* srcImage, Sheet* dstImage, int blockLeft, int blockTop, int blockRight, int blockBottom)
{
	return ImageEffect::hsl(srcImage, dstImage, &blob, shouldColorize, hsl.hue, hsl.saturation, hsl.lightness,
		blockLeft, blockTop, blockRight, blockBottom);
}

void UIDHSL::colorize() {
	if (shouldColorize)
	{
		normalHsl = hsl;
		hsl = colorizeHsl;
		cHue.config((double)hsl.hue, 1., 0., 360., 60);
		cSaturation.config((double)hsl.saturation, 1., 0., 100., 60);
	}
	else
	{
		colorizeHsl = hsl;
		hsl = normalHsl;
		cHue.config((double)hsl.hue, 1., -180., 180., 60);
		cSaturation.config((double)hsl.saturation, 1., -100., 100., 60);
	}

	shouldInvalidate = true;
}

void UIDHSL::load_preset(size_t presetId) {
	if (presetId > 8) {
		if (selPreset.get_selected_option_index() != 9) 
			selPreset.select_option(9);
	}
	else {
		hsl.reset();
		colorizeHsl.reset();
		normalHsl.reset();
		shouldColorize = false;
		colorizeHsl.hue = presetId ? 66 : 0;
		colorizeHsl.saturation = 25;

		switch (presetId)
		{
		case 1:
			shouldColorize = true;
			colorizeHsl.hue = 215;
			break;
		case 2:
			normalHsl.saturation = 30;
			break;
		case 3:
			normalHsl.saturation = 10;
			break;
		case 4:
			normalHsl.saturation = -40;
			normalHsl.lightness = 5;
			break;
		case 5:
			normalHsl.hue = -5;
			normalHsl.saturation = 20;
			break;
		case 6:
			colorizeHsl.hue = 35;
			shouldColorize = true;
			break;
		case 7:
			normalHsl.saturation = 50;
			break;
		case 8:
			colorizeHsl.hue = 18;
			normalHsl.hue = 5;
			normalHsl.saturation = 20;
			break;
		default:
			break;
		}

		chkColorize.select(shouldColorize);

		if (shouldColorize)
		{
			hsl = colorizeHsl;
			cHue.config((double)hsl.hue, 1., 0., 360., 60);
			cSaturation.config((double)hsl.saturation, 1., 0., 100., 60);
		}
		else
		{
			hsl = normalHsl;
			cHue.config((double)hsl.hue, 1., -180., 180., 60);
			cSaturation.config((double)hsl.saturation, 1., -100., 100., 60);
		}

		cLightness.config((double)hsl.lightness, 1., -100., 100., 60);

		shouldInvalidate = true;
	}
}
