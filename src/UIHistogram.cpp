#include "UIHistogram.h"

void UIHistogram::on_init() {
	img.create(10, 10, 3);
	set_fill_color(OUITheme::text);
}

void UIHistogram::set_fill_color(Color fillColor) {
	this->fillColor = fillColor;
	fill_image();
	invalidate();
}

void UIHistogram::fill_image() {
	if (srcImage == NULL) return;

	int x, y, w, h, h1, p, yLevel, invertMin, invertMax, lookupIndex, lookupValue;
	byte *d, *s, *data, backR, backG, backB, fillR, fillG, fillB;

	data = img.data;
	w = img.w;
	h = img.h;
	p = img.pitch;
	h1 = h - 1;

	invertMin = Min(invertStart, invertEnd);
	invertMax = Max(invertStart, invertEnd);

	for (x = 0; x < w; ++x) {
		lookupIndex = x * 256 / w;
		d = data + x * 3;
		lookupValue = histogramBlob.grayHisto[lookupIndex];
		yLevel = h - (lookupValue * h / 256);

		if (bPressed && x >= invertMin && x <= invertMax) {
			fillR = backgroundColor.r;
			fillG = backgroundColor.g;
			fillB = backgroundColor.b;

			backR = fillColor.r;
			backG = fillColor.g;
			backB = fillColor.b;
		}
		else {
			backR = backgroundColor.r;
			backG = backgroundColor.g;
			backB = backgroundColor.b;

			fillR = fillColor.r;
			fillG = fillColor.g;
			fillB = fillColor.b;
		}

		for (y = 0; y < h; ++y, d += p) {
			if (y < yLevel) {
				d[0] = backB;
				d[1] = backG;
				d[2] = backR;
			}
			else {
				d[0] = fillB;
				d[1] = fillG;
				d[2] = fillR;
			}
		}
	}
}

void UIHistogram::set_image(Sheet* srcImage) {
	this->srcImage = srcImage;
	ImageEffect::get_histo(srcImage, &histogramBlob);
	fill_image();
	invalidate();
}

void UIHistogram::on_resize(int width, int height) {
	img.resize(contentArea.width, contentArea.height);
	fill_image();
}

void UIHistogram::on_update() {
	OUI::on_update();
	if (srcImage) {
		canvas.bit_blt(img, contentArea.left, contentArea.top, contentArea.width, contentArea.height, 0, 0, true);
	}
}

void UIHistogram::on_mouse_move(int x, int y, uint32_t flags) {
	if (bPressed) {
		invertEnd = x - contentArea.left;
		fill_image();
		invalidate();
	}
}

void UIHistogram::on_mouse_down(int x, int y, uint32_t flags) {
	bPressed = true;
	invertStart = x - contentArea.left;
	invalidate();
}

void UIHistogram::on_mouse_up(int x, int y, uint32_t flags) {
	bPressed = false;
	fill_image();
	invalidate();
}

void UIHistogram::on_dbl_click(int x, int y, uint32_t flags) {
	on_mouse_down(x, y, flags);
}
