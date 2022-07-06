#pragma once
#include "UIComplexS.h"

class UIComplexP : public UIComplexS
{
	UILabel lblPercent;
public:
	void on_init() override;
	void on_resize(int width, int height) override;
};


