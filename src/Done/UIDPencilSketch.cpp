#include "UIDPencilSketch.h"
#include "ImageEffect.h"

void UIDPencilSketch::measure_size(int* width, int* height) {
	if (width) *width = 320;
	if (height) *height = 170;
}

void UIDPencilSketch::on_destroy() {
	if (blob) delete blob;
	blob = NULL;
}

void UIDPencilSketch::on_init() {
	set_title(L"Pencil Sketch");
	cBrushSize.create(this);
	cRange.create(this);
	cBrushSize.set_text(L"Brush Size");
	cRange.set_text(L"Range");
	brushSize = 1;
	range = 0;
	cBrushSize.config(1., 1., 1., 50., 60);
	cRange.config(0., 1., -20., 20., 60);

	blob = new PencilSketchBlob();
}

void UIDPencilSketch::on_resize(int width, int height) {
	UIDEffect::on_resize(width, height);

	int w, h, l, t, bottomMargin;
	w = contentArea.width,
		h = 50;
	l = 0;
	t = 0;
	bottomMargin = 5;
	cBrushSize.move(l, t, w, h); t += h + bottomMargin;
	cRange.move(l, t, w, h);
}

void UIDPencilSketch::process_event(OUI* element, uint32_t message, uint64_t param, bool bubbleUp) {
	if (element == &cBrushSize) {
		brushSize = (size_t)cBrushSize.get_value();
		bInvalidate = true;
	}
	else if (element == &cRange) {
		range = (int)cRange.get_value();
		bInvalidate = true;
	}
	else {
		UIDEffect::process_event(element, message, param, bubbleUp);
	}
}

int UIDPencilSketch::render(Sheet* srcImage, Sheet* dstImage, int blockLeft, int blockTop, int blockRight, int blockBottom)
{
	return ImageEffect::pencil_sketch(srcImage, dstImage, blob, brushSize, range,
		blockLeft, blockTop, blockRight, blockBottom);
}
