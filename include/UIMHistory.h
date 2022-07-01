#pragma once
#include "UIComMenu.h"
#include "History.h"
#include "Document.h"

#define UIHISTO_UPDATE (Event::_last + 7)

class UIMHistory : public UIComMenu
{
	Document* document;
public:
	void on_init() override;
	void set_document(Document* document);
	void update_history_list();
	void process_event(OUI* element, uint32_t message, uint64_t param, bool bubbleUp) override;
};
