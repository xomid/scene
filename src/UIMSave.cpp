#include "UIMSave.h"

void UIMSave::on_init() {
	boxModel.width = MENU_WIDTH;
	boxModel.height = 100;

	set_title(L"Save");
	UIComMenu::on_init();

	int w = boxModel.width - 8;
	int h = 30;

	btnSave.margin.set(4, 4, 0);
	btnSaveAs.margin.set(0, 4, 4);
	btnSave.padding.set(20, 0);
	btnSaveAs.padding.set(20, 0);
	btnSave.set_hover_border_color(Colors::lightgray);
	btnSaveAs.set_hover_border_color(Colors::lightgray);

	btnSave.create(0, 0, w, h, &list);
	btnSaveAs.create(0, 0, w, h, &list);
	btnSave.set_text(L"Save");
	btnSaveAs.set_text(L"Save As ...");

	btnSave.set_font_size(13);
	btnSaveAs.set_font_size(12);

	btnSave.canvas.art.alignX = Align::LEFT;
	btnSaveAs.canvas.art.alignX = Align::LEFT;

	btnSave.set_color(Color("#333"));
	btnSaveAs.set_color(Color("#333"));

	btnSave.set_background_color(backgroundColor);
	btnSaveAs.set_background_color(backgroundColor);
}

void UIMSave::process_event(OUI* element, uint32_t message, uint64_t param, bool bubbleUp) {
	if (message == Event::Click) {
		if (element == &btnSave) 
			parent->process_event(this, UISAVE_UPDATE, 0, true);
		else if (element == &btnSaveAs)
			parent->process_event(this, UISAVE_UPDATE, 1, true);
	}
}

void UIMSave::enable_element(uint32_t index, bool enable) {
	switch (index) {
	case 0:
		btnSave.enable(enable);
		break;
	default:
		btnSaveAs.enable(enable);
		break;
	}
}