#pragma once
#include <oui_menu.h>
#include "UIComMenu.h"

void UIComMenu::set_title(std::wstring title) {
	lblTitle.set_text(title);
	invalidate();
}

void UIComMenu::on_init() {
	int w = boxModel.width;
	int h = 30;
	int t = h;

	list.create(0, t, w, boxModel.height - t, this);
	list.mode = UIStackMode::STACKVER;

	auto c = backgroundColor.bright(-20);
	lblTitle.create(0, 0, w, h, this);
	lblTitle.border.set(0, 0, 1, Colors::white, Colors::white, c.bright(-20));
	lblTitle.set_background_color(c);
	lblTitle.set_color(Color("#444"));
	lblTitle.canvas.art.alignX = Align::LEFT;
	lblTitle.padding.set(10, 0);
}

void UIComMenu::apply_theme(bool shouldInvalidate) {
	set_background_color(OUITheme::primary);
	set_color(OUITheme::text);
	if (shouldInvalidate) invalidate();
}

void UIComMenu::on_resize(int width, int height) {
	OUI::on_resize(width, height);
	int w = boxModel.width;
	int t = 30;
	int h = boxModel.height - t;
	list.move(0, t, w, h);
}

void UIComMenu::enable_element(uint32_t index, bool enable) {
}

UIStack* UIComMenu::get_list() {
	return &list;
}