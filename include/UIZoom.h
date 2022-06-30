#pragma once
#include <oui_button.h>
#include "zoom_common.h"
#include "Document.h"

class UIZoom : public UIButton
{
	ZoomInfo zInfo;
	Rect rcZoom, rcImage, rcZoomClipped;
	Document* document;
	Sheet* srcImage;
	Border zoomBorder;
	byte zoomBorderOpacity;
	Drag<double, double> dragMgr;

public:
	void on_init() override;
	void on_update() override;
	void set_document(Document* document);

	void alert_parent(uint32_t message);
	ZoomInfo get_zoom_info() const;
	void set_zoom_info(ZoomInfo zInfo);

	void on_resize(int width, int height) override;
	void on_mouse_move(int x, int y, uint32_t flags) override;
	void on_mouse_down(int x, int y, uint32_t flags) override;
	void on_mouse_up(int x, int y, uint32_t flags) override;
};

