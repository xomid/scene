#include "UIDRipple.h"
#include "ImageEffect.h"

void UIDRipple::measure_size(int* width, int* height) {
	if (width) *width = 320;
	if (height) *height = 210;
}

void UIDRipple::on_init() {
	set_title(L"Ripple");
	cWaveLength.create(this);
	cAmplitude.create(this);
	grpMode.create(this);
	rdSinuous.create(0, 0, 80, 20, &grpMode);
	rdTriangle.create(85, 0, 80, 20, &grpMode);

	cWaveLength.set_text(L"Wave Length");
	cAmplitude.set_text(L"Amplitude");
	rdSinuous.set_text(L"Sinuous");
	rdTriangle.set_text(L"Triangle");

	waveLength = 2;
	amplitude = 1;
	cWaveLength.config(1., 1., 1., 200., 60);
	cAmplitude.config(1., 1., 1., 100., 60);

	rdSinuous.select(true);
	rippleMode = RippleMode::Sinuous;
}

void UIDRipple::on_resize(int width, int height) {
	UIDEffect::on_resize(width, height);

	int w, h, l, t, bottomMargin;
	w = contentArea.width,
		h = 50;
	l = 0;
	t = 0;
	bottomMargin = 5;
	cWaveLength.move(l, t, w, h); t += h + bottomMargin;
	cAmplitude.move(l, t, w, h); t += h + bottomMargin + 5;
	grpMode.move(l, t, w, h);
}

void UIDRipple::process_event(OUI* element, uint32_t message, uint64_t param, bool bubbleUp) {
	if (element == &cWaveLength) {
		waveLength = (int)cWaveLength.get_value();
		bInvalidate = true;
	}
	else if (element == &cAmplitude) {
		amplitude = (int)cAmplitude.get_value();
		bInvalidate = true;
	}
	else if (element == &rdSinuous || element == &rdTriangle) {
		if (message == Event::Select)
			element->select(true);

		if (rdSinuous.bSelected)
			rippleMode = RippleMode::Sinuous;
		else
			rippleMode = RippleMode::Triangle;

		bInvalidate = true;
	}
	else {
		UIDEffect::process_event(element, message, param, bubbleUp);
	}
}

int UIDRipple::render(Sheet* srcImage, Sheet* dstImage, int blockLeft, int blockTop, int blockRight, int blockBottom)
{
	return ImageEffect::ripple(srcImage, dstImage, rippleMode, waveLength, amplitude,
		blockLeft, blockTop, blockRight, blockBottom);
}
