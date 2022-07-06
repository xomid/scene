#pragma once
#include "UIDEffect.h"
#include "UIComplexP.h"
#include <oui_select.h>

class UIDChannelMixer : public UIDEffect
{
	UISelect<UISelectDefaultMenu> selPreset, selOutChannel;
	UIComplexP cRed, cGreen, cBlue;
	UICheck chkMono, chkPreserveLum;
	bool bMono, bPreserveLum;
	int red, green, blue;
	ChannelMixerBlob blob;

	void reset_output_channel_menu();
public:
	void measure_size(int* width, int* height) override;
	void on_init() override;
	void on_resize(int width, int height) override;
	void process_event(OUI* element, uint32_t message, uint64_t param, bool bubbleUp) override;

protected:
	int render(Sheet* srcImage, Sheet* dstImage, int blockLeft, int blockTop, int blockRight, int blockBottom) override;
};
