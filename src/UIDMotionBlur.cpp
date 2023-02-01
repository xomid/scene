#include "UIDMotionBlur.h"
#include "ImageEffect.h"

void UIDMotionBlur::measure_size(int* width, int* height) {
	if (width) *width = 320;
	if (height) *height = 180;
}

void UIDMotionBlur::on_init() {
	set_title(L"Motion Blur");
	cSize.create(this);
	numAngle.create(this);
	slAngle.create(this);
	cSize.set_text(L"Levels");
	size = 3;
	cSize.config((double)size, 1, 3, 2000, 100);
	numAngle.set_range(0, 1, -180, 180);
	slAngle.set_range(-180, 180);
	numAngle.border.set(1, backgroundColor.bright(-30));
}

void UIDMotionBlur::on_resize(int width, int height) {
	UIDEffect::on_resize(width, height);

	int w, h, l, t, bottomMargin;
	w = contentArea.width,
		h = 50;
	l = 0;
	t = 0;
	bottomMargin = 5;
	cSize.move(l, t, w, h); t += h + bottomMargin;

	h = 60;
	int W = h;
	int marginL = 5;
	slAngle.move((w - W) / 2, t, W, h);

	W = 70;
	int numH = 24;
	numAngle.move((w - W) / 2 + W + marginL, t + (h - numH) / 2, W, numH);
}

void UIDMotionBlur::process_event(OUI* element, uint32_t message, uint64_t param, bool bubbleUp) {
	if (element == &cSize) {
		size = (size_t)cSize.get_value();
		shouldInvalidate = true;
	}
	else if (element == &slAngle) {
		angle = slAngle.get_value();
		numAngle.set_value(angle);
		shouldInvalidate = true;
	}
	else if (element == &numAngle) {
		angle = atof(numAngle.get_number().c_str());
		slAngle.set_value(angle);
		shouldInvalidate = true;
	}
	else {
		UIDEffect::process_event(element, message, param, bubbleUp);
	}
}

int UIDMotionBlur::render(Sheet* srcImage, Sheet* dstImage, int blockLeft, int blockTop, int blockRight, int blockBottom)
{
	return ImageEffect::motion_blur(srcImage, dstImage, size, angle / 180. * PI, 
		blockLeft, blockTop, blockRight, blockBottom);
}
