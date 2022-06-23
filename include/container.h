#pragma once
#include "oui.h"

class UIContainer : public OUI
{
public:
	void on_init() override;
	void process_event(OUI* element, uint32_t message, uint64_t param, bool bubbleUp) override;
};

