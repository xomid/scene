#include "UIButtonWithBorderOnHover.h"

void UIButtonWithBorderOnHover::on_init() {
	border.set(1, backgroundColor);
}

void UIButtonWithBorderOnHover::enable(bool enable) {
	UIButton::enable(enable);
	if (isHover && isEnabled) {
		border.set(1, hoverBorderColor);
	}
	else {
		border.set(1, backgroundColor);
	}
}

void UIButtonWithBorderOnHover::hover(bool isHover) {
	UIButton::hover(isHover);
	if (isHover && isEnabled) {
		border.set(1, hoverBorderColor);
	}
	else {
		border.set(1, backgroundColor);
	}
	invalidate();
}

void UIButtonWithBorderOnHover::set_hover_border_color(Color color) {
	hoverBorderColor = color;
}

void UIButtonWithBorderOnHover::apply_theme(bool bInvalidate) {
	UIButton::set_background_color(parent->backgroundColor);
	border.set(1, backgroundColor);
}

void UIButtonWithBorderOnHover::on_update() {
	if (!isEnabled) {
		color.save();
		Color alteredColor = color.bright(50);
		colors["currentColor"] = alteredColor;
		color = alteredColor;
		canvas.art.textColor = alteredColor;
		UILabel::on_update();
		color.restore();
		return;
	}
	else {
		canvas.art.textColor = color;
		colors["currentColor"] = color;
	}

	if (!isPressed && !isHover) return UILabel::on_update();
	backgroundColor.save();
	backgroundColor.set(isPressed && isHover ? downBackColor : isHover && !isPressed ? hoverBackColor : backgroundColor);
	UILabel::on_update();
	backgroundColor.restore();
}