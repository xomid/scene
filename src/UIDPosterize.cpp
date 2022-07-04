#include "UIDPosterize.h"
#include "ImageEffect.h"

void UIDPosterize::measure_size(int* width, int* height) {
	if (width) *width = 320;
	if (height) *height = 140;
}

void UIDPosterize::on_init() {
	set_title(L"Posterize");
	cLevels.create(this);
	cLevels.set_text(L"Levels");
	threshold = 0;
	cLevels.config((double)threshold, 1, 0, 255);
}

void UIDPosterize::on_resize(int width, int height) {
	UIDEffect::on_resize(width, height);

	int w, h, l, t, botttomMargin;
	w = contentArea.width,
		h = 50;
	l = 0;
	t = 0;
	botttomMargin = 5;
	cLevels.move(l, t, w, h);
}

void UIDPosterize::process_event(OUI* element, uint32_t message, uint64_t param, bool bubbleUp) {
	if (element == &cLevels) {
		threshold = cLevels.get_value();
		bInvalidate = true;
	}
	else {
		UIDEffect::process_event(element, message, param, bubbleUp);
	}
}

void UIDPosterize::render(Sheet* srcImage, Sheet* dstImage, int blockLeft, int blockTop, int blockRight, int blockBottom)
{
	ImageEffect::posterize(srcImage, dstImage, threshold,
		blockLeft, blockTop, blockRight, blockBottom);
}
