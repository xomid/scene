#pragma once
#include "UIDEffect.h"
#include "UIComplexS.h"
#include "UIRadialBlur.h"
#include <oui_radio.h>
#include <oui_group.h>

class UIDRadialBlur : public UIDEffect
{
	UIRadialBlur cBlur;
	UILabel label;
	UIGroup grpMode;
	UIRadio rdSpin, rdZoom;
	UIComplexS cAmount;
	double amount, cx, cy;
	RadialBlurMode blurMode;

public:
	void measure_size(int* width, int* height) override;
	void on_init() override;
	void on_resize(int width, int height) override;
	void process_event(OUI* element, uint32_t message, uint64_t param, bool bubbleUp) override;

protected:
	int render(Sheet* srcImage, Sheet* dstImage, int blockLeft, int blockTop, int blockRight, int blockBottom) override;
};
