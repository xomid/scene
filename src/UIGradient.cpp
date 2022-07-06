#include "UIGradient.h"


void UIGradient::on_init() {
	img.create(10, 10, 3);
	border.set(1, backgroundColor.bright(-30));
	borderRadius.set(0);
}

void UIGradient::on_resize(int width, int height) {
	OUI::get_content_area(contentArea);
	img.resize(contentArea.width, contentArea.height);
	img.clear(0xff, 0, 0xff);
}

void UIGradient::set_gradient(GradientFunc gradientFunc, bool vertical, bool reverse) {
	this->gradientFunc = gradientFunc;
	this->vertical = vertical;
	this->reverse = reverse;
	fill_image();
}

void UIGradient::fill_image() {
	if (img.is_useless())return;

	int w, h, x, y, pitch, px;
	pyte data = img.data, d;
	w = img.w;
	h = img.h;
	pitch = img.pitch;
	Color res;
	double t, h1, w1;

	if (gradientFunc) {
		if (vertical) {
			h1 = double(h - 1);
			for (y = 0; y < h; ++y) {
				d = data + y * pitch;
				t = double(y) / h1;
				if (reverse) t = 1. - t;
				gradientFunc(0, t, res);
				px = res.to_int();
				for (x = 0; x < w; ++x, d += 3)
					*(int*)d = px;
			}
		}
		else {
			w1 = double(w - 1);
			for (x = 0; x < w; ++x) {
				d = data + 3 * x;
				t = double(x) / w1;
				if (reverse) t = 1. - t;
				gradientFunc(0, t, res);
				px = res.to_int();
				for (y = 0; y < h; ++y, d += pitch)
					*(int*)d = px;
			}
		}
	}
	else {
		img.clear(backgroundColor.r, backgroundColor.g, backgroundColor.b);
	}
}

void UIGradient::on_update() {
	OUI::on_update();
	canvas.bit_blt(img, contentArea.left, contentArea.top, contentArea.width, contentArea.height, 0, 0, true);
}

void UIGradient::apply_theme(bool bInvalidate) {
	OUI::apply_theme(bInvalidate);
}