#pragma once
#include <oui_window.h>
#include <oui_check.h>

class UIDCommon : public UIDialog {
	UICheck chkPreview;
public:
	void on_init() override;
	void on_resize(int width, int height) override;
	void process_event(OUI* element, uint32_t message, uint64_t param, bool bubbleUp) override;
};