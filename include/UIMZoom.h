#pragma once
#include "UIComMenu.h"
#include "UIZoom.h"
#include "Document.h"
#include <oui_button.h>
#include <oui_number.h>
#include <UISlide.h>

#define UIZOOM_UPDATE (Event::_last + 8)

class UIMZoom : public UIComMenu
{
	UIButton btnUp, btnDown;
	UILinearSlide slide;
	UINumber numScale;
	UILabel lblPercent;
	UIZoom view;
	double minScale, maxScale;

public:
	void on_init() override;
	void on_resize(int width, int height) override;
	void set_document(Document* document);

	ZoomInfo get_zoom_info() const;
	void set_zoom_info(ZoomInfo zInfo);
	void set_scale_range(double minValue, double maxValue);

	void process_event(OUI* element, uint32_t message, uint64_t param, bool bubbleUp) override;
};