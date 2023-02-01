#include "UIButtonWithThumbnail.h"
#include "BitBlt.h"

UIButtonWithThumbnail::~UIButtonWithThumbnail() {
	thumb.data = NULL;
	big = false;
}

void UIButtonWithThumbnail::set_thumbnail(byte* thumbData, int width, int height, int pitch, int nbpp) {
	thumb.attach(thumbData, width, height, pitch, nbpp);
	transform(big = true);
}

void UIButtonWithThumbnail::on_update() {
	UIButton::on_update();
	bit_blt(&area, canvas.sheet, rcThumb.left, rcThumb.top, rcThumb.width, rcThumb.height,
		thumb, 0, 0, thumb.w, thumb.h,
		BlendingMode::NORMAL, InterpolationType::NEAREST);
}

void UIButtonWithThumbnail::on_init() {
}

bool UIButtonWithThumbnail::on_mouse_wheel(int x, int y, int zDelta, uint32_t param) {
	x = TOABSX(x);
	y = TOABSX(y);
	x = TORELX(x, parent->area);
	y = TORELX(y, parent->area);
	return parent->on_mouse_wheel(x, y, zDelta, param);
}

void UIButtonWithThumbnail::transform(bool big) {
	this->big = big;

	if (big) {
		canvas.art.alignX = Align::CENTER;
		canvas.art.alignY = Align::BOTTOM;

		int m = 10, textH = canvas.get_box16((wchar_t*)text.c_str(), text.length())->get_height();

		padding.set(m);

		double ds = fmin(
			(double)(boxModel.width - thumbPadding.left - thumbPadding.right) / (double)thumb.w,
			(double)(boxModel.height - thumbPadding.top - thumbPadding.bottom - textH) / (double)thumb.h
		);

		rcThumb.set((boxModel.width - (int)(thumb.w * ds)) / 2, padding.top,
			int(ds * (double)thumb.w), int(ds * (double)thumb.h));
	}
	else {
		canvas.art.alignX = Align::LEFT;
		canvas.art.alignY = Align::CENTER;

		double ds = fmin(
			(double)(boxModel.width - thumbPadding.left - thumbPadding.right) / (double)thumb.w,
			(double)(boxModel.height - thumbPadding.top - thumbPadding.bottom) / (double)thumb.h
		);

		rcThumb.set(thumbPadding.left, (boxModel.height - (int)(thumb.h * ds)) / 2,
			int(ds * (double)thumb.w), int(ds * (double)thumb.h));

		padding.set(0, 0, 0, rcThumb.width + thumbPadding.left + thumbPadding.right);
	}

	reset_size();
}

void UIButtonWithThumbnail::set_thumb_padding(Spacing thumbPadding) {
	this->thumbPadding = thumbPadding;
	transform(big);
}