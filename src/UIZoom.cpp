#include "UIZoom.h"
#include "BitBlt.h"

void UIZoom::on_init() {
	document = 0;
	zInfo.reset();
	rcZoom.set(0, 0, 10, 10);
	border.set(1, Colors::purple);
	zoomBorder.set(1, Colors::red);
	zoomBorderOpacity = 0xff;
}

void UIZoom::set_document(Document* document) {
	this->document = document;
	invalidate();
}

ZoomInfo UIZoom::get_zoom_info() const {
	return zInfo;
}

void UIZoom::set_zoom_info(ZoomInfo zInfo) {
	this->zInfo = zInfo;

	Sheet* srcImage = get_image();
	if (srcImage) {
		int l, t, w, h;
		double rat = srcImage->w / double(srcImage->h);
		w = contentArea.width;
		h = int(double(w) / rat);
		l = 0;
		t = (contentArea.height - h) / 2;
		if (h > contentArea.height)
		{
			t = 0;
			h = contentArea.height;
			w = int(double(h) * rat);
			l = (contentArea.width - w) / 2;
		}

		rcImage.set(l, t, w, h);
		rcZoom.set(int(zInfo.l * double(w)) + l, int(zInfo.t * h) + t, int(w * zInfo.w), int(h * zInfo.h));

		int r = CLAMP3(rcImage.left, rcZoom.right(), rcImage.right());
		rcZoomClipped.left = CLAMP3(rcImage.left, rcZoom.left, rcImage.right());
		rcZoomClipped.width = r - rcZoomClipped.left;

		int b = CLAMP3(rcImage.top, rcZoom.bottom(), rcImage.bottom());
		rcZoomClipped.top = CLAMP3(rcImage.top, rcZoom.top, rcImage.bottom());
		rcZoomClipped.height = b - rcZoomClipped.top;
	}
}

void UIZoom::on_update() {
	Sheet* srcImage = get_image();
	if (srcImage) {
		bit_blt_fill_rest(&area, canvas.sheet, contentArea.left + rcImage.left, contentArea.top + rcImage.top,
			rcImage.width, rcImage.height,
			*srcImage, 0, 0, srcImage->w, srcImage->h,
			BlendingMode::NORMAL, InterpolationType::NEAREST, backgroundColor);
	}
	else {
		OUI::on_update();
	}

	canvas.draw_box(rcZoomClipped, zoomBorder, zoomBorderOpacity);
}

void UIZoom::on_resize(int width, int height) {
	Sheet* srcImage = get_image();
	if (srcImage) {
		set_zoom_info(zInfo);
	}
	else {
		rcZoom.set(contentArea);
	}
	invalidate();
}

Sheet* UIZoom::get_image() const {
	if (document == NULL || !document->is_open()) return NULL;
	return document->get_image();
}

void UIZoom::on_mouse_down(int x, int y, uint32_t flags) {
	Sheet* srcImage = get_image();
	if (srcImage == NULL) return;

	UIButton::on_mouse_down(x, y, flags);

	double fx = (x - rcImage.left) / double(rcImage.width);
	double fy = (y - rcImage.top) / double(rcImage.height);

	if (!rcZoom.is_inside(x, y)) {
		zInfo.l = (x - (rcZoom.width >> 1) - rcImage.left) / double(rcImage.width);
		zInfo.t = (y - (rcZoom.height >> 1) - rcImage.top) / double(rcImage.height);
	}
	
	dragMgr.startDragging(fx, fy, zInfo.l, zInfo.t);
	on_mouse_move(x, y, flags);
}

void UIZoom::on_mouse_up(int x, int y, uint32_t flags) {
	Sheet* srcImage = get_image();
	if (srcImage == NULL) return;

	UIButton::on_mouse_up(x, y, flags);
	dragMgr.stopDragging();
}

void UIZoom::on_mouse_move(int x, int y, uint32_t flags) {
	Sheet* srcImage = get_image();
	if (srcImage == NULL) return;

	if (isPressed) {
		double fx = (x - rcImage.left) / double(rcImage.width);
		double fy = (y - rcImage.top) / double(rcImage.height);
		dragMgr.drag(fx, fy, zInfo.l, zInfo.t);

		zInfo.l = fmin(fmax(zInfo.l, zInfo.minLeft), zInfo.maxLeft);
		zInfo.t = fmin(fmax(zInfo.t, zInfo.minTop), zInfo.maxTop);

		set_zoom_info(zInfo);

		invalidate();
		alert_parent(0);
	}
}

void UIZoom::alert_parent(uint32_t message) {
	Sheet* srcImage = get_image();
	if (srcImage == NULL) return;

	if (parent)
		parent->process_event(this, Event::Update, 0, true);
}
