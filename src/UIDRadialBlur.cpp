#include "UIDRadialBlur.h"
#include "ImageEffect.h"

void UIDRadialBlur::measure_size(int* width, int* height) {
	if (width) *width = 320;
	if (height) *height = 260;
}

void UIDRadialBlur::on_init() {
	set_title(L"Radial Blur");

	cBlur.create(this);
	label.create(this);
	grpMode.create(this);
	rdSpin.create(0, 0, 100, 30, &grpMode);
	rdZoom.create(0, 35, 100, 30, &grpMode);
	cAmount.create(this);

	label.set_text(L"Blur Mode");
	rdSpin.set_text(L"Spin");
	rdZoom.set_text(L"Zoom");
	cAmount.set_text(L"Amount");
	label.canvas.art.alignX = Align::LEFT;

	amount = 1;
	cx = .5;
	cy = .5;
	cAmount.config(amount, 1., 1., 100., 80);
	cBlur.set_amount((int)amount);
	cBlur.set_blur_mode(RadialBlurMode::Spin);
	rdSpin.select(true);
}

void UIDRadialBlur::on_resize(int width, int height) {
	UIDEffect::on_resize(width, height);

	int w, h, l, t, bottomMargin;
	w = contentArea.width,
		h = 50;
	l = 0;
	t = 0;
	bottomMargin = 5;

	int blurW;
	blurW = 130;
	h = blurW;
	cBlur.move(w - blurW, t, blurW, h); t += h + bottomMargin + 10;
	h = 50;
	cAmount.move(l, t, w, h);

	t = 40;
	h = 30;
	label.move(l, t, w - blurW, h); t += h + bottomMargin;
	h = 70;
	grpMode.move(l, t, w - blurW, h);
}

void UIDRadialBlur::process_event(OUI* element, uint32_t message, uint64_t param, bool bubbleUp) {
	if (element == &cAmount) {
		amount = cAmount.get_value();
		cBlur.set_amount(amount);
		shouldInvalidate = true;
	}
	else if (element == &rdSpin || element == &rdZoom) {
		if (message == Event::Select) {
			element->select(true);
			blurMode = RadialBlurMode::Spin;
			if (rdZoom.bSelected)
				blurMode = RadialBlurMode::Zoom;
			cBlur.set_blur_mode(blurMode);
			shouldInvalidate = true;
		}
	}
	else if (element == &cBlur) {
		cBlur.get_pivot_point(cx, cy);
		shouldInvalidate = true;
	}
	else {
		UIDEffect::process_event(element, message, param, bubbleUp);
	}
}

int UIDRadialBlur::render(Sheet* srcImage, Sheet* dstImage, int blockLeft, int blockTop, int blockRight, int blockBottom)
{
	return ImageEffect::radial_blur(srcImage, dstImage, blurMode, amount, cx, cy,
		blockLeft, blockTop, blockRight, blockBottom);
}
