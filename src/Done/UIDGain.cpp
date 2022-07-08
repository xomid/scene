#include "UIDGain.h"
#include "ImageEffect.h"

void UIDGain::measure_size(int* width, int* height) {
	if (width) *width = 320;
	if (height) *height = 170;
}

void UIDGain::on_destroy() {
	if (blob) delete blob;
	blob = NULL;
}

void UIDGain::on_init() {
	set_title(L"Gain");
	cGain.create(this);
	cBias.create(this);
	cGain.set_text(L"Gain");
	cBias.set_text(L"Bias");
	gain = .5;
	bias = .5;
	cGain.config("0.5", ".001", "0", "1.000", 80);
	cBias.config("0.5", ".001", "0", "1.000", 80);

	blob = new GainBlob();
}

void UIDGain::on_resize(int width, int height) {
	UIDEffect::on_resize(width, height);

	int w, h, l, t, bottomMargin;
	w = contentArea.width,
		h = 50;
	l = 0;
	t = 0;
	bottomMargin = 5;
	cGain.move(l, t, w, h); t += h + bottomMargin;
	cBias.move(l, t, w, h);
}

void UIDGain::process_event(OUI* element, uint32_t message, uint64_t param, bool bubbleUp) {
	if (element == &cGain) {
		gain = cGain.get_value();
		bInvalidate = true;
	}
	else if (element == &cBias) {
		bias = cBias.get_value();
		bInvalidate = true;
	}
	else {
		UIDEffect::process_event(element, message, param, bubbleUp);
	}
}

int UIDGain::render(Sheet* srcImage, Sheet* dstImage, int blockLeft, int blockTop, int blockRight, int blockBottom)
{
	return ImageEffect::gain(srcImage, dstImage, blob, gain, bias,
		blockLeft, blockTop, blockRight, blockBottom);
}
