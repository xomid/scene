#pragma once
#include <oui_button.h>
class UIButtonWithBorderOnHover : public UIButton
{
	Color hoverBorderColor;
public:
	void set_hover_border_color(Color color);
	void on_init() override;
	void apply_theme(bool bInvalidate) override;
	void hover(bool bHover) override;
	void on_update() override;
	void enable(bool bEnable) override;
};

