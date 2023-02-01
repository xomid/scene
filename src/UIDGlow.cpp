#include "UIDGlow.h"
#include "ImageEffect.h"

void UIDGlow::measure_size(int* width, int* height) {
	if (width) *width = 320;
	if (height) *height = 230;
}

void UIDGlow::on_destroy() {
	if (blob) delete blob;
	blob = NULL;
}

void UIDGlow::on_init() {
	set_title(L"Glow");
	cSoftness.create(this);
	cBrightness.create(this);
	cContrast.create(this);
	cSoftness.set_text(L"Softness");
	cBrightness.set_text(L"Brightness");
	cContrast.set_text(L"Contrast");
	softness = 1; 
	brightness = 0; 
	contrast = 0; 
	cSoftness.config(1., 1., 1., 16., 60);
	cBrightness.config(0., 1., -100., 100., 60);
	cContrast.config(0., 1., -100., 100., 60);

	blob = new GlowBlob();
}

void UIDGlow::on_resize(int width, int height) {
	UIDEffect::on_resize(width, height);

	int w, h, l, t, bottomMargin;
	w = contentArea.width,
		h = 50;
	l = 0;
	t = 0;
	bottomMargin = 5;
	cSoftness.move(l, t, w, h); t += h + bottomMargin;
	cBrightness.move(l, t, w, h); t += h + bottomMargin;
	cContrast.move(l, t, w, h);
}

void UIDGlow::process_event(OUI* element, uint32_t message, uint64_t param, bool bubbleUp) {
	if (element == &cSoftness) {
		softness = (int)cSoftness.get_value();
		shouldInvalidate = true;
	}
	else if (element == &cBrightness) {
		brightness = (int)cBrightness.get_value();
		shouldInvalidate = true;
	}
	else if (element == &cContrast) {
		contrast = (int)cContrast.get_value();
		shouldInvalidate = true;
	}
	else {
		UIDEffect::process_event(element, message, param, bubbleUp);
	}
}

int UIDGlow::render(Sheet* srcImage, Sheet* dstImage, int blockLeft, int blockTop, int blockRight, int blockBottom)
{
	return ImageEffect::glow(srcImage, dstImage, blob, softness, brightness, contrast,
		blockLeft, blockTop, blockRight, blockBottom);
}
