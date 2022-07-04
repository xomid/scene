#pragma once
#include <oui_label.h>
class UISeparator : public UILabel
{
	Color lineColor, shadowColor;
public:
	UISeparator();
	void on_update() override;
	void set_line_colors(Color lineColor, Color shadowColor);
};

