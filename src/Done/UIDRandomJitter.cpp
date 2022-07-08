#include "UIDRandomJitter.h"
#include "ImageEffect.h"

void UIDRandomJitter::measure_size(int* width, int* height) {
	if (width) *width = 320;
	if (height) *height = 115;
}

void UIDRandomJitter::on_init() {
	set_title(L"Random Jitter");
	cAmount.create(this);
	cAmount.set_text(L"Amount");
	amount = 0;
	cAmount.config((double)amount, 1., 0., 200., 60);
}

void UIDRandomJitter::on_resize(int width, int height) {
	UIDEffect::on_resize(width, height);

	int w, h, l, t, bottomMargin;
	w = contentArea.width,
		h = 50;
	l = 0;
	t = 0;
	bottomMargin = 5;
	cAmount.move(l, t, w, h);
}

void UIDRandomJitter::process_event(OUI* element, uint32_t message, uint64_t param, bool bubbleUp) {
	if (element == &cAmount) {
		amount = (size_t)cAmount.get_value();
		bInvalidate = true;
	}
	else {
		UIDEffect::process_event(element, message, param, bubbleUp);
	}
}

int UIDRandomJitter::render(Sheet* srcImage, Sheet* dstImage, int blockLeft, int blockTop, int blockRight, int blockBottom)
{
	return ImageEffect::random_jitter(srcImage, dstImage, amount,
		blockLeft, blockTop, blockRight, blockBottom);
}
