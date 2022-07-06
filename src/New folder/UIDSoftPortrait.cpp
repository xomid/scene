#include "UIDSoftPortrait.h"
#include "ImageEffect.h"

void UIDSoftPortrait::measure_size(int* width, int* height) {
	if (width) *width = 320;
	if (height) *height = 230;
}

void UIDSoftPortrait::on_init() {
	set_title(L"SoftPortrait");
	cSoftness.create(this);
	cWarmness.create(this);
	cBrightness.create(this);
	cSoftness.set_text(L"Softness");
	cWarmness.set_text(L"Warmness");
	cBrightness.set_text(L"Brightness");
	softness = 0;
	warmness = 0;
	brightness = 0;
	cSoftness.config(0., 1., 0., 10., 60);
	cWarmness.config(0., 1., 0., 40., 60);
	cBrightness.config(0., 1., -100., 100., 60);
}

void UIDSoftPortrait::on_resize(int width, int height) {
	UIDEffect::on_resize(width, height);

	int w, h, l, t, bottomMargin;
	w = contentArea.width,
		h = 50;
	l = 0;
	t = 0;
	bottomMargin = 5;
	cSoftness.move(l, t, w, h); t += h + bottomMargin;
	cWarmness.move(l, t, w, h); t += h + bottomMargin;
	cBrightness.move(l, t, w, h);
}

void UIDSoftPortrait::process_event(OUI* element, uint32_t message, uint64_t param, bool bubbleUp) {
	if (element == &cSoftness) {
		softness = cSoftness.get_value();
		bInvalidate = true;
	}
	else if (element == &cWarmness) {
		warmness = cWarmness.get_value();
		bInvalidate = true;
	}
	else if (element == &cBrightness) {
		brightness = cBrightness.get_value();
		bInvalidate = true;
	}
	else {
		UIDEffect::process_event(element, message, param, bubbleUp);
	}
}

void UIDSoftPortrait::render(Sheet* srcImage, Sheet* dstImage, int blockLeft, int blockTop, int blockRight, int blockBottom)
{
	/*ImageEffect::posterize(srcImage, dstImage, waveLength,
		blockLeft, blockTop, blockRight, blockBottom);*/
}
