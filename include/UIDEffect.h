#pragma once
#include <oui_window.h>
#include <oui_check.h>
#include "Document.h"
#include "ImageEffect.h"
#include <thread>
#include <chrono>

#define UIDEFFECT_UPDATE (Event::_last + 9)
#define UIDEFFECT_PREVIEW (UIDEFFECT_UPDATE + 1)

class UIDEffect : public UIDialog
{
public:
	static void config(size_t horizontalBlockCount, size_t verticalBlockCount);

protected:
	static size_t verticalBlockCount, horizontalBlockCount;
	UICheck chkPreview;
	Document* document;
	bool shouldPreview, isRenderThreadRunning, shouldInvalidate, shouldTerminate, shouldUpdateView, isViewUpdated, shouldCompleteRendering;
	double progress;

	// this is a dummy function and is used so that compiler 
	// doesnt optimize away an empty useless while loop
	void wait_for_painting();
	void stop_render_thread();
public:
	void render_thread();
	void on_resize(int width, int height) override;
	void process_event(OUI* element, uint32_t message, uint64_t param, bool bubbleUp) override;
	void cancel();
	void close(uint32_t wmsg) override;
	void create(OUI* caller);
	void on_timer(uint32_t nTimer) override;

	virtual double get_progress() const;
	virtual bool is_rendering_done() const;
	virtual void reset_image();
	virtual void set_document(Document* document);
	virtual void show(bool bShow = true);
	virtual int render(Sheet* srcImage, Sheet* dstImage, int blockLeft, int blockTop, int blockRight, int blockBottom) = 0;
};
