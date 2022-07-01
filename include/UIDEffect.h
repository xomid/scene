#pragma once
#include <oui_window.h>
#include <oui_check.h>
#include "Document.h"
#include <thread>
#include <chrono>

class UIDEffect : public UIDialog
{
protected:
	UICheck chkPreview;
	Document* document;
	bool bPreview, bRunning, bInvalidate, bTerminate;
	size_t nx, ny, xpart, ypart, ylevel;
	void Foo();

public:
	void on_resize(int width, int height) override;
	void process_event(OUI* element, uint32_t message, uint64_t param, bool bubbleUp) override;
	void set_document(Document* document);
	void config(size_t inx, size_t iny, size_t ixpart, size_t iypart, size_t iylevel);
	void cancel();
	void close(uint32_t wmsg) override;
	void set_progress(size_t precent);
	void create(OUI* caller);


	virtual void show(bool bShow = true);
	virtual void apply();
	virtual void render();

	/*virtual void init_render();
	*/
};
