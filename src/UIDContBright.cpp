#include "UIDContBright.h"
#include "ImageEffect.h"

void UIDContBright::measure_size(int* width, int* height) {
	if (width) *width = 360;
	if (height) *height = 190;
}

void UIDContBright::on_init() {
	scBright.create(this);
	scContst.create(this);
	chkLegacy.create(this);

	scBright.set_text(L"Brightness");
	brightness = 0;
	scBright.config((int)brightness, 1, -150, 150);

	scContst.set_text(L"Contrast");
	contrast = 0;
	scContst.config((int)contrast, 1, -100, 100);

	chkLegacy.set_text(L"Legacy");
	chkLegacy.select(bLegacy = true);
}

void UIDContBright::on_resize(int width, int height) {
	UIDEffect::on_resize(width, height);

	int w, h, l, t, botttomMargin;
	w = contentArea.width,
		h = 50;
	l = 0;
	t = 0;
	botttomMargin = 5;
	scBright.move(l, t, w, h); t += h + botttomMargin;
	scContst.move(l, t, w, h); t += h + botttomMargin + 5;

	h = 30;
	chkLegacy.move(l, t, 80, h);
}

void UIDContBright::process_event(OUI* element, uint32_t message, uint64_t param, bool bubbleUp) {
	if (element == &scBright) {
		brightness = scBright.get_value();
		bInvalidate = true;
	}
	else if (element == &scContst) {
		contrast = scContst.get_value();
		bInvalidate = true;
	}
	else if (element == &chkLegacy) {
		if (message == Event::Select || message == Event::Deselect) {
			auto prevState = bLegacy;
			chkLegacy.select(message == Event::Select);
			bLegacy = chkLegacy.bSelected;
			if (bLegacy) {
				scBright.config(scBright.get_value(), 1, -100, 100);
				scContst.config(scContst.get_value(), 1, -100, 100);
			}
			else {
				scBright.config(scBright.get_value(), 1, -150, 150);
				scContst.config(scContst.get_value(), 1, -50, 100);
			}

			bInvalidate = bLegacy != prevState;
		}
	}
	else {
		UIDEffect::process_event(element, message, param, bubbleUp);
	}
}

void UIDContBright::render(Sheet* srcImage, Sheet* dstImage, int blockLeft, int blockTop, int blockRight, int blockBottom) 
{
	ImageEffect::brightness_contrast(srcImage, dstImage, &blob, bLegacy, brightness, contrast,
		blockLeft, blockTop, blockRight, blockBottom);
}
