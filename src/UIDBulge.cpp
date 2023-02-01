#include "UIDBulge.h"
#include "ImageEffect.h"

void UIDBulge::measure_size(int* width, int* height) {
	if (width) *width = 320;
	if (height) *height = 280;
}

void UIDBulge::on_init() {
	set_title(L"Bulge");
	cBulge.create(this);
	cAmount.create(this);
	cAmount.set_text(L"Amount");
	amount = 0;
	cAmount.config(amount, 1, -200, 100, 80);
}

void UIDBulge::on_resize(int width, int height) {
	UIDEffect::on_resize(width, height);

	int w, h, l, t, bottomMargin;
	w = contentArea.width,
		h = 160;
	l = 0;
	t = 0;
	bottomMargin = 5;

	cBulge.move(l, t, w, h); t += h + bottomMargin + 5;
	h = 50;
	cAmount.move(l, t, w, h);
}

void UIDBulge::process_event(OUI* element, uint32_t message, uint64_t param, bool bubbleUp) {
	if (element == &cAmount) {
		amount = cAmount.get_value() / 100.;
		cBulge.set_amount(amount);
		shouldInvalidate = true;
	}
	else if (element == &cBulge) {
		shouldStretch = cBulge.is_stretched();
		shouldInvalidate = true;
	}
	else {
		UIDEffect::process_event(element, message, param, bubbleUp);
	}
}

int UIDBulge::render(Sheet* srcImage, Sheet* dstImage, int blockLeft, int blockTop, int blockRight, int blockBottom)
{
	return ImageEffect::bulge(srcImage, dstImage, amount, shouldStretch, .5, .5,
		blockLeft, blockTop, blockRight, blockBottom);
}
