#include "UIDCommon.h"

void UIDCommon::on_init() {
	chkPreview.create(10, 10, 50, 20, this);
	chkPreview.set_text(L"Preview");
	chkPreview.select(true);
	padding.set(0, 0, 30, 0);
}

void UIDCommon::on_resize(int width, int height) {
	chkPreview.move(contentArea.left, contentArea.height - chkPreview.boxModel.height);
}

void UIDCommon::process_event(OUI* element, uint32_t message, uint64_t param, bool bubbleUp) {
	if (element == &chkPreview && (message == Event::Select || message == Event::Deselect)) {
		chkPreview.select(message == Event::Select);
		message = Event::Update;
		param = 0;
		element = this;
	}

	if (parent) 
		parent->process_event(element, message, param, true);
}
