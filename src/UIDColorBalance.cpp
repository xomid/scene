#include "UIDColorBalance.h"
#include "ImageEffect.h"


void gradient_func_cyan_to_red(void* param, double t, Color& res) {
	res.r = CLAMP255(int(t * 255. + .5));
	res.b = res.g = 255 - res.r;
}

void gradient_func_purple_to_green(void* param, double t, Color& res) {
	res.g = CLAMP255(int(t * 255. + .5));
	res.r = res.b = 255 - res.g;
}

void gradient_func_yellow_to_blue(void* param, double t, Color& res) {
	res.b = CLAMP255(int(t * 255. + .5));
	res.r = res.g = 255 - res.b;
}

void UIDColorBalance::measure_size(int* width, int* height) {
	if (width) *width = 320;
	if (height) *height = 260;
}

void UIDColorBalance::on_init() {
	cRed.create(this);
	cGreen.create(this);
	cBlue.create(this);
	chkPreserveLum.create(this);

	cRed.set_text(L"Red");
	cGreen.set_text(L"Green");
	cBlue.set_text(L"Blue");

	red = green = blue = 0;
	cRed.config((double)red, 1, -100, 100, 60);
	cGreen.config((double)green, 1, -100, 100, 60);
	cBlue.config((double)blue, 1, -100, 100, 60);

	cRed.set_gradient(gradient_func_cyan_to_red);
	cGreen.set_gradient(gradient_func_purple_to_green);
	cBlue.set_gradient(gradient_func_yellow_to_blue);

	chkPreserveLum.set_text(L"Preserve Luminosity");
	chkPreserveLum.select(bPreserveLum = true);
}

void UIDColorBalance::on_resize(int width, int height) {
	UIDEffect::on_resize(width, height);

	int w, h, l, t, botttomMargin;
	w = contentArea.width,
		h = 50;
	l = 0;
	t = 0;
	botttomMargin = 5;
	cRed.move(l, t, w, h); t += h + botttomMargin;
	cGreen.move(l, t, w, h); t += h + botttomMargin;
	cBlue.move(l, t, w, h); t += h + botttomMargin + 5;

	h = 30;
	chkPreserveLum.move(l, t, 80, h);
}

void UIDColorBalance::process_event(OUI* element, uint32_t message, uint64_t param, bool bubbleUp) {
	if (element == &cRed) {
		red = (int)cRed.get_value();
		shouldInvalidate = true;
	}
	else if (element == &cGreen) {
		green = (int)cGreen.get_value();
		shouldInvalidate = true;
	}
	else if (element == &cBlue) {
		blue = (int)cBlue.get_value();
		shouldInvalidate = true;
	}
	else if (element == &chkPreserveLum) {
		if (message == Event::Select || message == Event::Deselect) {
			auto prevState = bPreserveLum;
			chkPreserveLum.select(message == Event::Select);
			bPreserveLum = chkPreserveLum.isSelected;
			shouldInvalidate = bPreserveLum != prevState;
		}
	}
	else {
		UIDEffect::process_event(element, message, param, bubbleUp);
	}
}

int UIDColorBalance::render(Sheet* srcImage, Sheet* dstImage, int blockLeft, int blockTop, int blockRight, int blockBottom)
{
	return ImageEffect::color_balance(srcImage, dstImage, bPreserveLum, red, green, blue,
		blockLeft, blockTop, blockRight, blockBottom);
}
