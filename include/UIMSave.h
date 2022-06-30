#pragma once
#include "UIComMenu.h"
#include "UIButtonWithBorderOnHover.h"

#define UISAVE_UPDATE (Event::_last + 2)

class UIMSave : public UIComMenu
{
	UIButtonWithBorderOnHover btnSave, btnSaveAs;
public:
	void on_init() override;
	void process_event(OUI* element, uint32_t message, uint64_t param, bool bubbleUp) override;
	void enable_element(uint32_t index, bool enable) override;
};