#pragma once
#include <oui_stack.h>
#include "zoom_common.h"
#include "Document.h"

struct ImageProperties {
	int left, top, width, height;
	double scale, minScale;
	ImageProperties();
};

class UIMainView : public OUI
{
	Document* document;
	Sheet* image;
	ImageProperties imgProps;
	bool shouldShowDoc;
	Drag<int, int> dragMgr;
	Rect unmodifiedContentArea;
	int clientWidth, clientHeight, scrollSize,
		visibleLeft, visibleRight, visibleTop, visibleBottom;

public:
	UIMainView();

	void show_frame(bool show = true);
	void show_image(bool show = false);
	ZoomInfo get_zoom_info() const;
	void set_zoom_info(ZoomInfo zInfo);
	void set_document(Document* document);
	double get_min_scale() const;
	double get_max_scale() const;

	void reset_view();
	void alert_parent(uint32_t message);
	void measure_content(int* w, int* h);
	void zoom_dir(bool Up);
	void zoom(double newScale);
	void zoom_rel(double increament);
	void zoom(int dir, int lx, int ly);
	void zoom_fit();
	void clamp_image_position();

	void on_update() override;
	void get_content_area(Rect& rc) override;
	void on_resize(int width, int height) override;
	void on_mouse_move(int x, int y, uint32_t flags) override;
	void on_mouse_down(int x, int y, uint32_t flags) override;
	void on_mouse_up(int x, int y, uint32_t flags) override;
	void on_key_down(uint32_t key, uint32_t nrep = 1, uint32_t flags = 0) override;
	void on_dbl_click(int x, int y, uint32_t flags) override;
	bool on_mouse_wheel(int x, int y, int zDelta, uint32_t param) override;
	void process_event(OUI* element, uint32_t message, uint64_t param, bool bubbleUp = true) override;
};
