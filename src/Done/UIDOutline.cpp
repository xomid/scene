#include "UIDOutline.h"
#include "ImageEffect.h"

void UIDOutline::measure_size(int* width, int* height) {
	if (width) *width = 320;
	if (height) *height = 170;
}

void UIDOutline::on_destroy() {
	if (blob) delete blob;
	blob = NULL;
}

void UIDOutline::on_init() {
	set_title(L"Outline");
	cRadius.create(this);
	cIntensity.create(this);
	cRadius.set_text(L"Radius");
	cIntensity.set_text(L"Intensity");
	radius = 1;
	intensity = 0;
	cRadius.config(1., 1., 1., 200., 60);
	cIntensity.config(0., 1., 0., 100., 60);

	blob = new OutlineBlob();
}

void UIDOutline::on_resize(int width, int height) {
	UIDEffect::on_resize(width, height);

	int w, h, l, t, bottomMargin;
	w = contentArea.width,
		h = 50;
	l = 0;
	t = 0;
	bottomMargin = 5;
	cRadius.move(l, t, w, h); t += h + bottomMargin;
	cIntensity.move(l, t, w, h);
}

void UIDOutline::process_event(OUI* element, uint32_t message, uint64_t param, bool bubbleUp) {
	if (element == &cRadius) {
		radius = (int)cRadius.get_value();
		bInvalidate = true;
	}
	else if (element == &cIntensity) {
		intensity = (int)cIntensity.get_value();
		bInvalidate = true;
	}
	else {
		UIDEffect::process_event(element, message, param, bubbleUp);
	}
}

int UIDOutline::render(Sheet* srcImage, Sheet* dstImage, int blockLeft, int blockTop, int blockRight, int blockBottom)
{
	return ImageEffect::outline(srcImage, dstImage, blob, radius, intensity,
		blockLeft, blockTop, blockRight, blockBottom);
}
