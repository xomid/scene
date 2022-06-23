#include "UIButtonWithBorderOnHover.h"

void UIButtonWithBorderOnHover::on_init() {
	border.set(1, backgroundColor);
}

void UIButtonWithBorderOnHover::hover(bool bHover) {
	UIButton::hover(bHover);
	if (bHover) {
		border.set(1, hoverBorderColor);
	}
	else {
		border.set(1, backgroundColor);
	}
}

void UIButtonWithBorderOnHover::set_hover_border_color(Color color) {
	hoverBorderColor = color;
}