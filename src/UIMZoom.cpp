#include "UIMZoom.h"
#include <easing_funcs.h>

void UIMZoom::on_init() {
	boxModel.width = MENU_WIDTH;
	boxModel.height = MENU_HEIGHT + 50;

	set_title(L"Navigator");
	UIComMenu::on_init();

	list.show_window(false);

	int w, h;
	view.create(0, 0, 10, 10, this);
	view.margin.set(5, 10);

	h = 30;
	w = 50;
	btnUp.create(0, 0, w, h, this);
	btnDown.create(0, 0, w, h, this);
	numScale.create(0, 0, w, h, this);
	lblPercent.create(0, 0, 10, h, this);

	slide.create(0, 0, 10, 10, this);

	numScale.set_text(L"100");
	btnUp.set_text(L"+");
	btnDown.set_text(L"-");
	lblPercent.set_text(L"%");
	numScale.border.set(1, backgroundColor.bright(-30));

	minScale = 0.01;
	maxScale = 3200.0;
	numScale.set_range(1., 0, minScale, maxScale);
	slide.set_range(0., 1.);

	int fs = 20;
	btnUp.set_font_size(fs);
	btnDown.set_font_size(fs);

	padding.set(0, 10, 10);
}

void UIMZoom::on_resize(int width, int height) {
	OUI::get_content_area(contentArea);
	int w = contentArea.width;
	int h = contentArea.height;
	int b = h;
	int t = lblTitle.contentArea.bottom() + 10;
	int l = 0;
	int vh, vw;
	int m = 10;

	vh = 20;
	slide.move(l, b - vh, w, vh);

	t = b - vh - m;
	vh = 30;
	vw = 30;
	t = t - vh;

	btnDown.move(l, t, vw, vh);
	btnUp.move(w - vw, t, vw, vh);

	int lblW = 12;
	int txtW = Min(80, (w - 2 * btnUp.boxModel.width - 2 - lblW));
	lblPercent.move((w - lblW - txtW) / 2 + txtW + m, t, lblW, vh);
	numScale.move((w - lblW - txtW) / 2, t, txtW, vh);

	vh = t - m;
	t = lblTitle.contentArea.bottom() + 10;
	vh -= t;
	view.move(l, t, w, vh);

	//t += vh;
	
	/*numScale.move((w - numScale.boxModel.width) >> 1, t);*/
}

void UIMZoom::set_document(Document* document) {
	view.set_document(document);
}

void UIMZoom::process_event(OUI* element, uint32_t message, uint64_t param, bool bubbleUp) {
	if (element == &slide || element == &numScale) {
		double scale = 1.;

		if (element == &slide) {
			auto num = slide.get_value();
			num = pow(num, 5) * (maxScale - minScale) + minScale;
			numScale.set_value(num);
			scale = num / 100.;
		}
		else {
			auto num = atof(numScale.get_number().c_str());
			scale = num / 100.;
			num = pow((num - minScale) / (maxScale - minScale), .2);
			slide.set_value(num);
		}

		ZoomInfo zInfo = view.get_zoom_info();
		zInfo.scale = scale;
		view.set_zoom_info(zInfo);
		parent->process_event(this, UIZOOM_UPDATE, 4, true);
	}
	else if (element == &btnUp || element == &btnDown) {
		parent->process_event(this, UIZOOM_UPDATE, element == &btnUp ? 1 : 2, true);
	}
	else if (element == &view) {
		return parent->process_event(this, UIZOOM_UPDATE, 3, true);
	}
	else 
		UIMenu::process_event(element, message, param, bubbleUp);

	auto c = numScale.text.find(L'.');
	if (c != std::wstring::npos)
		c += 3;
	numScale.text = numScale.text.substr(0, c);
}

ZoomInfo UIMZoom::get_zoom_info() const {
	return view.get_zoom_info();
}

void UIMZoom::set_zoom_info(ZoomInfo zInfo) {
	view.set_zoom_info(zInfo);

	double t = zInfo.scale * 100.;
	numScale.set_value(t);
	
	t = pow((t - minScale) / (maxScale - minScale), .2);
	slide.set_value(t);
}

void UIMZoom::set_scale_range(double minValue, double maxValue) {
	minScale = minValue * 100;
	maxScale = maxValue * 100;
	numScale.set_range(0., 1, minScale, maxScale);
}