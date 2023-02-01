#include "UIRadialBlur.h"
#include "RadialBlurPattern.h"
#include "ImageEffect.h"

void UIRadialBlur::get_pivot_point(double& cx, double& cy) {
	cx = this->cx;
	cy = this->cy;
}

void UIRadialBlur::set_amount(double amount) {
	this->amount = amount;
	fill_image();
}

void UIRadialBlur::set_blur_mode(RadialBlurMode blurMode) {
	this->blurMode = blurMode;
	fill_image();
}

int offsetX, offsetY;

void UIRadialBlur::on_init() {
	img.create(10, 10, 3);
	imgPattern.create(RADIAL_BLUR_PATTERN_WIDTH, RADIAL_BLUR_PATTERN_WIDTH, 3);
	blurMode = RadialBlurMode::Zoom;
	cx = .5;
	cy = .5;
	calc_offset();
}

void UIRadialBlur::on_update() {
	canvas.bit_blt(img, contentArea.left, contentArea.top, contentArea.width, contentArea.height,
		offsetX, offsetY, true);
}

void UIRadialBlur::calc_offset() {
	cx = fmin(fmax(cx, .0), 1.);
	cy = fmin(fmax(cy, .0), 1.);
	offsetX = int((1. - cx) * (double)img.w / 2.);
	offsetY = int((1. - cy) * (double)img.h / 2.);
}

void UIRadialBlur::fill_image() {

	constexpr int RADIUS_LENGTH = 64;
	int fcx, fcy, fr, sb, sg, sr, sc, u, v, fx, fy, i, fsr, ox1, ox2, oy1, oy2, x, y, w, h, p, sp, fcxx, fcyy, ffsr, right, bottom;
	pyte srcData, disData, d;

	sb = 0;
	sg = 0;
	sr = 0;
	w = img.w, h = img.h;
	disData = img.data;
	p = img.pitch;

	int length = (int)amount;
	fr = (int)(amount * PI * 65536.0 / 181.0);
	fcx = (int)(img.w * 32768);
	fcy = (int)(img.h * 32768);
	fsr = fr / RADIUS_LENGTH;
	fcxx = fcx + 32768;
	fcyy = fcy + 32768;
	ffsr = -fsr;

	byte gray;
	int sx, sy;
	srcData = imgPattern.data;
	sp = RADIAL_BLUR_PATTERN_WIDTH;
	right = RADIAL_BLUR_PATTERN_WIDTH - 1, bottom = RADIAL_BLUR_PATTERN_WIDTH - 1;

	if (blurMode == RadialBlurMode::Spin)
	{
		sc = 2 * RADIUS_LENGTH + 1;

		for (y = 0; y < h; y++)
		{
			d = disData + y * p;
			for (x = 0; x < w; x++)
			{
				sx = x * RADIAL_BLUR_PATTERN_WIDTH / w;
				sy = y * RADIAL_BLUR_PATTERN_WIDTH / h;
				sb = RadialBlurPatternData[sy * sp + sx];

				ox1 = ox2 = fx = (x << 16) - fcx;
				oy1 = oy2 = fy = (y << 16) - fcy;

				for (i = 0; i < RADIUS_LENGTH; i++)
				{
					ox1 = ox1 - ((oy1 >> 8) * fsr >> 8) - ((ox1 >> 14) * (fsr * fsr >> 11) >> 8);
					oy1 = oy1 + ((ox1 >> 8) * fsr >> 8) - ((oy1 >> 14) * (fsr * fsr >> 11) >> 8);

					ox2 = ox2 - ((oy2 >> 8) * ffsr >> 8) - ((ox2 >> 14) * (fsr * fsr >> 11) >> 8);
					oy2 = oy2 + ((ox2 >> 8) * ffsr >> 8) - ((oy2 >> 14) * (fsr * fsr >> 11) >> 8);

					u = (ox1 + fcxx) >> 16;
					v = (oy1 + fcyy) >> 16;

					sb += RadialBlurPatternData[Max(Min(v, bottom), 0) * sp + Max(Min(u, right), 0)];

					u = (ox2 + fcxx) >> 16;
					v = (oy2 + fcyy) >> 16;

					sb += RadialBlurPatternData[Max(Min(v, bottom), 0) * sp + Max(Min(u, right), 0)];
				}

				sb = Max(Min(sb / sc, 255), 0);
				sb = sb != 255 ? 0 : 255;

				gray = CLAMP255(sb);
				*d++ = gray;
				*d++ = gray;
				*d++ = gray;
			}
		}
	}
	else
	{
		sc = RADIUS_LENGTH + 1;
		for (y = 0; y < h; y++) {
			d = disData + y * p;
			for (x = 0; x < w; x++) {

				sx = x * RADIAL_BLUR_PATTERN_WIDTH / w;
				sy = y * RADIAL_BLUR_PATTERN_WIDTH / h;
				sb = RadialBlurPatternData[sy * sp + sx];

				fx = (x << 16) - fcx;
				fy = (y << 16) - fcy;

				for (i = 0; i < RADIUS_LENGTH; i++)
				{
					fx = fx - (((fx >> 4) * length) >> 10);
					fy = fy - (((fy >> 4) * length) >> 10);

					u = (fx + fcxx) >> 16;
					v = (fy + fcyy) >> 16;

					sb += RadialBlurPatternData[Max(Min(v, bottom), 0) * sp + Max(Min(u, right), 0)];
				}

				sb = Max(Min(sb / sc, 255), 0);
				sb = sb != 255 ? 0 : 255;

				gray = CLAMP255(sb);
				*d++ = gray;
				*d++ = gray;
				*d++ = gray;
			}
		}
	}
}

void UIRadialBlur::on_resize(int width, int height) {
	img.resize(contentArea.width * 2, contentArea.height * 2);
	calc_offset();
	fill_image();
}

void UIRadialBlur::on_mouse_move(int x, int y, uint32_t flags) {
	if (bPressed) {
		cx = x / double(contentArea.width);
		cy = y / double(contentArea.height);
		calc_offset();
		if (parent)
			parent->process_event(this, Event::Update, 0, true);
	}
}

void UIRadialBlur::on_mouse_down(int x, int y, uint32_t flags) {
	bPressed = true;
	set_capture(this);
	on_mouse_move(x, y, flags);
}

void UIRadialBlur::on_mouse_up(int x, int y, uint32_t flags) {
	bPressed = false;
	if (get_capture() == this)
		release_capture();
}

void UIRadialBlur::on_dbl_click(int x, int y, uint32_t flags) {
	on_mouse_down(x, y, flags);
}
