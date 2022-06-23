#pragma once
#include <oui_label.h>
class UISeperator : public UILabel
{
	Color shadowColor;
public:
	void on_init() override;
	void on_update() override;
	void set_color(Color color) override;
};

