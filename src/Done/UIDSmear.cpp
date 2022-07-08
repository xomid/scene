#include "UIDSmear.h"
#include "ImageEffect.h"

void UIDSmear::measure_size(int* width, int* height) {
	if (width) *width = 320;
	if (height) *height = 270;
}

void UIDSmear::on_init() {
	set_title(L"Smear");

	selMode.create(this);
	cAmount.create(this);
	cMix.create(this);
	cDensity.create(this);

	cAmount.set_text(L"Amount");
	cMix.set_text(L"Mix");
	cDensity.set_text(L"Density");

	int numFieldW = 90;
	amount = 1;
	mix = 0;
	density = 0;
	cAmount.config(1., 1., 1., 100., numFieldW);
	cMix.config("0.00", ".01", "0.00", "1.00", numFieldW);
	cDensity.config("0.00", "0.01", "0.00", "1.00", numFieldW);

	int optH = 30;
	selMode.add_option<UIButton>(L"Crosses", optH);
	selMode.add_option<UIButton>(L"Lines", optH);
	selMode.add_option<UIButton>(L"Circles", optH);
	selMode.add_option<UIButton>(L"Squares", optH);
	selMode.set_title(L"Mode");
	selMode.select_option(0);
	smearMode = SmearMode::Crosses;
}

void UIDSmear::on_resize(int width, int height) {
	UIDEffect::on_resize(width, height);

	int w, h, l, t, bottomMargin;
	w = contentArea.width,
		h = 30;
	l = 0;
	t = 0;
	bottomMargin = 5;

	selMode.move(l, t, w, h); t += h + bottomMargin + 5;
	h = 50;
	cAmount.move(l, t, w, h); t += h + bottomMargin;
	cMix.move(l, t, w, h); t += h + bottomMargin;
	cDensity.move(l, t, w, h);
}

void UIDSmear::process_event(OUI* element, uint32_t message, uint64_t param, bool bubbleUp) {
	if (element == &selMode) {
		auto index = selMode.get_selected_option_index();
		switch (index) {
		case 1:
			smearMode = SmearMode::Lines;
			break;
		case 2:
			smearMode = SmearMode::Circles;
			break;
		case 3:
			smearMode = SmearMode::Squares;
			break;
		default:
			smearMode = SmearMode::Crosses;
			break;
		}
		bInvalidate = true;
	}
	else if (element == &cAmount) {
		amount = (int)cAmount.get_value();
		bInvalidate = true;
	}
	else if (element == &cMix) {
		mix = cMix.get_value();
		bInvalidate = true;
	}
	else if (element == &cDensity) {
		density = cDensity.get_value();
		bInvalidate = true;
	}
	else {
		UIDEffect::process_event(element, message, param, bubbleUp);
	}
}

int UIDSmear::render(Sheet* srcImage, Sheet* dstImage, int blockLeft, int blockTop, int blockRight, int blockBottom)
{
	return ImageEffect::smear(srcImage, dstImage, smearMode, amount, mix, density, 0,
		blockLeft, blockTop, blockRight, blockBottom);
}
