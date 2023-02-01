#include "UISideView.h"
#include <oui_uix.h>

UISideView::UISideView()
	: document(0) {
	mode = UIStackMode::STACKVER;
}

void UISideView::set_document(Document* document) {
	this->document = document;
	mZoom->set_document(document);
	mHisto->set_document(document);
	if (document)
		mOpen->populate_recent_files_list(document->get_file_manager());
}

void UISideView::config_elements_based_on_document_status() {
	bool isDocAvail = document != NULL;
	bool isDocOpen = isDocAvail && document->is_open();
	btnOpen.enable(isDocAvail);
	btnSave.enable(isDocOpen);
	mSave->enable_element(0, isDocOpen && document->is_invalidated());
	btnClose.enable(isDocOpen);
	btnAdj.enable(isDocOpen);
	btnFilter.enable(isDocOpen);
	btnUndo.enable(isDocOpen);
	btnRedo.enable(isDocOpen);
	btnHistory.enable(isDocOpen);
	btnZoom.enable(isDocOpen);
	btnInfo.enable(true);
	invalidate();
}

void UISideView::on_init() {

	int l, t, bh, w, lh, lrMargin;
	l = 0;
	t = 7;
	bh = 30;
	lh = 5;
	lrMargin = 7;
	padding.set(t, 0, 0, 0);

	Spacing margin; 
	margin.set(lrMargin, 0);

	w = boxModel.width - margin.left - margin.right;

	btnOpen.create(this);
	btnSave.create(this);
	btnClose.create(this);
	sp1.create(this);
	btnAdj.create(this);
	btnFilter.create(this);
	sp2.create(this);
	btnUndo.create(this);
	btnRedo.create(this);
	btnHistory.create(this);
	sp3.create(this);
	btnZoom.create(this);
	btnInfo.create(this);

	UIButtonWithBorderOnHover* btns[] = {
		&btnOpen, &btnSave, &btnClose, &btnAdj, &btnFilter,
		&btnUndo, &btnRedo, &btnHistory, &btnZoom, &btnInfo
	};

	Color borderColor(Colors::lightgray);
	
	iterateI(10) {
		auto btn = btns[i];
		btn->margin = margin;
		btn->set_background_color(backgroundColor);
		btn->set_hover_border_color(borderColor);
		btn->padding.set(5);
	}

	OUI* sps[] = { &sp1, &sp2, &sp3 };
	iterateI(3) {
		auto sp = sps[i];
		sp->margin = margin;
		sp->set_background_color(backgroundColor);
		sp->set_color(borderColor);
	}

	reset_size();

	//uix->show_magnifier(true, 200, 200, 10);
	//uix->show_box_model();

	Color rightBorderColor = backgroundColor.bright(-30);
	border.set(0, 1, 0, 0,
		rightBorderColor, rightBorderColor,
		rightBorderColor, rightBorderColor);

	btnOpen.set_text(LR"(
		<svg viewBox="0 0 512 512" fill="currentColor">
			<path d="M490.053,118.398H259.926V77.852c0-12.121-9.826-21.947-21.947-21.947H71.731c-12.121,0-21.947,9.826-21.947,21.947
				v40.546H21.947C9.826,118.398,0,128.224,0,140.345v293.804c0,12.121,9.826,21.947,21.947,21.947h468.106
				c12.121,0,21.947-9.826,21.947-21.947V140.345C512,128.224,502.174,118.398,490.053,118.398z M93.678,99.799h122.354v18.599
				H93.678V99.799z M468.106,412.201H43.894v-249.91h424.212V412.201z"/>
		</svg>
	)");
	btnSave.set_text(LR"(
		<svg width="256" height="256" viewBox="0 0 256 256" fill="currentColor">
			<path d="M216 91.314V208a8 8 0 0 1-8 8l-32 .008v-64a8 8 0 0 0-8-8H88a8 8 0 0 0-8 8v64L48 216a8 8 0 0 1-8-8V48a8 8 0 0 1 8-8h116.686a8 8 0 0 1 5.657 2.343l43.314 43.314A8 8 0 0 1 216 91.314Z"/>
			<path d="m219.314 80-43.313-43.312A15.89 15.89 0 0 0 164.687 32H48a16.018 16.018 0 0 0-16 16v160a16.018 16.018 0 0 0 16 16h31.826c.059 0 .115.008.174.008s.115-.007.174-.008h95.652c.059 0 .115.008.174.008s.115-.007.174-.008H208a16.018 16.018 0 0 0 16-16V91.314A15.898 15.898 0 0 0 219.314 80ZM168 208H88v-55.992h80Zm40-116.686V208h-24v-55.992a16.018 16.018 0 0 0-16-16H88a16.018 16.018 0 0 0-16 16V208H48V48h116.686L208 91.315h8ZM160 72.01a8 8 0 0 1-8 8H96a8 8 0 0 1 0-16h56a8 8 0 0 1 8 8Z"/>
		</svg>
	)");
	btnClose.set_text(LR"(
		<svg viewBox="0 0 252 252" fill="currentColor">
			<path d="M126 0C56.523 0 0 56.523 0 126s56.523 126 126 126 126-56.523 126-126S195.477 0 126 0zm0 234c-59.551 0-108-48.449-108-108S66.449 18 126 18s108 48.449 108 108-48.449 108-108 108z"/>
			<path d="M164.612 87.388a9 9 0 0 0-12.728 0L126 113.272l-25.885-25.885a9 9 0 0 0-12.728 0 9 9 0 0 0 0 12.728L113.272 126l-25.885 25.885a9 9 0 0 0 6.364 15.364 8.975 8.975 0 0 0 6.364-2.636L126 138.728l25.885 25.885c1.757 1.757 4.061 2.636 6.364 2.636s4.606-.879 6.364-2.636a9 9 0 0 0 0-12.728L138.728 126l25.885-25.885a9 9 0 0 0-.001-12.727z"/>
		</svg>
	)");
	btnAdj.set_text(LR"(
		<svg width="32" height="32" viewBox="0 0 32 32">
			<path fill="currentColor" d="M8 9.142V4H6v5.142c-1.72.447-3 2-3 3.858s1.28 3.411 3 3.858v10.096h2V16.858c1.72-.447 3-2 3-3.858S9.72 9.589 8 9.142zM7 15a2 2 0 1 1-.001-3.999A2 2 0 0 1 7 15zm10 1.142V4h-2v12.142c-1.72.447-3 2-3 3.858s1.28 3.411 3 3.858v3.096h2v-3.096c1.72-.447 3-2 3-3.858s-1.28-3.411-3-3.858zM16 22a2 2 0 1 1-.001-3.999A2 2 0 0 1 16 22zm13-6c0-1.858-1.28-3.411-3-3.858V4h-2v8.142c-1.72.447-3 2-3 3.858s1.28 3.411 3 3.858v7.096h2v-7.096c1.72-.447 3-2 3-3.858zm-4 2a2 2 0 1 1-.001-3.999A2 2 0 0 1 25 18z"/>
		</svg>
	)");
	btnFilter.set_text(LR"(
		<svg viewBox="0 0 32 32" stroke="currentColor" fill="none" stroke-width="2" stroke-linecap="round" stroke-linejoin="round" stroke-miterlimit="10"  > 
			<circle class="st0" cx="16" cy="16" r="4"/>
			<path class="st0" d="M12.5,9.9c0.3-1.4,1-2.7,2.1-3.8C16.7,4,19.6,3.3,22.3,4"/>
			<path class="st0" d="M9.2,14.2C8.4,13,8,11.5,8,10c0-3,1.6-5.5,4-6.9"/>
			<path class="st0" d="M9.9,19.5c-1.4-0.3-2.7-1-3.8-2.1C4,15.3,3.3,12.4,4,9.7"/>
			<path class="st0" d="M14.2,22.8C13,23.6,11.5,24,10,24c-3,0-5.5-1.6-6.9-4"/>
			<path class="st0" d="M19.5,22.1c-0.3,1.4-1,2.7-2.1,3.8c-2.1,2.1-5.1,2.8-7.7,2.1"/>
			<path class="st0" d="M22.8,17.8C23.6,19,24,20.5,24,22c0,3-1.6,5.5-4,6.9"/>
			<path class="st0" d="M22.1,12.5c1.4,0.3,2.7,1,3.8,2.1c2.1,2.1,2.8,5.1,2.1,7.7"/>
			<path class="st0" d="M17.8,9.2C19,8.4,20.5,8,22,8c3,0,5.5,1.6,6.9,4"/>
		</svg>
	)");
	btnUndo.set_text(LR"(
		<svg viewBox="-2 -2 36 36" stroke="currentColor" fill="none" stroke-width="2" stroke-miterlimit="10">
			<path d="M5.4 14H21c3.3 0 6 2.7 6 6v7"/>
			<path d="m13.5 6-8 8 8 8"/>
		</svg>
	)");
	btnRedo.set_text(LR"(
		<svg viewBox="-2 -2 36 36" stroke="currentColor" fill="none" stroke-width="2" stroke-miterlimit="10">
			<path d="M26.6 18H11c-3.3 0-6-2.7-6-6V5"/>
			<path d="m18.5 26 8-8-8-8"/>
		</svg>
	)");
	btnHistory.set_text(LR"(
		<svg width="18" height="18" viewBox="0 0 18 18" fill="currentColor">
			<path d="M17 10a8 8 0 0 1-16 0 1 1 0 0 1 2 0 6 6 0 1 0 6.5-5.98V5.5a.477.477 0 0 1-.27.44A.466.466 0 0 1 9 6a.52.52 0 0 1-.29-.09l-3.5-2.5a.505.505 0 0 1 0-.82l3.5-2.5a.488.488 0 0 1 .52-.03.477.477 0 0 1 .27.44v1.52A7.987 7.987 0 0 1 17 10z"/>
			<path d="M11.71 8.71 10 10.42V13a1 1 0 0 1-2 0v-3a.69.69 0 0 1 .04-.25.37.37 0 0 1 .04-.14.988.988 0 0 1 .21-.32l2-2a1.004 1.004 0 0 1 1.42 1.42z"/>
		</svg>
	)");
	btnZoom.set_text(LR"(
		<svg viewBox="0 0 310.42 310.42" transform="scale(-1,1) " fill="currentColor">
			<path d="M273.587 214.965c49.11-49.111 49.109-129.021 0-178.132-49.111-49.111-129.02-49.111-178.13 0C53.793 78.497 47.483 140.462 76.51 188.85c0 0 2.085 3.498-.731 6.312l-64.263 64.263c-12.791 12.79-15.836 30.675-4.493 42.02l1.953 1.951c11.343 11.345 29.229 8.301 42.019-4.49l64.128-64.128c2.951-2.951 6.448-.866 6.448-.866 48.387 29.026 110.352 22.717 152.016-18.947zM118.711 191.71c-36.288-36.288-36.287-95.332.001-131.62 36.288-36.287 95.332-36.288 131.619 0 36.288 36.287 36.288 95.332 0 131.62-36.288 36.286-95.331 36.286-131.62 0z"/>
			<path d="M126.75 118.424c-1.689 0-3.406-.332-5.061-1.031-6.611-2.798-9.704-10.426-6.906-17.038 17.586-41.559 65.703-61.062 107.261-43.476 6.611 2.798 9.704 10.426 6.906 17.038-2.799 6.612-10.425 9.703-17.039 6.906-28.354-11.998-61.186 1.309-73.183 29.663-2.099 4.959-6.913 7.938-11.978 7.938z"/>
		</svg>
	)");
	btnInfo.set_text(LR"(
		<svg x="0px" y="0px" viewBox="0 0 32 32" fill="currentColor">
			<path d="M10,16c1.105,0,2,0.895,2,2v8c0,1.105-0.895,2-2,2H8v4h16v-4h-1.992c-1.102,0-2-0.895-2-2L20,12H8v4H10z"/>
			<circle cx="16" cy="4" r="4"/>
		</svg>
	)");

	Color color;
	color.set(Colors::gray);
	btnOpen.set_color(color);
	btnSave.set_color(color);
	btnClose.set_color(color);
	btnAdj.set_color(color);
	btnFilter.set_color(color);
	btnUndo.set_color(color);
	btnRedo.set_color(color);
	btnHistory.set_color(color);
	btnZoom.set_color(color);
	btnInfo.set_color(color);

	btnOpen.set_menu(mOpen = new UIMOpen());
	btnSave.set_menu(mSave = new UIMSave());
	btnAdj.set_menu(mAdj = new UIMAdjust());
	btnFilter.set_menu(mFilter = new UIMFilter());
	btnHistory.set_menu(mHisto = new UIMHistory());
	btnZoom.set_menu(mZoom = new UIMZoom());
}

