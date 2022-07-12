#include "UIDThreshold.h"

void UIDThreshold::measure_size(int* width, int* height) {
	if (width) *width = 320;
	if (height) *height = 290;
}

void UIDThreshold::reset_image() {
	cHisto.set_image(document ? document->get_image() : NULL);
}

void UIDThreshold::on_destroy() {
	if (blob) delete blob;
	blob = NULL;
}

void UIDThreshold::on_init() {
	set_title(L"Threshold");

	cHisto.create(this);
	cLevels.create(this);
	chkMono.create(this);
	threshold = 128;
	cLevels.config((double)threshold, 1, 0, 255, 60);
	chkMono.set_text(L"Monochrome");
	chkMono.select(bMono = true);

	blob = new ThresholdBlob();
}

void UIDThreshold::on_resize(int width, int height) {
	UIDEffect::on_resize(width, height);

	int w, h, l, t, botttomMargin;
	w = contentArea.width,
		h = 30;
	l = 0;
	t = 0;
	botttomMargin = 5;

	h = 130;
	cHisto.move(l, t, w, h); t += h + botttomMargin + 5;

	h = 50;
	cLevels.move(l, t, w, h); t += h + botttomMargin + 5;

	h = 30;
	chkMono.move(l, t, 80, h);
}

void UIDThreshold::process_event(OUI* element, uint32_t message, uint64_t param, bool bubbleUp) {
	if (element == &cLevels) {
		threshold = cLevels.get_value();
		shouldInvalidate = true;
	}
	else if (element == &chkMono) {
		if (message == Event::Select || message == Event::Deselect) {
			auto prevState = bMono;
			chkMono.select(message == Event::Select);
			bMono = chkMono.bSelected;
			shouldInvalidate = bMono != prevState;
		}
	}
	else {
		UIDEffect::process_event(element, message, param, bubbleUp);
	}
}

int UIDThreshold::render(Sheet* srcImage, Sheet* dstImage, int blockLeft, int blockTop, int blockRight, int blockBottom)
{
	return ImageEffect::threshold(srcImage, dstImage, blob, bMono, threshold, blockLeft, blockTop, blockRight, blockBottom);
}
