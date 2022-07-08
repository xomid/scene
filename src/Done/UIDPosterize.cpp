#include "UIDPosterize.h"
#include "ImageEffect.h"

void UIDPosterize::measure_size(int* width, int* height) {
	if (width) *width = 320;
	if (height) *height = 110;
}

void UIDPosterize::on_destroy() {
	if (blob) delete blob;
	blob = NULL;
}

void UIDPosterize::on_init() {
	set_title(L"Posterize");
	cThreshold.create(this);
	cThreshold.set_text(L"Threshold");
	threshold = 0;
	cThreshold.config((double)threshold, 1, 2, 255, 60);
	blob = new PosterizeBlob();
}

void UIDPosterize::on_resize(int width, int height) {
	UIDEffect::on_resize(width, height);

	int w, h, l, t, botttomMargin;
	w = contentArea.width,
		h = 50;
	l = 0;
	t = 0;
	botttomMargin = 5;
	cThreshold.move(l, t, w, h);
}

void UIDPosterize::process_event(OUI* element, uint32_t message, uint64_t param, bool bubbleUp) {
	if (element == &cThreshold) {
		threshold = (byte)cThreshold.get_value();
		bInvalidate = true;
	}
	else {
		UIDEffect::process_event(element, message, param, bubbleUp);
	}
}

int UIDPosterize::render(Sheet* srcImage, Sheet* dstImage, int blockLeft, int blockTop, int blockRight, int blockBottom)
{
	return ImageEffect::posterize(srcImage, dstImage, blob, threshold,
		blockLeft, blockTop, blockRight, blockBottom);
}
