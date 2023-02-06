#include "UIButtonWithMenu.h"

void UIButtonWithMenu::on_init() {
	triangle = parse_svg(R"(
		<svg x="0px" y="0px" viewBox="0 0 284.356 284.356" fill="currentColor">
			<path d="M0,284.356h284.356V0L0,284.356z"/>
		</svg>
	)");
	triangle->m_colors = &colors;
	bOn = false;
}

void UIButtonWithMenu::on_resize(int width, int height) {
	UIButton::on_resize(width, height);
	if (menu) menu->move(boxModel.left + boxModel.width + 2, 0, 
		menu->boxModel.width, menu->boxModel.height);
}

void UIButtonWithMenu::on_update() {
	UIButtonWithBorderOnHover::on_update();
	int l, t, w, h, m;
	w = 5;
	h = 5;
	m = 2;
	l = contentArea.left + contentArea.width - w - m;
	t = contentArea.left + contentArea.width - h - m;
	canvas.render_svg(triangle, l, t, w, h, 0xff);
}

void UIButtonWithMenu::set_menu(UIMenu* menu) {
	this->menu = menu;
	menu->menuActivationMode = MenuActivationMode::PointerDown;
	menu->menuType = MenuType::Solid;
	menu->set_background_color(backgroundColor);
	menu->border.set(1, Colors::lightgray);
	menu->borderRadius.set(5);
	menu->create(30, 0, 0, 0, this);
	menu->move(boxModel.left + boxModel.width + 2, 0, menu->boxModel.width, menu->boxModel.height);
	menu->add_box_shadow(false, 2, 2, 4, 2, Color("rgb(0, 0, 0, 0.1)"));
}

void UIButtonWithMenu::on_mouse_down(int x, int y, uint32_t flags) {
	if (!isEnabled) return;

	bOn = menu->isVisible;
	UIButton::on_mouse_down(x, y, flags);
	if (bOn) {
		bOn = false;
		menu->fade(); 
	}
	else {
		bOn = true;
	}
}

void UIButtonWithMenu::on_click(int x, int y, uint32_t flags) {
	
}