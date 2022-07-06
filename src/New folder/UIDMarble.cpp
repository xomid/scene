#include "UIDMarble.h"
#include "ImageEffect.h"

void UIDMarble::measure_size(int* width, int* height) {
	if (width) *width = 320;
	if (height) *height = 170;
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
		telorance = cTelorance.get_value();
		bInvalidate = true;
	}
	else if (element == &cScale) {
		scale = cScale.get_value();
		bInvalidate = true;
	}
	else {
		UIDEffect::process_event(element, message, param, bubbleUp);
	}
}

void UIDMarble::render(Sheet* srcImage, Sheet* dstImage, int blockLeft, int blockTop, int blockRight, int blockBottom)
{
	/*ImageEffect::posterize(srcImage, dstImage, telorance,
		blockLeft, blockTop, blockRight, blockBottom);*/
}
