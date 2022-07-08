#include "container.h"
#include <oui_uix.h>
#include <chrono>
#include "UIDOpenFile.h"
#include "UIDSaveFile.h"

UIDEffect* dlgEffect;

void UIContainer::create_effect_windows() {
	/*

	dlgContBright.config(3, 3);

	dlgContBright.create(this);
	dlgChannelMixer.create(this);
	dlgColorBalance.create(this);
	dlgCurves.create(this);
	dlgHSL.create(this);
	dlgLevels.create(this);
	dlgPosterize.create(this);
	dlgThreshold.create(this);*/

	/*dlgContBright.set_document(&document);
	dlgChannelMixer.set_document(&document);
	dlgColorBalance.set_document(&document);
	dlgCurves.set_document(&document);
	dlgHSL.set_document(&document);
	dlgLevels.set_document(&document);
	dlgPosterize.set_document(&document);
	dlgThreshold.set_document(&document);
	
	dlgAddNoise.create(this);
	dlgBulge.create(this);
	dlgCrystalize.create(this);
	dlgDespeckle.create(this);
	dlgGain.create(this);
	dlgGlow.create(this);
	dlgGussianBlur.create(this);
	dlgMarble.create(this);
	dlgMedian.create(this);
	dlgMotionBlur.create(this);
	dlgOldPaint.create(this);
	dlgOutline.create(this);
	dlgPencilSketch.create(this);
	dlgPixelate.create(this);
	dlgRadialBlur.create(this);
	dlgRandomJitter.create(this);
	dlgRipple.create(this);
	dlgSmartBlur.create(this);
	dlgSmear.create(this);
	dlgSoftPortrait.create(this);
	dlgStamp.create(this);
	dlgSurfaceBlur.create(this);
	dlgSwirl.create(this);
	dlgTileGlass.create(this);
	dlgWater.create(this);
	dlgWave.create(this);

	dlgAddNoise.set_document(&doc);
	dlgBulge.set_document(&doc);
	dlgCrystalize.set_document(&doc);
	dlgDespeckle.set_document(&doc);
	dlgGain.set_document(&doc);
	dlgGlow.set_document(&doc);
	dlgGussianBlur.set_document(&doc);
	dlgMarble.set_document(&doc);
	dlgMedian.set_document(&doc);
	dlgMotionBlur.set_document(&doc);
	dlgOldPaint.set_document(&doc);
	dlgOutline.set_document(&doc);
	dlgPencilSketch.set_document(&doc);
	dlgPixelate.set_document(&doc);
	dlgRadialBlur.set_document(&doc);
	dlgRandomJitter.set_document(&doc);
	dlgRipple.set_document(&doc);
	dlgSmartBlur.set_document(&doc);
	dlgSmear.set_document(&doc);
	dlgSoftPortrait.set_document(&doc);
	dlgStamp.set_document(&doc);
	dlgSurfaceBlur.set_document(&doc);
	dlgSwirl.set_document(&doc);
	dlgTileGlass.set_document(&doc);
	dlgWater.set_document(&doc);
	dlgWave.set_document(&doc);
	*/

	dlgProgress.create(300, 20, this);
	dlgProgress.set_progress(0.);

	dlgEffect = new UIDSurfaceBlur();

	dlgEffect->config(3, 3);
	dlgEffect->create(this);
	dlgEffect->set_document(&document);
	show_effect(dlgEffect);

	//uix->show_box_model();
}

