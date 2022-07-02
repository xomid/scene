#pragma once
#include <oui_window.h>
#include <oui_check.h>
#include "Document.h"
#include "ImageEffect.h"
#include <thread>
#include <chrono>

#define UIDEFFECT_UPDATE (Event::_last + 9)

class UIDEffect : public UIDialog
{
public:
	static void config(size_t horizontalBlockCount, size_t verticalBlockCount);

protected:
	static size_t verticalBlockCount, horizontalBlockCount;
	UICheck chkPreview;
	Document* document;
	bool bPreview, bRenderThreadRunning, bInvalidate, bTerminate, bUpdateView, bViewUpdated;
	BrightnessContrastBlob blob;

	// this is a dummy function and is used so that compiler 
	// doesnt optimize away an empty useless while loop
	void wait_for_painting();

	void render_thread();
	void stop_render_thread();
public:
	void on_resize(int width, int height) override;
	void process_event(OUI* element, uint32_t message, uint64_t param, bool bubbleUp) override;
	void set_document(Document* document);
	void cancel();
	void close(uint32_t wmsg) override;
	void create(OUI* caller);
	void on_timer(uint32_t nTimer) override;

	virtual void show(bool bShow = true);
	virtual void apply() = 0;
	virtual void render(Sheet* srcImage, Sheet* dstImage, int blockLeft, int blockTop, int blockRight, int blockBottom) = 0;
};
