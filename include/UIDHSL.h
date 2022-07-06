#pragma once
#include "UIDEffect.h"
#include "UIComplexS.h"
#include <oui_select.h>

class UIDHSL : public UIDEffect
{
	UISelect<UISelectDefaultMenu> selPreset;
	UIComplexS cHue, cSaturation, cLightness;
	UICheck chkColorize;
	bool bColorize;
	int hue, saturation, lightness;
	HSLBlob blob;

public:
	void colorize();
	void load_preset(size_t presetId);
	void measure_size(int* width, int* height) override;
	void on_init() override;
	void on_resize(int width, int height) override;
	void process_event(OUI* element, uint32_t message, uint64_t param, bool bubbleUp) override;

protected:
	int render(Sheet* srcImage, Sheet* dstImage, int blockLeft, int blockTop, int blockRight, int blockBottom) override;
};
