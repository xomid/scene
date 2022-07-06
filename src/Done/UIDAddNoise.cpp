#include "UIDAddNoise.h"
#include "ImageEffect.h"

void UIDAddNoise::measure_size(int* width, int* height) {
	if (width) *width = 320;
	if (height) *height = 160;
}

void UIDAddNoise::on_init() {
	set_title(L"Add Noise");

	cAmount.create(this);
	grpMode.create(this);
	chkMono.create(this);

	int w = 80, h = 30;
	rdGussian.create(0, 0, w, h, &grpMode);
	rdUniform.create(w, 0, w, h, &grpMode);

	cAmount.set_text(L"Amount");
	rdGussian.set_text(L"Gussian");
	rdUniform.set_text(L"Uniform");
	chkMono.set_text(L"Monochromatic");

	noiseType = NoiseType::Gussian;
	bMono = true;
	amount = 0.01;

	cAmount.config(".01", ".1", ".01", "400.", 80);
	chkMono.select(bMono);
	rdGussian.select(noiseType == NoiseType::Gussian);
	rdUniform.select(noiseType == NoiseType::Uniform);
}

void UIDAddNoise::on_resize(int width, int height) {
	UIDEffect::on_resize(width, height);

	int w, h, l, t, bottomMargin, marginL = 5;
	w = contentArea.width,
		h = 50;
	l = 0;
	t = 0;
	bottomMargin = 5;
	cAmount.move(l, t, w, h);  t += h + bottomMargin + 5;

	h = 30;
	int grpW = 160;
	grpMode.move(l, t, grpW, h);
	chkMono.move(l + grpW + marginL, t, 100, h);
}

void UIDAddNoise::process_event(OUI* element, uint32_t message, uint64_t param, bool bubbleUp) {
	if (element == &cAmount) {
		amount = cAmount.get_value();
		bInvalidate = true;
	}
	else if (element == &rdGussian || element == &rdUniform) {
		if (message == Event::Select) {
			element->select(true);
			if (rdGussian.bSelected) noiseType = NoiseType::Gussian;
			else noiseType = NoiseType::Gussian;
			bInvalidate = true;
		}
	}
	else if (element == &chkMono) {
		chkMono.select(message == Event::Select);
		bMono = chkMono.bSelected;
		bInvalidate = true;
	}
	else {
		UIDEffect::process_event(element, message, param, bubbleUp);
	}
}

void UIDAddNoise::render(Sheet* srcImage, Sheet* dstImage, int blockLeft, int blockTop, int blockRight, int blockBottom)
{
	ImageEffect::add_noise(srcImage, dstImage, noiseType, amount, bMono,
		blockLeft, blockTop, blockRight, blockBottom);
}
