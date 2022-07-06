#include "UIComplexS.h"

void UIComplexS::config(double value, double step, double minValue, double maxValue, size_t numberFieldWidth) {
	slide.set_range(minValue, maxValue);
	slide.set_value(value);
	slide.set_step(step);
	number.set_range(value, step, minValue, maxValue);
	this->numberFieldWidth = (int)numberFieldWidth;
}

void UIComplexS::config(std::string value, std::string step, std::string minValue, std::string maxValue, size_t numberFieldWidth) {
	slide.set_range(minValue, maxValue);
	slide.set_value(value);
	slide.set_step(step);
	number.set_range(value, step, minValue, maxValue);
	this->numberFieldWidth = (int)numberFieldWidth;
}

double UIComplexS::get_value() const {
	return slide.get_value();
}

void UIComplexS::set_text(std::wstring text) {
	label.set_text(text);
}

void UIComplexS::set_gradient(GradientFunc gradientFunc) {
	slide.set_gradient_func(gradientFunc);
}

void UIComplexS::on_init() {
	label.create(this);
	number.create(this);
	slide.create(this);

	number.border.set(1, backgroundColor.bright(-30));
	label.canvas.art.alignX = Align::LEFT;
	config(0, 1, 0, 1, 60);
}

void UIComplexS::on_resize(int width, int height) {
	int l, t, labelW = 100, h = 24, bottomMargin = 5;
	l = 0;
	t = 0;
	label.move(l, t, labelW, h);
	number.move(contentArea.width - numberFieldWidth, t, numberFieldWidth, h);
	t += h + bottomMargin;
	h = 20;
	slide.move(l, contentArea.height - h, contentArea.width, h);
}

void UIComplexS::process_event(OUI* element, uint32_t message, uint64_t param, bool bubbleUp) {
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
