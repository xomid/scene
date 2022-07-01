#include "UIMHistory.h"
#include "UIButtonWithThumbnail.h"

void UIMHistory::on_init() {
	boxModel.width = MENU_WIDTH;
	boxModel.height = MENU_HEIGHT;

	set_title(L"History");
	UIComMenu::on_init();
}

void UIMHistory::set_document(Document* document) {
	this->document = document;
}

void UIMHistory::update_history_list() {
	list.elements.clear();
	if (!document) return;

	int w = list.boxModel.width - 10;
	int h = 50;
	bool bFirst = true;
	Spacing padding;

	padding.set(4);
	auto& history = *document->get_history();
	for (auto& item : history) {
		auto btn = new UIButtonWithThumbnail();
		if (bFirst) {
			bFirst = false;
			btn->create(0, 0, w, h * 1.5, &list);
			padding.left = 5;
		}
		else {
			btn->create(0, 0, w, h, &list);
			padding.left = 20;
		}

		auto img = item->image;
		btn->set_text(item->title);
		btn->set_thumbnail(img->data, img->w, img->h, img->pitch, img->nbpp);
		btn->transform(false);
		btn->set_thumb_padding(padding);
	}
}

void UIMHistory::process_event(OUI* element, uint32_t message, uint64_t param, bool bubbleUp) {
	if (message == Event::Click) {
		iterateI(list.elements.size()) {
			auto elem = list.elements[i];
			if (element == elem) {
				if (parent)
					parent->process_event(this, UIHISTO_UPDATE, i, true);
				break;
			}
		}
	}
}