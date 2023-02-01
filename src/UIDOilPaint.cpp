#include "UIDOilPaint.h"
#include "ImageEffect.h"

void UIDOilPaint::measure_size(int* width, int* height) {
	if (width) *width = 320;
	if (height) *height = 170;
}

void UIDOilPaint::on_destroy() {
	if (blob) delete blob;
	blob = NULL;
}

void UIDOilPaint::on_init() {
	set_title(L"Oil Paint");
	cBrushSize.create(this);
	cCoareness.create(this);
	cBrushSize.set_text(L"Brush Size");
	cCoareness.set_text(L"Coareness");
	brushSize = 1;
	coareness = 3;
	cBrushSize.config(1., 1., 1., 100., 60);
	cCoareness.config(3., 1., 3., 255., 60);

	blob = new OldPaintBlob();
}

void UIDOilPaint::on_resize(int width, int height) {
	UIDEffect::on_resize(width, height);

	int w, h, l, t, bottomMargin;
	w = contentArea.width,
		h = 50;
	l = 0;
	t = 0;
	bottomMargin = 5;
	cBrushSize.move(l, t, w, h); t += h + bottomMargin;
	cCoareness.move(l, t, w, h);
}

void UIDOilPaint::process_event(OUI* element, uint32_t message, uint64_t param, bool bubbleUp) {
	if (element == &cBrushSize) {
		brushSize = (size_t)cBrushSize.get_value();
		shouldInvalidate = true;
	}
	else if (element == &cCoareness) {
		coareness = (size_t)cCoareness.get_value();
		shouldInvalidate = true;
	}
	else {
		UIDEffect::process_event(element, message, param, bubbleUp);
	}
}

int UIDOilPaint::render(Sheet* srcImage, Sheet* dstImage, int blockLeft, int blockTop, int blockRight, int blockBottom)
{
	return ImageEffect::oil_paint(srcImage, dstImage, blob, brushSize, coareness,
		blockLeft, blockTop, blockRight, blockBottom);
}
