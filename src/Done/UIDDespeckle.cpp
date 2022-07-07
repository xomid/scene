#include "UIDDespeckle.h"
#include "ImageEffect.h"

void UIDDespeckle::measure_size(int* width, int* height) {
	if (width) *width = 320;
	if (height) *height = 115;
}

void UIDDespeckle::on_init() {
	set_title(L"Despeckle");
	cIteration.create(this);
	cIteration.set_text(L"Iteration");
	iteration = 0;
	cIteration.config((double)iteration, 1., 0., 20., 60);
}

void UIDDespeckle::on_resize(int width, int height) {
	UIDEffect::on_resize(width, height);

	int w, h, l, t, bottomMargin;
	w = contentArea.width,
		h = 50;
	l = 0;
	t = 0;
	bottomMargin = 5;
	cIteration.move(l, t, w, h);
}

void UIDDespeckle::process_event(OUI* element, uint32_t message, uint64_t param, bool bubbleUp) {
	if (element == &cIteration) {
		iteration = (size_t)cIteration.get_value();
		bInvalidate = true;
	}
	else {
		UIDEffect::process_event(element, message, param, bubbleUp);
	}
}

int UIDDespeckle::render(Sheet* srcImage, Sheet* dstImage, int blockLeft, int blockTop, int blockRight, int blockBottom)
{
	return ImageEffect::despeckle(srcImage, dstImage, iteration,
		blockLeft, blockTop, blockRight, blockBottom);
}