void UISideView::process_event(OUI* element, uint32_t message, uint64_t param, bool bubbleUp) {
	if (message == UIOPEN_UPDATE) {
		parent->process_event(this, UIOPEN_UPDATE, param, true);
	}
	else if (message == UISAVE_UPDATE) {
		parent->process_event(this, UISAVE_UPDATE, param, true);
	}
	else if (message == UIHISTO_UPDATE) {
		parent->process_event(this, UIHISTO_UPDATE, param << 4, true);
	}
	else if (message == UIZOOM_UPDATE) {
		parent->process_event(this, UIZOOM_UPDATE, param, true);
	}
	else if (message == Event::Click) {
		if (element == &btnUndo) {
			parent->process_event(this, UIHISTO_UPDATE, 1, true);
		}
		else if (element == &btnRedo) {
			parent->process_event(this, UIHISTO_UPDATE, 2, true);
		}
		else if (element == &btnClose) {
			parent->process_event(this, UISIDE_CLOSE_DOCUMENT, 0, true);
		}
		else if (element == &btnInfo) {

		}
	} else if (message == Event::Update) {
		
	}
}

void UISideView::on_resize(int width, int height) {
	int l, t, w, bh, lh, lrMargin;
	l = 0;
	t = 7;
	bh = 30;
	lh = 5;
	lrMargin = 7;
	 
	Spacing margin;
	margin.set(lrMargin, 0);
	w = boxModel.width - margin.left - margin.right;

	btnOpen.move(l, t, w, bh);
	btnSave.move(l, t, w, bh);
	btnClose.move(l, t, w, bh);
	sp1.move(l, t, w, lh);
	btnAdj.move(l, t, w, bh);
	btnFilter.move(l, t, w, bh);
	sp2.move(l, t, w, lh);
	btnUndo.move(l, t, w, bh);
	btnRedo.move(l, t, w, bh);
	btnHistory.move(l, t, w, bh);
	sp3.move(l, t, w, lh);
	btnZoom.move(l, t, w, bh);
	btnInfo.move(l, t, w, bh);

	UIStack::on_resize(width, height);
}