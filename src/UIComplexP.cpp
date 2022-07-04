#include "UIComplexP.h"

void UIComplexP::config(double value, double step, double minValue, double maxValue) {
	slide.set_range(minValue, maxValue);
	slide.set_value(value);
	slide.set_step(step);
	number.set_range(value, step, minValue, maxValue);
}

double UIComplexP::get_value() const {
	return slide.get_value();
}

void UIComplexP::set_text(std::wstring text) {
	label.set_text(text);
}

void UIComplexP::set_gradient(GradientFunc gradientFunc) {
	slide.set_gradient_func(gradientFunc);
}

void UIComplexP::on_init() {
	label.create(this);
	lblPercent.create(this);
	number.create(this);
	slide.create(this);

	lblPercent.set_text(L"%");
	number.border.set(1, backgroundColor.bright(-30));
	label.canvas.art.alignX = Align::LEFT;
	config(0, 1, 0, 1);
}

void UIComplexP::on_resize(int width, int height) {
	int l, t, labelW = 100, numW = 60, h = 24, bottomMargin = 5, lblPercentW = 10;
	l = 0;
	t = 0;
	label.move(l, t, labelW, h);
	lblPercent.move(contentArea.width - lblPercentW, t, lblPercentW, h);
	number.move(contentArea.width - numW - lblPercentW, t, numW, h);
	t += h + bottomMargin;
	h = 20;
	slide.move(l, contentArea.height - h, contentArea.width, h);
}

void UIComplexP::process_event(OUI* element, uint32_t message, uint64_t param, bool bubbleUp) {
	if (element == &number) {
		auto val = atof(number.get_number().c_str());
		slide.set_value(val);
	}
	else if (element == &slide) {
		auto val = slide.get_value();
		number.set_value(val);
	}
	else return;

	if (parent) {
		parent->process_event(this, Event::Update, 0, true);
	}
}
