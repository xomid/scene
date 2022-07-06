#include "UIDOldPaint.h"
#include "ImageEffect.h"

void UIDOldPaint::measure_size(int* width, int* height) {
	if (width) *width = 320;
	if (height) *height = 170;
}

void UIDOldPaint::on_init() {
	set_title(L"Old Paint");
	cBrushSize.create(this);
	cCoareness.create(this);
	cBrushSize.set_text(L"Brush Size");
	cCoareness.set_text(L"Coareness");
	brushSize = 1;
	coareness = 3;
	cBrushSize.config(1., 1., 1., 100., 60);
	cCoareness.config(3., 1., 3., 255., 60);
}

void UIDOldPaint::on_resize(int width, int height) {
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

void UIDOldPaint::process_event(OUI* element, uint32_t message, uint64_t param, bool bubbleUp) {
	if (element == &cBrushSize) {
		brushSize = (int)cBrushSize.get_value();
		bInvalidate = true;
	}
	else if (element == &cCoareness) {
		coareness = (int)cCoareness.get_value();
		bInvalidate = true;
	}
	else {
		UIDEffect::process_event(element, message, param, bubbleUp);
	}
}

void UIDOldPaint::render(Sheet* srcImage, Sheet* dstImage, int blockLeft, int blockTop, int blockRight, int blockBottom)
{
	/*ImageEffect::posterize(srcImage, dstImage, brushSize,
		blockLeft, blockTop, blockRight, blockBottom);*/
}
