#include "UISeparator.h"

UISeparator::UISeparator() {
	set_line_colors(Color("#ccc"), Colors::white);
}

void UISeparator::set_line_colors(Color lineColor, Color shadowColor) {
	this->lineColor.set(lineColor);
	this->shadowColor.set(shadowColor);
}

void UISeparator::on_update() {
	UILabel::on_update();

	int y = area.height / 2, l, r;
	
	if (text.length()) {
		int l = 0;
		int m = 4;
		int r = contentArea.left - m;

		canvas.art.strokeColor.set(lineColor);
		canvas.draw_horizontal_line(y, l, r, 1);
		canvas.art.strokeColor.set(shadowColor);
		canvas.draw_horizontal_line(y + 1, l, r, 1);

		auto box = canvas.get_box16((wchar_t*)text.c_str(), text.length());

		l = contentArea.left + box->get_width() + m;
		r = contentArea.left + contentArea.width;
		canvas.art.strokeColor.set(lineColor);
		canvas.draw_horizontal_line(y, l, r, 1);
		canvas.art.strokeColor.set(shadowColor);
		canvas.draw_horizontal_line(y + 1, l, r, 1);
	}
	else {
		l = contentArea.left;
		r = contentArea.left + contentArea.width;
		canvas.art.strokeColor.set(lineColor);
		canvas.draw_horizontal_line(y, l, r, 1);
		canvas.art.strokeColor.set(shadowColor);
		canvas.draw_horizontal_line(y + 1, l, r, 1);
	}
}