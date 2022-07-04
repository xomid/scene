#pragma once
#include "UIDEffect.h"
#include "UIComplexS.h"
#include "UICurve.h"
#include "UIGradient.h"
#include <oui_group.h>
#include <oui_radio.h>
#include <oui_select.h>

class UIDCurves : public UIDEffect
{
	UISelect<UISelectDefaultMenu> selPreset, selChannel;
	UIGroup grpModes;
	UIRadio rdCurve, rdPoint;
	UICurve cCurve;
	UIGradient gradLeft, gradBottom;
	CurvesBlob blob;
	bool bCurve;

public:
	void calc_lookup_tables();
	void load_preset(size_t presetId);
	void reset_image() override;
	void measure_size(int* width, int* height) override;
	void on_init() override;
	void on_resize(int width, int height) override;
	void process_event(OUI* element, uint32_t message, uint64_t param, bool bubbleUp) override;

protected:
	void render(Sheet* srcImage, Sheet* dstImage, int blockLeft, int blockTop, int blockRight, int blockBottom) override;
};
