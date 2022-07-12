#include "UIDSmartBlur.h"
#include "ImageEffect.h"

void UIDSmartBlur::measure_size(int* width, int* height) {
	if (width) *width = 320;
	if (height) *height = 170;
}

void UIDSmartBlur::on_destroy() {
	if (blob) delete blob;
	blob = NULL;
}

void UIDSmartBlur::on_init() {
	set_title(L"Smart Blur");
	cRadius.create(this);
	cThreshold.create(this);
	cRadius.set_text(L"Radius");
	cThreshold.set_text(L"Threshold");
	radius = 3;
	threshold = 0;
	cRadius.config(3., 1., 3., 100., 60);
	cThreshold.config(0., 1., 0., 255., 60);

	blob = new SmartBlurBlob();
}

void UIDSmartBlur::on_resize(int width, int height) {
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

void UIDSmartBlur::process_event(OUI* element, uint32_t message, uint64_t param, bool bubbleUp) {
	if (element == &cRadius) {
		radius = (size_t)cRadius.get_value();
		shouldInvalidate = true;
	}
	else if (element == &cThreshold) {
		threshold = (size_t)cThreshold.get_value();
		shouldInvalidate = true;
	}
	else {
		UIDEffect::process_event(element, message, param, bubbleUp);
	}
}

int UIDSmartBlur::render(Sheet* srcImage, Sheet* dstImage, int blockLeft, int blockTop, int blockRight, int blockBottom)
{
	return ImageEffect::smart_blur(srcImage, dstImage, blob, radius, threshold,
		blockLeft, blockTop, blockRight, blockBottom);
}
