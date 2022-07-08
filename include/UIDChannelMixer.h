#pragma once
#include "UIDEffect.h"
#include "UIComplexP.h"
#include <oui_select.h>

class UIDChannelMixer : public UIDEffect
{
	UISelect<UISelectDefaultMenu> selPreset, selOutChannel;
	UIComplexP cRed, cGreen, cBlue;
	UICheck chkMono, chkPreserveLum;
	bool isMonochromatic, shouldPreserveLum;
	ChannelMixInfo gray, red, green, blue;

	ChannelMixInfo* get_channel_info(bool isMonochromatic, size_t index);
	void reset_output_channel_menu();
	void load_preset(size_t index);

public:
	void measure_size(int* width, int* height) override;
	void on_init() override;
	void on_resize(int width, int height) override;
	void process_event(OUI* element, uint32_t message, uint64_t param, bool bubbleUp) override;

protected:
	int render(Sheet* srcImage, Sheet* dstImage, int blockLeft, int blockTop, int blockRight, int blockBottom) override;
};
