#pragma once
#include "oui.h"
#include "UISideView.h"
#include "UIMainView.h"
#include "ImageTranscoder.h"
#include "History.h"
#include "FileManager.h"
#include <UIDFile.h>
#include <oui_window.h>

class UIContainer : public OUI
{
	UISideView sideView;
	UIMainView mainView;
	FileManager fileMgr;
	Document document;
	UIDialog dlgCloseWithoutSave;
	std::vector<FileDialogFileType> fileTypes;
	int sw;

public:
	void load(std::wstring filePath);
	void save(std::wstring filePath);
	void close_document(bool bForceClose);

	void create_effect_windows();
	void invalidate_document();
	void on_init() override;
	void on_resize(int width, int height) override;
	void process_event(OUI* element, uint32_t message, uint64_t param, bool bubbleUp) override;

	void on_window_closed(UIWindow* window, size_t wmsg) override;
};

