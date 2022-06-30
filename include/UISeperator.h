#pragma once
#include <oui_label.h>
class UISeperator : public UILabel
{
	Color lineColor, shadowColor;
public:
	UISeperator();
	void on_update() override;
	void set_line_colors(Color lineColor, Color shadowColor);
};

