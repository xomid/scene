#pragma once
#include <oui_stack.h>
#include "UISeperator.h"
#include "UIButtonWithMenu.h"
#include "UIMOpen.h"
#include "UIMSave.h"
#include "UIMAdjust.h"
#include "UIMFilter.h"
#include "UIMHistory.h"
#include "UIMZoom.h"
#include "Document.h"

#define UISIDE_CLOSE_DOCUMENT (Event::_last + 3)

class UISideView : public UIStack
{
public:
	UISeperator sp1, sp2, sp3;
	UIButtonWithBorderOnHover btnClose, btnRedo, btnUndo, btnInfo;
	UIButtonWithMenu btnOpen, btnSave, btnAdj, btnFilter, btnHistory, btnZoom;
	UIMOpen* mOpen;
	UIMSave* mSave;
	UIMAdjust* mAdj;
	UIMFilter* mFilter;
	UIMHistory* mHisto;
	UIMZoom* mZoom;
	Document* document;

public:
	UISideView();
	void config_elements_based_on_document_status();
	void set_document(Document* document);
	void on_init() override;
	void on_update() override;
	void process_event(OUI* element, uint32_t message,
		uint64_t param, bool bubbleUp = true) override;
};
