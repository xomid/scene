#pragma once
#include "UIDEffect.h"
#include "UIComplexS.h"

class UIDSurfaceBlur : public UIDEffect
{
	UIComplexS cRadius, cLevel;
	int radius, level;

public:
	void measure_size(int* width, int* height) override;
	void on_init() override;
	void on_resize(int width, int height) override;
	void process_event(OUI* element, uint32_t message, uint64_t param, bool bubbleUp) override;

protected:
	int render(Sheet* srcImage, Sheet* dstImage, int blockLeft, int blockTop, int blockRight, int blockBottom) override;
};
