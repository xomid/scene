#pragma once
#include "UIDEffect.h"
#include "UIComplexS.h"
#include "UIHistogram.h"
#include <oui_select.h>
#include <UIDoubleSlide.h>
#include <UIRangeSlide.h>

class UIDLevels : public UIDEffect
{
	UISelect<UISelectDefaultMenu> selPreset, selChannel;
	UIHistogram cHisto;
	UINumber numLow, numMid, numHigh, numMin, numMax;
	UIRangeSlide slRange;
	UIDoubleSlide slOutput;
	UILabel lblOutput;
	LevelsBlob* blob;
	ChannelLevelInfo redInfo, greenInfo, blueInfo, rgbInfo, *currentChannelInfo;
	bool isNew;

	ChannelLevelInfo* get_channel_info(size_t index);
	void load_preset(size_t index);

public:
	void measure_size(int* width, int* height) override;
	void on_init() override;
	void on_destroy() override;
	void on_resize(int width, int height) override;
	void process_event(OUI* element, uint32_t message, uint64_t param, bool bubbleUp) override;
	void reset_image() override;

protected:
	int render(Sheet* srcImage, Sheet* dstImage, int blockLeft, int blockTop, int blockRight, int blockBottom) override;
};