void UIContainer::show_effect(UIDEffect* dlgEffect) {
	if (dlgEffect) {
		currEffectDlg = dlgEffect;
		dlgProgress.set_title(dlgEffect->title);
		dlgProgress.move(0, 0);
		mainView.show_frame();
		dlgEffect->show();
	}
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
	OUITheme::secondary.set(Color("#e9e9e9"));
	OUITheme::border.set(Color("#aaa"));
	OUITheme::text.set(Color("#444"));

	bCopyResult = false;
	bApplyThreadRunning = false;
	progressBlockCount = 100;
	progress = 0.;
	currEffectDlg = NULL;
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

	if (element == dlgEffect) {
		if (message == Event::Click) {
			mainView.show_frame(param);
		}
		mainView.invalidate();
	}
	else if (message == UIDEFFECT_UPDATE) {
		mainView.invalidate();
	}
	else if (message == UIOPEN_UPDATE) {
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
		dlgEffect->show();

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
		else if (message == UIHISTO_UPDATE && !bApplyThreadRunning) {
			auto low = param & 15;
			auto high = param >> 4;
			if (low == 1) document.undo();
			else if (low == 2) document.redo();
			else document.checkout((size_t)high);

			bool bDocumentNeedsInvalidation = document.is_invalidated();
			if (bDocumentNeedsInvalidation) {
				update_image_dependant_elements();
				invalidate_document();
			}

			invalidate();
		}
	}
}

void UIContainer::update_history_list() {
	sideView.mHisto->update_history_list();
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

	update_history_list();
	update_image_dependant_elements();
}

void UIContainer::update_image_dependant_elements() {
	if (dlgEffect) dlgEffect->reset_image();

	/*dlgContBright.reset_image();
	dlgChannelMixer.reset_image();
	dlgColorBalance.reset_image();
	dlgCurves.reset_image();
	dlgHSL.reset_image();
	dlgLevels.reset_image();
	dlgPosterize.reset_image();
	dlgThreshold.reset_image();*/
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
	else if (window == currEffectDlg) {
		if (wmsg == DialogButtonId::OK) {
			progress = 0.;
			bCopyResult = true;
			/*bApplyThreadRunning = true;
			std::thread thread(&UIContainer::apply_thread, this);
			thread.detach();
			tempImage.free();
			dlgProgress.show_window();*/
			set_timer(1, 10);
		}
		else if (wmsg == DialogButtonId::Cancel) {
			document.reset_frame();
		}
		mainView.show_frame();
	}
}

void UIContainer::on_timer(uint32_t nTimer) {
	if (bUpdateProgress) {
		dlgProgress.set_progress(progress);
		bUpdateProgress = false;
	}

	if (bApplyThreadRunning == false || bCopyResult) {
		kill_timer(1);
		dlgProgress.show_window(false);
	}

	if (bCopyResult) {
		bCopyResult = false;
		copy_result();
	}
}

void UIContainer::copy_result() {
	auto dstImage = document.snap_shot(currEffectDlg->title);
	if (dstImage)
		dstImage->copy(document.get_frame());
	document.reset_frame();

	dlgProgress.show_window(false);
	update_history_list();
	update_image_dependant_elements();
	sideView.config_elements_based_on_document_status();
	invalidate();
}

void UIContainer::invalidate_document() {
	document.invalidate();
	sideView.mSave->enable_element(0, document.is_open());
}

void UIContainer::apply_thread() {
	if (currEffectDlg == NULL) return;

	auto srcImage = document.get_image();
	auto dstImage = &tempImage;

	dstImage->clone(srcImage);

	using namespace std::chrono_literals;
	size_t verticalBlockIndex, horizontalBlockIndex;
	int blockLeft, blockTop, blockRight, blockBottom, blockHeight;

	blockHeight = int(ceil((double)srcImage->h / (double)progressBlockCount));
	blockTop = 0;

	// entire line
	blockLeft = 0;
	blockRight = srcImage->w;
	auto progressBlockCount1 = progressBlockCount - 1;
	bool wasSuccessful = true;

	for (verticalBlockIndex = 0; verticalBlockIndex < progressBlockCount; ++verticalBlockIndex, blockTop += blockHeight) {
		blockBottom = blockTop + blockHeight;
		if (verticalBlockIndex == progressBlockCount1)
			blockBottom = srcImage->h;

		auto res = currEffectDlg->render(srcImage, dstImage, blockLeft, blockTop, blockRight, blockBottom);
		if (res != IMAGE_EFFECT_RESULT_OK) {
			if (res == IMAGE_EFFECT_RESULT_ERROR) wasSuccessful = false;
			break;
		}
		
		progress = (double)verticalBlockIndex / (double)(progressBlockCount1);
		bUpdateProgress = true;
	}

	bCopyResult = wasSuccessful;
	bApplyThreadRunning = false;
}