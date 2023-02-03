#include "UIMainView.h"
#include "BitBlt.h"
#include <oui_scroll.h>

#define DEFAULT_IMAGE_MIN_VISIBLE_SIZE 100
#define DEFAULT_IMAGE_MIN_SCALE 0.01
#define DEFAULT_IMAGE_MAX_SCALE 30.0
#define MAX_SCALE    32.0
#define MIN_SCALE    0.01 

ImageProperties::ImageProperties()
	: left(0), top(0), width(0), height(0), scale(1.0), minScale(0.01) {
}

static double ZoomLevel[29] =
{
	.01,
	.063,
	.07,
	.01,
	.015,
	.02,
	.03,
	.04,
	.05,
	.0625,
	.0833,
	.125,
	.1667,
	.25,
	.3333,
	.5,
	.6667,
	1.0,
	2.0,
	3.0,
	4.0,
	5.0,
	6.0,
	7.0,
	8.0,
	12.0,
	16.0,
	32.0,
	48.0,
};

UIMainView::UIMainView()
	: clientWidth(0), clientHeight(0), scrollSize(10),
	visibleLeft(0), visibleTop(0), visibleRight(0), visibleBottom(0), document(0), image(0)
{
	bScrollable = true;
	bShowDoc = false;
}

void UIMainView::set_document(Document* document) {
	if (!document)
		image = 0;
	else
		image = document->get_frame();
	this->document = document;
}

void UIMainView::reset_view() {
	if (!image) return;
	imgProps.minScale = fmin(
		100.0 / (double)image->w,
		100.0 / (double)image->h
	);
	zoom_fit();
}

void UIMainView::show_image(bool show) {
	bShowDoc = show;
	if (show && document) image = document->get_frame();
	else image = 0;
	zoom(imgProps.scale);
	invalidate();
}

void UIMainView::show_frame(bool show) {
	if (!document) image = NULL;
	else if (show) image = document->get_frame();
	else image = document->get_image();
	invalidate();
}

void UIMainView::on_update() {
	if (image) {
		bit_blt_fill_rest(&area, canvas.sheet, contentArea.left + imgProps.left, contentArea.top + imgProps.top,
			imgProps.width, imgProps.height,
			*image, 0, 0, image->w, image->h,
			BlendingMode::NORMAL, InterpolationType::NEAREST, backgroundColor);
	}
	else {
		OUI::on_update();
	}
}

void UIMainView::get_content_area(Rect& rc) {
	OUI::get_content_area(rc);
	if (scrollX && scrollX->bVisible) rc.height -= scrollX->boxModel.height;
	if (scrollY && scrollY->bVisible) rc.width -= scrollY->boxModel.width;
}

void UIMainView::on_resize(int width, int height) {
	scrollX->move(0, boxModel.height - scrollSize, boxModel.width - scrollSize, scrollSize);
	scrollY->move(boxModel.width - scrollSize, 0, scrollSize, boxModel.height - scrollSize);
	zoom(imgProps.scale);
}

void UIMainView::measure_content(int* w, int* h) {
	if (w) *w = imgProps.width + (imgProps.width > boxModel.width ? boxModel.width - DEFAULT_IMAGE_MIN_VISIBLE_SIZE : 0);
	if (h) *h = imgProps.height + (imgProps.height > boxModel.height ? boxModel.height - DEFAULT_IMAGE_MIN_VISIBLE_SIZE : 0);
}

void UIMainView::on_mouse_move(int x, int y, uint32_t flags) {
	if (!image) return;
	if (dragMgr.mouseAction == MouseDragStatus::STRATED) {
		dragMgr.drag(x, y, imgProps.left, imgProps.top);
		clamp_image_position();
		invalidate();
		alert_parent(1);
	}
}

void UIMainView::clamp_image_position() {

	if (imgProps.width <= unmodifiedContentArea.width &&
		imgProps.height <= unmodifiedContentArea.height) {
		imgProps.left = (unmodifiedContentArea.width - imgProps.width) >> 1;
		imgProps.top = (unmodifiedContentArea.height - imgProps.height) >> 1;
	}
	else {
		if (imgProps.left < visibleLeft)
			imgProps.left = visibleLeft;
		else if (imgProps.left > visibleRight)
			imgProps.left = visibleRight;

		if (imgProps.top < visibleTop)
			imgProps.top = visibleTop;
		else if (imgProps.top > visibleBottom)
			imgProps.top = visibleBottom;
	}

	move_page(visibleRight - imgProps.left, visibleBottom - imgProps.top);
}

