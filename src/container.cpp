#include "container.h"
#include <oui_uix.h>
#include "UIDOpenFile.h"
#include "UIDSaveFile.h"
#include "UIDContBright.h"

UIDContBright dlgContBright;

void UIContainer::create_effect_windows() {
	dlgContBright.create(this);
	dlgContBright.set_document(&document);
	dlgContBright.show();
}

void UIContainer::on_init()
{
	fileTypes = {
		{ L"JPEG", L"*.jpg;*.jpeg" },
		{ L"PNG", L"*.png" },
		{ L"BMP", L"*.bmp" },
		{ L"GIF", L"*.gif" },
		{ L"All", L"*.*" },
	};

	OUITheme::primary.set(Color("#eee"));
	OUITheme::secondary.set(Color("#ddd"));
	OUITheme::border.set(Color("#aaa"));
	OUITheme::text.set(Color("#444"));

	sw = 45;
	int w = boxModel.width;
	int h = boxModel.height;
	sideView.create(0, 0, sw, h, this);
	mainView.create(sw, 0, w - sw, h, this);
	mainView.set_background_color(OUITheme::primary.bright(-20));
	dlgCloseWithoutSave.create(300, 100, this, DialogButtonSet::Yes_No);
	dlgCloseWithoutSave.set_title(L"Unsaved Changes");
	dlgCloseWithoutSave.set_text(L"Foo");

	fileMgr.load_db(L"C:\\dev\\scene");
	document.set_file_manager(&fileMgr);
	mainView.set_document(&document);
	sideView.set_document(&document);
	sideView.config_elements_based_on_document_status();

	load(L"C:\\Users\\Omid\\Pictures\\6.jpg");

	create_effect_windows();
	uix->apply_theme_all();
}

void UIContainer::process_event(OUI* element, uint32_t message, uint64_t param, bool bubbleUp) {

	if (element == &dlgContBright) {
		if (message == Event::Click) {
			mainView.show_frame(param);
		}
		mainView.invalidate();
	}


	if (message == UIOPEN_UPDATE) {
		if (param == 0) {
			UIDOpenFile dlgOpen;
			dlgOpen.set_type_index(1);
			dlgOpen.set_file_types(fileTypes);
			if (dlgOpen.show_window(this) == 0) {
				auto fileNames = dlgOpen.get_selected_file_names();
				if (fileNames.size()) {
					load(fileNames[0]);
					sideView.mOpen->populate_recent_files_list(&fileMgr);
				}
			}
		}
		else if (param == 1) {
			fileMgr.clear_recent_file_list();
			sideView.mOpen->populate_recent_files_list(&fileMgr);
		}
		else if (param == 2) {
			auto selectedFileInfo = sideView.mOpen->get_selected_recent_file();
			if (selectedFileInfo) {
				load(selectedFileInfo->path);
			}
		}
	}
	else if (message == UISAVE_UPDATE) {

		std::wstring filePath = L"";
		if (param == 0) {
			filePath = document.get_alternative_file_path();
		}
		else if (param == 1) {
			UIDSaveFile dlgSave;
			dlgSave.set_default_dir(L"C:\\Users\\Omid\\Pictures");
			dlgSave.set_type_index(1);
			dlgSave.set_file_types(fileTypes);
			if (dlgSave.show_window(this) == 0) {
				auto fileNames = dlgSave.get_selected_file_names();
				if (fileNames.size()) {
					filePath = fileNames[0];
				}
			}
		}

		save(filePath);
	}
	else if (message == UISIDE_CLOSE_DOCUMENT) {

		dlgContBright.show();

		//if (document.is_invalidated())
		//	dlgCloseWithoutSave.show_window();
		//else close_document(false);// this is guraranteed to close it because it is not invalidated i.e saved
	}
	else if (element == &mainView) {
		auto zInfo = mainView.get_zoom_info();
		sideView.mZoom->set_zoom_info(zInfo);
	}
	else if (element == &sideView) {
		if (message == UIZOOM_UPDATE) // zoom messages
		{
			if (param == 3) {
				auto zInfo = sideView.mZoom->get_zoom_info();
				mainView.set_zoom_info(zInfo);
			}
			else if (param == 1 || param == 2) {
				// zoom up
				mainView.zoom_dir(param == 1);
				auto zInfo = mainView.get_zoom_info();
				sideView.mZoom->set_zoom_info(zInfo);
			}
			else if (param == 4) {
				// zoom up
				mainView.zoom_rel(sideView.mZoom->get_zoom_info().scale);
				auto zInfo = mainView.get_zoom_info();
				sideView.mZoom->set_zoom_info(zInfo);
			}
		}
		else if (message == UIHISTO_UPDATE) {
			auto low = param & 15;
			auto high = param >> 4;
			if (low == 1) document.undo();
			else if (low == 2) document.redo();
			else document.checkout((size_t)high);

			bool bDocumentNeedsInvalidation = document.is_invalidated();
			if (bDocumentNeedsInvalidation)
				invalidate_document();

			invalidate();
		}
	}
}


void UIContainer::on_resize(int width, int height) {
	int w = boxModel.width;
	int h = boxModel.height;
	sideView.move(0, 0, sw, h);
	mainView.move(sw, 0, w - sw, h);
	sideView.reset_size();
}

void UIContainer::load(std::wstring filePath) {
	document.load(filePath);
	mainView.show_image(true);
	mainView.reset_view();
	sideView.config_elements_based_on_document_status();

	auto zInfo = mainView.get_zoom_info();
	sideView.mZoom->set_scale_range(mainView.get_min_scale(), mainView.get_max_scale());
	sideView.mZoom->set_zoom_info(zInfo);
	
	sideView.mHisto->update_history_list();
}

void UIContainer::save(std::wstring filePath) {
	document.save(filePath);
	sideView.config_elements_based_on_document_status();
}

void UIContainer::close_document(bool bForceClose) {
	if (document.close(bForceClose)) return;
	mainView.show_image(false);
	sideView.config_elements_based_on_document_status();
}

void UIContainer::on_window_closed(UIWindow* window, size_t wmsg) {
	if (window == &dlgCloseWithoutSave) {
		if (wmsg == DialogButtonId::Yes) {
			save(document.get_alternative_file_path());
			close_document(false);
		}
		else if (wmsg == DialogButtonId::No) {
			close_document(true);
		}
	}
}

void UIContainer::invalidate_document() {
	document.invalidate();
	sideView.mSave->enable_element(0, document.is_open());
}