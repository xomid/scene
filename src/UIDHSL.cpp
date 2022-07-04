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

	hue = saturation = lightness = 0;
	cHue.config((double)hue, 1, -180, 180);
	cSaturation.config((double)saturation, 1, -100, 100);
	cLightness.config((double)lightness, 1, -100, 100);

	selPreset.set_title(L"Preset");
	chkColorize.set_text(L"Colorize");
	chkColorize.select(bColorize = true);

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
			hue = cHue.get_value();
			bInvalidate = true;
		}
		else if (element == &cSaturation) {
			saturation = cSaturation.get_value();
			bInvalidate = true;
		}
		else if (element == &cLightness) {
			lightness = cLightness.get_value();
			bInvalidate = true;
		}
		else if (element == &chkColorize) {
			if (message == Event::Select || message == Event::Deselect) {
				auto prevState = bColorize;
				chkColorize.select(message == Event::Select);
				bColorize = chkColorize.bSelected;
				colorize();
				bInvalidate = bColorize != prevState;
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

void UIDHSL::render(Sheet* srcImage, Sheet* dstImage, int blockLeft, int blockTop, int blockRight, int blockBottom)
{
	ImageEffect::hsl(srcImage, dstImage, &blob, bColorize, hue, saturation, lightness,
		blockLeft, blockTop, blockRight, blockBottom);
}

void UIDHSL::colorize() {
	/*int h, s, l;
	h = s = l = 0;
	double val[6] = { 0, 252, 255, 0, 0.25, 0 };

	if (bColorize)
	{
		lNocHue = lHue; lNocSat = lSat; lNocLit = lLit;
		lHue = lColHue; lSat = lColSat; lLit = lColLit;
		scHue.SetRange(0, 360, lColHue);
		scSat.SetRange(0, 100, lColSat);
		scLit.SetRange(-100, 100, lColLit);
	}
	else
	{
		lColHue = lHue; lColSat = lSat; lColLit = lLit;
		lHue = lNocHue; lSat = lNocSat; lLit = lNocLit;
		scHue.SetRange(-180, 180, lNocHue);
		scSat.SetRange(-100, 100, lNocSat);
		scLit.SetRange(-100, 100, lNocLit);
	}*/

	if (bColorize) {
		cHue.config((double)hue, 1, -180, 180);
		cSaturation.config((double)saturation, 1, -100, 100);
	}
	else {
		cHue.config((double)hue, 1, 0, 360);
		cSaturation.config((double)saturation, 1, 0, 100);
	}

	hue = (int)cHue.get_value();
	saturation = (int)cSaturation.get_value();
}

void UIDHSL::load_preset(size_t presetId) {
	/*if (presetId > 8) {
		selPreset.select_option(9);
	}
	else {
		bNew = 1;
		lHue = lSat = lLit = 0;
		bColorize = 0;
		lColHue = nPreset ? 66 : 0;
		lColSat = 25;
		lColLit = 0;
		lNocHue = 0;
		lNocSat = 0;
		lNocLit = 0;

		switch (nPreset)
		{
		case 1:
			bColorize = 1;
			lColHue = 215;
			break;
		case 2:
			lNocSat = 30;
			break;
		case 3:
			lNocSat = 10;
			break;
		case 4:
			lNocSat = -40;
			lNocLit = 5;
			break;
		case 5:
			lNocHue = -5;
			lNocSat = 20;
			break;
		case 6:
			lColHue = 35;
			bColorize = 1;
			break;
		case 7:
			lNocSat = 50;
			break;
		case 8:
			lColHue = 18;
			lNocHue = 5;
			lNocSat = 20;
			break;
		default:
			break;
		}

		chkColorize.Set_Check(bColorize);

		if (bColorize)
		{
			lHue = lColHue; lSat = lColSat; lLit = lColLit;
			cHue.config(lColHue, 1., 0., 360.);
			scSat.config(lColSat, 1., 0., 100);
			scSat.SetRange(0, 100, lColSat);
			scLit.SetRange(-100, 100, lColLit);
		}
		else
		{
			lHue = lNocHue; lSat = lNocSat; lLit = lNocLit;
			cHue.SetRange(-180, 180, lNocHue);
			scSat.SetRange(-100, 100, lNocSat);
			scLit.SetRange(-100, 100, lNocLit);
		}

		ApplyHue();
	}*/
}
