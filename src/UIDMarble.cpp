#include "UIDMarble.h"
#include "ImageEffect.h"

void UIDMarble::measure_size(int* width, int* height) {
	if (width) *width = 320;
	if (height) *height = 170;
}

void UIDMarble::on_destroy() {
	if (blob) delete blob;
	blob = NULL;
}

void UIDMarble::on_init() {
	set_title(L"Marble");
	cTelorance.create(this);
	cScale.create(this);
	cTelorance.set_text(L"Telorance");
	cScale.set_text(L"Scale");
	telorance = 0;
	scale = 0;
	cTelorance.config((double)telorance, 1., 0., 100., 60);
	cScale.config((double)scale, 1., 0., 100., 60);

	blob = new MarbleBlob();
}

void UIDMarble::on_resize(int width, int height) {
	UIDEffect::on_resize(width, height);

	int w, h, l, t, bottomMargin;
	w = contentArea.width,
		h = 50;
	l = 0;
	t = 0;
	bottomMargin = 5;
	cTelorance.move(l, t, w, h); t += h + bottomMargin;
	cScale.move(l, t, w, h);
}

void UIDMarble::process_event(OUI* element, uint32_t message, uint64_t param, bool bubbleUp) {
	if (element == &cTelorance) {
		telorance = (int)cTelorance.get_value();
		shouldInvalidate = true;
	}
	else if (element == &cScale) {
		scale = (int)cScale.get_value();
		shouldInvalidate = true;
	}
	else {
		UIDEffect::process_event(element, message, param, bubbleUp);
	}
}

int UIDMarble::render(Sheet* srcImage, Sheet* dstImage, int blockLeft, int blockTop, int blockRight, int blockBottom)
{
	return ImageEffect::marble(srcImage, dstImage, blob, telorance, scale,
		blockLeft, blockTop, blockRight, blockBottom);
}
