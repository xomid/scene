#include "UIComplexP.h"

void UIComplexP::on_init() {
	UIComplexS::on_init();
	lblPercent.create(this);
	lblPercent.set_text(L"%");
}

void UIComplexP::on_resize(int width, int height) {
	int l, t, labelW = 100, h = 24, bottomMargin = 5, lblPercentW = 10;
	l = 0;
	t = 0;
	label.move(l, t, labelW, h);
	lblPercent.move(contentArea.width - lblPercentW, t, lblPercentW, h);
	number.move(contentArea.width - numberFieldWidth - lblPercentW, t, numberFieldWidth, h);
	t += h + bottomMargin;
	h = 20;
	slide.move(l, contentArea.height - h, contentArea.width, h);
}