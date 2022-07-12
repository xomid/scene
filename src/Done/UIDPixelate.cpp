#include "UIDPixelate.h"
#include "ImageEffect.h"

void UIDPixelate::measure_size(int* width, int* height) {
	if (width) *width = 320;
	if (height) *height = 115;
}

void UIDPixelate::on_init() {
	set_title(L"Pixelate");
	cCellSize.create(this);
	cCellSize.set_text(L"Cell Size");
	cellSize = 0;
	cCellSize.config((double)cellSize, 1., 0., 200., 60);
}

void UIDPixelate::on_resize(int width, int height) {
	UIDEffect::on_resize(width, height);

	int w, h, l, t, bottomMargin;
	w = contentArea.width,
		h = 50;
	l = 0;
	t = 0;
	bottomMargin = 5;
	cCellSize.move(l, t, w, h);
}

void UIDPixelate::process_event(OUI* element, uint32_t message, uint64_t param, bool bubbleUp) {
	if (element == &cCellSize) {
		cellSize = cCellSize.get_value();
		shouldInvalidate = true;
	}
	else {
		UIDEffect::process_event(element, message, param, bubbleUp);
	}
}

int UIDPixelate::render(Sheet* srcImage, Sheet* dstImage, int blockLeft, int blockTop, int blockRight, int blockBottom)
{
	return ImageEffect::pixelate(srcImage, dstImage, cellSize,
		blockLeft, blockTop, blockRight, blockBottom);
}
