#include "UIDSurfaceBlur.h"
#include "ImageEffect.h"

void UIDSurfaceBlur::measure_size(int* width, int* height) {
	if (width) *width = 320;
	if (height) *height = 170;
}

void UIDSurfaceBlur::on_init() {
	set_title(L"Surface Blur");
	cRadius.create(this);
	cLevel.create(this);
	cRadius.set_text(L"Radius");
	cLevel.set_text(L"Level");
	radius = 1;
	level = 2;
	cRadius.config(1., 1., 1., 100., 60);
	cLevel.config(2., 1., 2., 255., 60);
}

void UIDSurfaceBlur::on_resize(int width, int height) {
	UIDEffect::on_resize(width, height);

	int w, h, l, t, bottomMargin;
	w = contentArea.width,
		h = 50;
	l = 0;
	t = 0;
	bottomMargin = 5;
	cRadius.move(l, t, w, h); t += h + bottomMargin;
	cLevel.move(l, t, w, h);
}

void UIDSurfaceBlur::process_event(OUI* element, uint32_t message, uint64_t param, bool bubbleUp) {
	if (element == &cRadius) {
		radius = (int)cRadius.get_value();
		bInvalidate = true;
	}
	else if (element == &cLevel) {
		level = (int)cLevel.get_value();
		bInvalidate = true;
	}
	else {
		UIDEffect::process_event(element, message, param, bubbleUp);
	}
}

void UIDSurfaceBlur::render(Sheet* srcImage, Sheet* dstImage, int blockLeft, int blockTop, int blockRight, int blockBottom)
{
	/*ImageEffect::posterize(srcImage, dstImage, radius,
		blockLeft, blockTop, blockRight, blockBottom);*/
}
