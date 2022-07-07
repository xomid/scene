#include "UIDMedian.h"
#include "ImageEffect.h"

void UIDMedian::measure_size(int* width, int* height) {
	if (width) *width = 320;
	if (height) *height = 170;
}

void UIDMedian::on_destroy() {
	if (blob) delete blob;
	blob = NULL;
}

void UIDMedian::on_init() {
	set_title(L"Median");
	cRadius.create(this);
	cPercentile.create(this);
	cRadius.set_text(L"Radius");
	cPercentile.set_text(L"Percentile");
	radius = 0;
	percentile = 0;
	cRadius.config(radius, 1., 1, 200., 60);
	cPercentile.config(percentile, 1., 0, 100., 60);

	blob = new MedianBlob();
}

void UIDMedian::on_resize(int width, int height) {
	UIDEffect::on_resize(width, height);

	int w, h, l, t, bottomMargin;
	w = contentArea.width,
		h = 50;
	l = 0;
	t = 0;
	bottomMargin = 5;
	cRadius.move(l, t, w, h); t += h + bottomMargin;
	cPercentile.move(l, t, w, h);
}

void UIDMedian::process_event(OUI* element, uint32_t message, uint64_t param, bool bubbleUp) {
	if (element == &cRadius) {
		radius = (size_t)cRadius.get_value();
		bInvalidate = true;
	}
	else if (element == &cPercentile) {
		percentile = (size_t)cPercentile.get_value();
		bInvalidate = true;
	}
	else {
		UIDEffect::process_event(element, message, param, bubbleUp);
	}
}

int UIDMedian::render(Sheet* srcImage, Sheet* dstImage, int blockLeft, int blockTop, int blockRight, int blockBottom)
{
	return ImageEffect::median(srcImage, dstImage, blob, radius, percentile,
		blockLeft, blockTop, blockRight, blockBottom);
}
