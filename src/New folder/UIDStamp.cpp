#include "UIDStamp.h"
#include "ImageEffect.h"

void UIDStamp::measure_size(int* width, int* height) {
	if (width) *width = 320;
	if (height) *height = 170;
}

void UIDStamp::on_init() {
	set_title(L"Stamp");
	cRadius.create(this);
	cThreshold.create(this);
	cRadius.set_text(L"Radius");
	cThreshold.set_text(L"Threshold");
	radius = 0;
	threshold = 0;
	cRadius.config(0., 1., 0., 100., 60);
	cThreshold.config(0., 1., 0., 100., 60);
}

void UIDStamp::on_resize(int width, int height) {
	UIDEffect::on_resize(width, height);

	int w, h, l, t, bottomMargin;
	w = contentArea.width,
		h = 50;
	l = 0;
	t = 0;
	bottomMargin = 5;
	cRadius.move(l, t, w, h); t += h + bottomMargin;
	cThreshold.move(l, t, w, h);
}

void UIDStamp::process_event(OUI* element, uint32_t message, uint64_t param, bool bubbleUp) {
	if (element == &cRadius) {
		radius = (int)cRadius.get_value();
		bInvalidate = true;
	}
	else if (element == &cThreshold) {
		threshold = (int)cThreshold.get_value();
		bInvalidate = true;
	}
	else {
		UIDEffect::process_event(element, message, param, bubbleUp);
	}
}

void UIDStamp::render(Sheet* srcImage, Sheet* dstImage, int blockLeft, int blockTop, int blockRight, int blockBottom)
{
	/*ImageEffect::posterize(srcImage, dstImage, radius,
		blockLeft, blockTop, blockRight, blockBottom);*/
}