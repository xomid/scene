#include "UIDTileGlass.h"
#include "ImageEffect.h"

void UIDTileGlass::measure_size(int* width, int* height) {
	if (width) *width = 320;
	if (height) *height = 240;
}

void UIDTileGlass::on_init() {
	set_title(L"Tile Glass");
	cSize.create(this);
	cAmount.create(this);
	numAngle.create(this);
	slAngle.create(this);
	cSize.set_text(L"Size");
	cAmount.set_text(L"Amount");
	size = 2;
	amount = 0;
	cSize.config((double)size, 1, 2, 1000, 100);
	cAmount.config((double)amount, 1, -20, 20, 100);
	numAngle.set_range(0, 1, -180, 180);
	slAngle.set_range(-180, 180);
	numAngle.border.set(1, backgroundColor.bright(-30));
}

void UIDTileGlass::on_resize(int width, int height) {
	UIDEffect::on_resize(width, height);

	int w, h, l, t, bottomMargin;
	w = contentArea.width,
		h = 50;
	l = 0;
	t = 0;
	bottomMargin = 5;
	cSize.move(l, t, w, h); t += h + bottomMargin;
	cAmount.move(l, t, w, h); t += h + bottomMargin + 5;

	h = 60;
	int W = h;
	int marginL = 5;
	slAngle.move((w - W) / 2, t, W, h);

	W = 70;
	int numH = 24;
	numAngle.move((w - W) / 2 + W + marginL, t + (h - numH) / 2, W, numH);
}

void UIDTileGlass::process_event(OUI* element, uint32_t message, uint64_t param, bool bubbleUp) {
	if (element == &cSize) {
		size = (int)cSize.get_value();
		bInvalidate = true;
	}
	else if (element == &cAmount) {
		amount = (int)cAmount.get_value();
		bInvalidate = true;
	}
	else if (element == &slAngle) {
		angle = (int)slAngle.get_value();
		numAngle.set_value((double)angle);
		bInvalidate = true;
	}
	else if (element == &numAngle) {
		angle = (int)atof(numAngle.get_number().c_str());
		slAngle.set_value((double)angle);
		bInvalidate = true;
	}
	else {
		UIDEffect::process_event(element, message, param, bubbleUp);
	}
}

int UIDTileGlass::render(Sheet* srcImage, Sheet* dstImage, int blockLeft, int blockTop, int blockRight, int blockBottom)
{
	return ImageEffect::tile_glass(srcImage, dstImage, size, amount, angle / 180. * PI, .5, .5,
		blockLeft, blockTop, blockRight, blockBottom);
}
