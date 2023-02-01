#include "UIDGaussianBlur.h"
#include "ImageEffect.h"

void UIDGaussianBlur::measure_size(int* width, int* height) {
	if (width) *width = 320;
	if (height) *height = 115;
}

void UIDGaussianBlur::on_init() {
	set_title(L"Gaussian Blur");
	cRadius.create(this);
	cRadius.set_text(L"Radius");
	radius = 0;
	cRadius.config((double)radius, 1., 0., 254., 60);
}

void UIDGaussianBlur::on_resize(int width, int height) {
	UIDEffect::on_resize(width, height);

	int w, h, l, t, bottomMargin;
	w = contentArea.width,
	h = 50;
	l = 0;
	t = 0;
	bottomMargin = 5;
	cRadius.move(l, t, w, h);
}

void UIDGaussianBlur::process_event(OUI* element, uint32_t message, uint64_t param, bool bubbleUp) {
	if (element == &cRadius) {
		radius = (size_t)cRadius.get_value();
		shouldInvalidate = true;
	}
	else {
		UIDEffect::process_event(element, message, param, bubbleUp);
	}
}

int UIDGaussianBlur::render(Sheet* srcImage, Sheet* dstImage, int blockLeft, int blockTop, int blockRight, int blockBottom)
{
	return ImageEffect::gaussian_blur(srcImage, dstImage, radius,
		blockLeft, blockTop, blockRight, blockBottom);
}
