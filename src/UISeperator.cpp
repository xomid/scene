#include "UISeperator.h"

void UISeperator::on_init() {
	padding.set(4);
}

void UISeperator::set_color(Color color) {
	UILabel::set_color(color);
	shadowColor.set("#fff");
}

void UISeperator::on_update() {
	UILabel::on_update();
	int y = area.height / 2;
	int l = contentArea.left;
	int r = l + contentArea.width;
	canvas.art.strokeColor.set(color);
	canvas.draw_horizontal_line(y, l, r, 1);
	canvas.art.strokeColor.set(shadowColor);
	canvas.draw_horizontal_line(y + 1, l, r, 1);
}