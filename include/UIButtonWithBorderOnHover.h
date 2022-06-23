#pragma once
#include <oui_button.h>
class UIButtonWithBorderOnHover : public UIButton
{
	Color hoverBorderColor;
public:
	void set_hover_border_color(Color color);
	void on_init() override;
	void hover(bool bHover) override;
};

