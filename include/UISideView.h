#pragma once
#include <oui_stack.h>
#include "UISeperator.h"
#include "UIButtonWithMenu.h"

class UISideView : public UIStack
{
	UISeperator sp1, sp2, sp3;
	UIButtonWithBorderOnHover btnClose, btnRedo, btnUndo, btnInfo;
	UIButtonWithMenu btnOpen, btnSave, btnAdj, btnFilter,
		btnHistory, btnZoom;
public:
	void on_init() override;
	void process_event(OUI* element, uint32_t message,
		uint64_t param, bool bubbleUp = true) override;
};

