#include "UISwirl.h"
#include "ImageEffect.h"

void UISwirl::on_init() {
	img.create(10, 10, 3);
	chkStretch.create(this);
	chkStretch.select(bStretch = true);
	chkStretch.set_type(UIRadioType::Button);
	chkStretch.border.set(1, backgroundColor.bright(-30));
	chkStretch.set_text(LR"(
		<svg viewBox="0 0 410.988 410.988" fill="currentColor">
			<path d="M149.654 195.495a7.5 7.5 0 0 0-7.5-7.5l-78.262-.001 12.527-12.528a7.501 7.501 0 0 0 0-10.606l-14.143-14.143a7.5 7.5 0 0 0-10.607 0L2.197 200.191a7.497 7.497 0 0 0 0 10.606l49.473 49.474a7.483 7.483 0 0 0 5.305 2.197 7.477 7.477 0 0 0 5.303-2.197l14.143-14.143a7.501 7.501 0 0 0 0-10.606l-12.527-12.527 78.262.001a7.5 7.5 0 0 0 7.5-7.5v-20.001zm259.137 4.696-49.473-49.474a7.5 7.5 0 0 0-10.607 0l-14.143 14.143a7.501 7.501 0 0 0 0 10.606l12.527 12.528-78.262.001a7.5 7.5 0 0 0-7.5 7.5v20a7.5 7.5 0 0 0 7.5 7.5l78.262-.001-12.527 12.527a7.501 7.501 0 0 0 0 10.606l14.143 14.143a7.481 7.481 0 0 0 5.303 2.197c1.92 0 3.84-.732 5.305-2.197l49.473-49.474a7.497 7.497 0 0 0-.001-10.605zM164.859 76.42a7.502 7.502 0 0 0 10.608 0l12.527-12.527v78.262c0 1.99.789 3.896 2.195 5.305a7.502 7.502 0 0 0 5.305 2.195h20a7.5 7.5 0 0 0 7.5-7.5V63.893l12.527 12.527a7.502 7.502 0 0 0 10.608 0l14.143-14.143a7.502 7.502 0 0 0 0-10.607L210.797 2.197a7.496 7.496 0 0 0-10.605 0L150.717 51.67a7.485 7.485 0 0 0-2.195 5.305c0 1.918.733 3.838 2.195 5.303l14.142 14.142zm81.27 258.148a7.503 7.503 0 0 0-10.608 0l-12.527 12.527v-78.262a7.5 7.5 0 0 0-7.5-7.5h-20c-1.99 0-3.898.789-5.305 2.195a7.504 7.504 0 0 0-2.195 5.305v78.262l-12.527-12.527a7.501 7.501 0 0 0-10.608 0l-14.143 14.143a7.484 7.484 0 0 0-2.195 5.303c0 1.92.733 3.84 2.195 5.305l49.475 49.473a7.496 7.496 0 0 0 10.605 0l49.475-49.473a7.502 7.502 0 0 0 0-10.607l-14.142-14.144z"/>
		</svg>
	)");
	chkStretch.canvas.art.alignX = Align::CENTER;
	chkStretch.canvas.art.alignY = Align::CENTER;
	chkStretch.padding.set(4);
	angle = 0;
}

bool UISwirl::is_stretched() const {
	return bStretch;
}

void UISwirl::set_angle(double angle) {
	this->angle = angle;
	fill_image();
}

void UISwirl::on_update() {
	chkStretch.set_color(Color(64, 64, 64));
	canvas.bit_blt(img, contentArea.left, contentArea.top, contentArea.width, contentArea.height,
		0, 0, true);
}

void UISwirl::fill_image() {
	Canvas can(NULL, &img);
	can.clear(NULL, &Color("#fff"));

	pixfmt_bgr pf(&img, 0);
	agg::path_storage path;
	agg::conv_stroke<agg::path_storage> stroke(path);
	stroke.width(1);
	stroke.line_join(agg::line_join_e::bevel_join);
	stroke.line_cap(agg::line_cap_e::square_cap);
	stroke.inner_join(agg::inner_join_e::inner_bevel);

	Color cr(5, 80, 120);

	int p = img.pitch;
	double u, v, theta, t, r, un_x, un_y, cw, ch;
	double twist = angle * angle * ((angle > 0) ? 1 : -1);
	int x2, y2, right, bottom;
	double size = 0.9;

	int x, y, w, h;
	w = img.w;
	h = img.h;
	cw = w / 2.0;
	ch = h / 2.0;
	right = w - 1;
	bottom = h - 1;
	double invmaxrad = 1.0 / ch;
	double sc = (double)w / (double)h;
	double ss = 1;
	double iwmax = w - 1;
	double ihmax = h - 1;

	if (!bStretch)
		sc = ss = 1;
	v = 0;

	path.move_to(0, ch);
	for (x = 0; x < w; x++) {
		u = (x - cw) / sc;

		r = sqrt(u * u);
		theta = atan2(0, u);

		t = 1 - ((r * size) * invmaxrad);
		t = (t < 0) ? 0 : (t * t * t);
		theta += (t * twist) / 100.0;

		un_x = fmax(fmin(cw + r * cos(theta) * sc, iwmax), 0.);
		un_y = fmax(fmin(ch + r * sin(theta) * ss, ihmax), 0.);

		x2 = (int)(un_x + 0.5);
		y2 = (int)(un_y + 0);
		path.line_to(x2, y2);
	}

	path.move_to(cw + 1, 0);
	for (y = 0; y < h; y++)
	{
		v = (y - ch);

		r = sqrt(v * v);
		theta = atan2(v, 0);

		t = 1 - ((r * size) * invmaxrad);
		t = (t < 0) ? 0 : (t * t * t);
		theta += (t * twist) / 100.0;

		un_x = fmax(fmin(cw + r * cos(theta) * sc, iwmax), 0.);
		un_y = fmax(fmin(ch + r * sin(theta) * ss, ihmax), 0.);

		x2 = (int)(un_x + 0.5);
		y2 = (int)(un_y + 0);
		path.line_to(x2, y2);
	}

	can.add_path<agg::conv_stroke<agg::path_storage>>(stroke);
	can.render(cr);

	invalidate();
}

void UISwirl::on_resize(int width, int height) {
	img.resize(contentArea.width, contentArea.height);
	fill_image();

	int margin = 10;
	int btnW = 30, btnH = 30;
	chkStretch.move(contentArea.width - btnW - margin, margin, btnW, btnH);
}

void UISwirl::process_event(OUI* element, uint32_t message, uint64_t param, bool bubbleUp) {
	if (element == &chkStretch) {
		if (message == Event::Select || message == Event::Deselect) {
			chkStretch.select(message == Event::Select);
			bStretch = chkStretch.isSelected;
			fill_image();
		}
	}
	else {
		OUI::process_event(element, message, param, bubbleUp);
	}
}
