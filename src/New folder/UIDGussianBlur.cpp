#include "UIDGussianBlur.h"
#include "ImageEffect.h"

void UIDGussianBlur::measure_size(int* width, int* height) {
	if (width) *width = 320;
	if (height) *height = 115;
}

void UIDGussianBlur::on_init() {
	set_title(L"Gussian Blur");
	cRadius.create(this);
	cRadius.set_text(L"Radius");
	radius = 0;
	cRadius.config((double)radius, 0., 0., 254., 60);
}

void UIDGussianBlur::on_resize(int width, int height) {
	UIDEffect::on_resize(width, height);

	int w, h, l, t, bottomMargin;
	w = contentArea.width,
	h = 50;
	l = 0;
	t = 0;
	bottomMargin = 5;
	cRadius.move(l, t, w, h);
}

void UIDGussianBlur::process_event(OUI* element, uint32_t message, uint64_t param, bool bubbleUp) {
	if (element == &cRadius) {
		radius = cRadius.get_value();
		bInvalidate = true;
	}
	else {
		UIDEffect::process_event(element, message, param, bubbleUp);
	}
}

void UIDGussianBlur::render(Sheet* srcImage, Sheet* dstImage, int blockLeft, int blockTop, int blockRight, int blockBottom)
{
	/*ImageEffect::posterize(srcImage, dstImage, gain,
		blockLeft, blockTop, blockRight, blockBottom);*/
}
