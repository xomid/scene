#pragma once
#include <oui_number.h>
#include <UISlide.h>

class UIComplexP : public OUI
{
	UILinearSlide slide;
	UINumber number;
	UILabel label, lblPercent;
public:
	void config(double value, double step, double minValue, double maxValue);
	double get_value() const;

	void set_gradient(GradientFunc gradientFunc);
	void set_text(std::wstring text) override;
	void on_init() override;
	void on_resize(int width, int height) override;
	void process_event(OUI* element, uint32_t message, uint64_t param, bool bubbleUp) override;
};

