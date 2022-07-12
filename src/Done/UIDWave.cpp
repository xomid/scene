#include "UIDWave.h"
#include "ImageEffect.h"

void UIDWave::measure_size(int* width, int* height) {
	if (width) *width = 320;
	if (height) *height = 170;
}

void UIDWave::on_init() {
	set_title(L"Wave");
	cWaveLength.create(this);
	cAmplitude.create(this);
	cWaveLength.set_text(L"Wave Length");
	cAmplitude.set_text(L"Amplitude");
	waveLength = 2;
	amplitude = 1;
	cWaveLength.config(2., 1., 2., 200., 60);
	cAmplitude.config(1., 1., 2.,50., 60);
}

void UIDWave::on_resize(int width, int height) {
	UIDEffect::on_resize(width, height);

	int w, h, l, t, bottomMargin;
	w = contentArea.width,
		h = 50;
	l = 0;
	t = 0;
	bottomMargin = 5;
	cWaveLength.move(l, t, w, h); t += h + bottomMargin;
	cAmplitude.move(l, t, w, h);
}

void UIDWave::process_event(OUI* element, uint32_t message, uint64_t param, bool bubbleUp) {
	if (element == &cWaveLength) {
		waveLength = cWaveLength.get_value();
		shouldInvalidate = true;
	}
	else if (element == &cAmplitude) {
		amplitude = cAmplitude.get_value();
		shouldInvalidate = true;
	}
	else {
		UIDEffect::process_event(element, message, param, bubbleUp);
	}
}

int UIDWave::render(Sheet* srcImage, Sheet* dstImage, int blockLeft, int blockTop, int blockRight, int blockBottom)
{
	return ImageEffect::wave(srcImage, dstImage, waveLength, amplitude, .5, .5,
		blockLeft, blockTop, blockRight, blockBottom);
}
