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
#include "UIDAddNoise.h"
#include "UIDBulge.h"
#include "UIDCrystalize.h"
#include "UIDDespeckle.h"
#include "UIDGain.h"
#include "UIDGlow.h"
#include "UIDGaussianBlur.h"
#include "UIDMarble.h"
#include "UIDMedian.h"
#include "UIDMotionBlur.h"
#include "UIDOilPaint.h"
#include "UIDOutline.h"
#include "UIDPencilSketch.h"
#include "UIDPixelate.h"
#include "UIDRadialBlur.h"
#include "UIDRandomJitter.h"
#include "UIDRipple.h"
#include "UIDSmartBlur.h"
#include "UIDSmear.h"
#include "UIDSoftPortrait.h"
#include "UIDStamp.h"
#include "UIDSurfaceBlur.h"
#include "UIDSwirl.h"
#include "UIDTileGlass.h"
#include "UIDWater.h"
#include "UIDWave.h"


class UIContainer : public OUI
{
	UISideView sideView;
	UIMainView mainView;
	FileManager fileMgr;
	Document document;
	UIDialog dlgCloseWithoutSave;
	UIDProgress dlgProgress;

	/*
	UIDAddNoise dlgAddNoise;
	UIDBulge dlgBulge;
	UIDCrystalize dlgCrystalize;
	UIDDespeckle dlgDespeckle;
	UIDGain dlgGain;
	UIDGlow dlgGlow;
	UIDGaussianBlur dlgGaussianBlur;
	UIDMarble dlgMarble;
	UIDMedian dlgMedian;
	UIDMotionBlur dlgMotionBlur;
	UIDOilPaint dlgOilPaint;
	UIDOutline dlgOutline;
	UIDPencilSketch dlgPencilSketch;
	UIDPixelate dlgPixelate;
	UIDRadialBlur dlgRadialBlur;
	UIDRandomJitter dlgRandomJitter;
	UIDRipple dlgRipple;
	UIDSmartBlur dlgSmartBlur;
	UIDSmear dlgSmear;
	UIDSoftPortrait dlgSoftPortrait;
	UIDStamp dlgStamp;
	UIDSurfaceBlur dlgSurfaceBlur;
	UIDSwirl dlgSwirl;
	UIDTileGlass dlgTileGlass;
	UIDWater dlgWater;
	UIDWave dlgWave;
	
	UIDContBright dlgContBright;
	UIDChannelMixer dlgChannelMixer;
	UIDColorBalance dlgColorBalance;
	UIDCurves dlgCurves;
	UIDHSL dlgHSL;
	UIDLevels dlgLevels;
	UIDPosterize dlgPosterize;
	UIDThreshold dlgThreshold;*/

	UIDEffect* currEffectDlg;
	std::vector<FileDialogFileType> fileTypes;
	Sheet tempImage;
	bool shouldCopyResult, isRenderingDone;
	int sideBarWidth;

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

