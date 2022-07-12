#pragma once
#include <oui_menu.h>
#include "UIButtonWithBorderOnHover.h"

class UIButtonWithMenu : public UIButtonWithBorderOnHover
{
	PSVGShape triangle;
	bool bOn;
	UIMenu* solidMenu;

public:
	void on_update() override;
	void on_init() override;
	void on_resize(int width, int height) override;
	void on_mouse_down(int x, int y, uint32_t flags) override;
	void on_click(int x, int y, uint32_t flags) override;
	void set_menu(UIMenu* menu);
};