void UIMainView::on_mouse_down(int x, int y, uint32_t flags) {
	if (!image) return;
	dragMgr.startDragging(x, y, imgProps.left, imgProps.top, MouseDragDirection::BOTHDIR);
	set_capture(this);
}

void UIMainView::on_mouse_up(int x, int y, uint32_t flags) {
	if (!image) return;
	dragMgr.stopDragging();
	if (get_capture() == this)
		release_capture();
}

bool UIMainView::on_mouse_wheel(int x, int y, int zDelta, uint32_t param) {
	if (!image) return false;
	if (zDelta) zoom(abs(zDelta) / zDelta, x - contentArea.left, y - contentArea.top);
	alert_parent(1);
	return true;
}

void UIMainView::on_key_down(uint32_t key, uint32_t nrep, uint32_t flags) {
	if (!image) return;
	auto bZoom = true;

	if (bZoom)
	{
		switch (key)
		{
		case 187: case 107: zoom(+1); break;
		case 189: case 109: zoom(-1); break;
		case 48: case 96:
			imgProps.scale = 1.0;
			int iw = int(imgProps.scale * image->w) - imgProps.width, ih = int(imgProps.scale * image->h) - imgProps.height;

			int x = ((clientWidth - 10) / 2) - imgProps.left;
			int y = ((clientHeight - 10) / 2) - imgProps.top;

			double dx = (double)x / (double)imgProps.width;
			double dy = (double)y / (double)imgProps.height;

			imgProps.left -= int(iw * dx);
			imgProps.top -= int(ih * dy);

			zoom(imgProps.scale);
			break;
		}
	}
}

void UIMainView::zoom(double newScale)
{
	if (!image) {
		scrollX->set_page(0, unmodifiedContentArea.width);
		scrollY->set_page(0, unmodifiedContentArea.height);
		scrollX->show_window(false);
		scrollY->show_window(false);
		return;
	}

	OUI::get_content_area(unmodifiedContentArea);
	imgProps.scale = fmax(fmin(newScale, MAX_SCALE), imgProps.minScale);
	imgProps.width = int(imgProps.scale * image->w);
	imgProps.height = int(imgProps.scale * image->h);

	clientWidth = unmodifiedContentArea.width;
	clientHeight = unmodifiedContentArea.height;

	int pageX = imgProps.width;
	int pageY = imgProps.height;

	if (pageX > unmodifiedContentArea.width || pageY > unmodifiedContentArea.height) {
		clientWidth -= scrollSize;
		clientHeight -= scrollSize;
		pageX += 2 * (unmodifiedContentArea.width - DEFAULT_IMAGE_MIN_VISIBLE_SIZE);
		pageY += 2 * (unmodifiedContentArea.height - DEFAULT_IMAGE_MIN_VISIBLE_SIZE);
	}

	scrollX->set_page(pageX, unmodifiedContentArea.width);
	scrollY->set_page(pageY, unmodifiedContentArea.height);

	visibleLeft = unmodifiedContentArea.left + DEFAULT_IMAGE_MIN_VISIBLE_SIZE - imgProps.width;
	visibleRight = unmodifiedContentArea.left + unmodifiedContentArea.width - DEFAULT_IMAGE_MIN_VISIBLE_SIZE;
	visibleTop = unmodifiedContentArea.top + DEFAULT_IMAGE_MIN_VISIBLE_SIZE - imgProps.height;
	visibleBottom = unmodifiedContentArea.top + unmodifiedContentArea.height - DEFAULT_IMAGE_MIN_VISIBLE_SIZE;

	clamp_image_position();
	invalidate();
}

void UIMainView::zoom_dir(bool Up) // -1 down, 0 set, 1 up
{
	double ds = imgProps.scale;
	int i = 0;

	if (Up)
	{
		for (i = 0; i < 28; i++)
			if (ds >= ZoomLevel[i] && ds < ZoomLevel[i + 1])
			{
				ds = imgProps.scale = (i == 27) ? ZoomLevel[27] : ZoomLevel[i + 1];
				break;
			}
	}
	else
	{
		for (i = 1; i < 28; i++)
			if (ds <= ZoomLevel[i] && ds > ZoomLevel[i - 1])
			{
				ds = imgProps.scale = i < 2 ? ZoomLevel[1] : ZoomLevel[i - 1];
				break;
			}

	}

	int iw = int(ds * image->w - imgProps.width), ih = int(ds * image->h) - imgProps.height;

	int x = (clientWidth - 10) / 2 - imgProps.left;
	int y = (clientHeight - 10) / 2 - imgProps.top;

	double dx = (double)x / (double)imgProps.width;
	double dy = (double)y / (double)imgProps.height;

	imgProps.left -= int(iw * dx);
	imgProps.top -= int(ih * dy);


	zoom(ds);
}

