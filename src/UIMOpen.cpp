#include "UIMOpen.h"
#include <oui_uix.h>

void UIMOpen::on_init() {
	boxModel.width = MENU_WIDTH;
	boxModel.height = 70;

	set_title(L"Open");
	UIComMenu::on_init();

	int w = boxModel.width - 8;
	int h = 30;
	itemHeight = h;

	btnOpen.margin.set(4, 4, 0);
	btnClear.margin.set(0, 4, 4);
	btnOpen.padding.set(20, 0);
	btnClear.padding.set(20, 0);
	btnOpen.set_hover_border_color(Colors::lightgray);
	btnClear.set_hover_border_color(Colors::lightgray);

	sp1.padding.set(0, 0, 0, 10);
	sp1.margin.set(4, 0);
	sp2.margin.set(4, 0);

	btnOpen.create(0, 0, w, h, &list);
	sp1.create(0, 0, boxModel.width - 10, 20, &list);
	sp2.create(0, 0, boxModel.width - 10, 8, &list);
	btnClear.create(0, 0, w, h, &list);

	btnOpen.set_text(L"Open");
	sp1.set_text(L"Recent Files");
	btnClear.set_text(L"Clear Recent Files List");

	btnOpen.set_font_size(12);
	sp1.set_font_size(12);
	btnClear.set_font_size(12);

	btnOpen.canvas.art.alignX = Align::LEFT;
	sp1.canvas.art.alignX = Align::LEFT;
	btnClear.canvas.art.alignX = Align::LEFT;

	btnOpen.set_color(Color("#333"));
	sp1.set_color(Color("#333"));
	btnClear.set_color(Color("#333"));

	btnOpen.set_background_color(backgroundColor);
	btnClear.set_background_color(backgroundColor);

	sp1.show_window(false);
	sp2.show_window(false);
	btnClear.show_window(false);
}

void UIMOpen::process_event(OUI* element, uint32_t message, uint64_t param, bool bubbleUp) {

	if (message == Event::Click) {
		if (element == &btnOpen) {
			parent->process_event(this, UIOPEN_UPDATE, 0, true);
		}
		else if (element == &btnClear) {
			parent->process_event(this, UIOPEN_UPDATE, 1, true);
		}
		else {
			// this must be one of the recent items
			for (auto elem : recentItems)
				if (elem == element) {
					selectedRecentFile = elem->fileInfo;
					parent->process_event(this, UIOPEN_UPDATE, 2, true);
				}
		}
	}

	UIComMenu::process_event(element, message, param, bubbleUp);
}

const File* UIMOpen::get_selected_recent_file() const {
	return selectedRecentFile;
}

void UIMOpen::populate_recent_files_list(FileManager* fileMgr) {

	// clear the list
	for (auto elem : recentItems) {
		uix->delete_element(elem);
	}
	recentItems.clear();

	if (fileMgr) {
		const auto& filesInfo = fileMgr->get_recent_files();
		for (auto& fInfo : filesInfo) {
			auto newItem = new UIButtonWithBorderOnHoverWithFileInfo();
			newItem->fileInfo = &fInfo;
			recentItems.push_back(newItem);
			newItem->create(0, 0, boxModel.width - 8, itemHeight, &list);
			newItem->margin.set(0, 4, 0);
			newItem->padding.set(20, 0);
			newItem->set_hover_border_color(Colors::lightgray);
			newItem->set_text(fInfo.name);
			newItem->set_font_size(13);
			newItem->canvas.art.alignX = Align::LEFT;
			newItem->set_color(Color("#333"));
			newItem->set_background_color(backgroundColor);
		}
	}

	auto& elemList = list.elements;
	elemList.clear();
	elemList.push_back(&btnOpen);

	bool show = false;
	if (recentItems.size() > 0) {
		elemList.push_back(&sp1);
		for (auto item : recentItems)
			elemList.push_back(item);
		elemList.push_back(&sp2);
		elemList.push_back(&btnClear);
		show = true;
	}

	sp1.show_window(show);
	sp2.show_window(show);
	btnClear.show_window(show);
	list.reset_size();

	int h = 0;
	list.measure_content(0, &h);
	h += lblTitle.boxModel.height + btnClear.margin.bottom;
	move(boxModel.left, boxModel.top, boxModel.width, h);
}