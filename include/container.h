#pragma once
#include "oui.h"
#include "UISideView.h"
#include "UIMainView.h"
#include "ImageTranscoder.h"
#include "History.h"
#include "FileManager.h"
#include <UIDFile.h>
#include "UIDProgress.h"

#include "UIDContBright.h"
#include "UIDChannelMixer.h"
#include "UIDColorBalance.h"
#include "UIDCurves.h"
#include "UIDHSL.h"
#include "UIDLevels.h"
#include "UIDPosterize.h"
#include "UIDThreshold.h"


class UIContainer : public OUI
{
	UISideView sideView;
	UIMainView mainView;
	FileManager fileMgr;
	Document document;
	UIDialog dlgCloseWithoutSave;
	UIDProgress dlgProgress;
	UIDContBright dlgContBright;
	UIDChannelMixer dlgChannelMixer;
	UIDColorBalance dlgColorBalance;
	UIDCurves dlgCurves;
	UIDHSL dlgHSL;
	UIDLevels dlgLevels;
	UIDPosterize dlgPosterize;
	UIDThreshold dlgThreshold;
	UIDEffect* currEffectDlg;

	std::vector<FileDialogFileType> fileTypes;
	int sw;
	Sheet tempImage;
	double progress;
	size_t progressBlockCount;
	bool bUpdateProgress, bApplyThreadRunning, bCopyResult;

	void apply_thread();
	void copy_result();

public:
	void load(std::wstring filePath);
	void save(std::wstring filePath);
	void close_document(bool bForceClose);
	void show_effect(UIDEffect* dlgEffect);
	void update_history_list();
	void update_image_dependant_elements();

	void create_effect_windows();
	void invalidate_document();
	void on_init() override;
	void on_resize(int width, int height) override;
	void process_event(OUI* element, uint32_t message, uint64_t param, bool bubbleUp) override;
	void on_timer(uint32_t nTimer) override;

	void on_window_closed(UIWindow* window, size_t wmsg) override;
};