void UIMainView::zoom_rel(double ds) // -1 down, 0 set, 1 up
{
	if (!image) return;

	imgProps.scale = ds;
	int iw = int(ds * image->w) - imgProps.width, ih = int(ds * image->h) - imgProps.height;

	int x = (clientWidth - 10) / 2 - imgProps.left;
	int y = (clientHeight - 10) / 2 - imgProps.top;

	double dx = (double)x / (double)imgProps.width;
	double dy = (double)y / (double)imgProps.height;

	imgProps.left -= int(iw * dx);
	imgProps.top -= int(ih * dy);

	zoom(imgProps.scale);
}

void UIMainView::zoom(int dir, int lx, int ly)
{
	if (!image) return;
	double ds;

	ds = imgProps.scale + (imgProps.scale / 4 * dir);
	imgProps.scale = ds = fmax(fmin(ds, MAX_SCALE), imgProps.minScale);

	if (ds < 1.1 && ds > 0.9) imgProps.scale = ds = 1.0;

	int iw = int(ds * image->w) - imgProps.width, ih = int(ds * image->h) - imgProps.height;

	int x = lx - imgProps.left;
	int y = ly - imgProps.top;

	double dx = (double)x / (double)imgProps.width;
	double dy = (double)y / (double)imgProps.height;

	imgProps.left -= int(iw * dx);
	imgProps.top -= int(ih * dy);

	zoom(imgProps.scale);
}

void UIMainView::on_dbl_click(int x, int y, uint32_t flags) {
	zoom_fit();
}

void UIMainView::zoom_fit() {
	if (!image) return;
	if (image->w > 0 && image->h > 0) {
		Spacing padding;
		padding.set(20);

		double ds = fmin(
			(double)(unmodifiedContentArea.width - padding.left - padding.right) / (double)image->w,
			(double)(unmodifiedContentArea.height - padding.top - padding.bottom) / (double)image->h
		);

		imgProps.left = (int)round(((double)unmodifiedContentArea.width - (double)image->w * ds) / 2.0);
		imgProps.top = (int)round(((double)unmodifiedContentArea.height - (double)image->h * ds) / 2.0);

		zoom(ds);
	}
}

void UIMainView::process_event(OUI* element, uint32_t message, uint64_t param, bool bubbleUp) {
	if (!image) return;
	if (message == Event::Scroll) {

		OUI::process_event(element, message, param, false);

		int posX = scrollX->get_pos();
		int posY = scrollY->get_pos();

		imgProps.left = visibleRight - posX;
		imgProps.top = visibleBottom - posY;

		//alert_parent(1);
		invalidate();
	}
}

void UIMainView::alert_parent(uint32_t message) {
	if (parent)
		parent->process_event(this, Event::Update, message, true);
}

void UIMainView::set_zoom_info(ZoomInfo zInfo) {
	if (!image) return;
	imgProps.width = int(zInfo.scale * image->w);
	imgProps.height = int(zInfo.scale * image->h);
	imgProps.left = int(-zInfo.l * imgProps.width);
	imgProps.top = int(-zInfo.t * imgProps.height);
	clamp_image_position();
	zoom(zInfo.scale);
}

ZoomInfo UIMainView::get_zoom_info() const {
	ZoomInfo res;
	res.reset();
	res.scale = imgProps.scale;

	if (imgProps.width > contentArea.width || imgProps.height > contentArea.height) {
		res.l = -imgProps.left / double(imgProps.width);
		res.t = -imgProps.top / double(imgProps.height);
		res.w = clientWidth / double(imgProps.width);
		res.h = clientHeight / double(imgProps.height);
		res.scale = imgProps.scale;

		double fx = double(DEFAULT_IMAGE_MIN_VISIBLE_SIZE) / double(imgProps.width);
		double fy = double(DEFAULT_IMAGE_MIN_VISIBLE_SIZE) / double(imgProps.height);
		res.maxLeft = fmin(1., 1 - fx);
		res.maxTop = fmin(1., 1 - fy);

		fx = double(clientWidth - DEFAULT_IMAGE_MIN_VISIBLE_SIZE) / double(imgProps.width);
		fy = double(clientHeight - DEFAULT_IMAGE_MIN_VISIBLE_SIZE) / double(imgProps.height);
		res.minLeft = fmax(-1., -fx);
		res.minTop = fmax(-1., -fy);
	}

	return res;
}

double UIMainView::get_min_scale() const {
	return double(int(imgProps.minScale * 100.) / 100.);
}

double UIMainView::get_max_scale() const {
	return MAX_SCALE;
}