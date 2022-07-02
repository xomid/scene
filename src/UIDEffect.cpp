#include "UIDEffect.h"

size_t UIDEffect::horizontalBlockCount = 1;
size_t UIDEffect::verticalBlockCount = 1;

void UIDEffect::config(size_t horizontalBlockCount, size_t verticalBlockCount)
{
	UIDEffect::horizontalBlockCount = horizontalBlockCount;
	UIDEffect::verticalBlockCount = verticalBlockCount;
}

void UIDEffect::create(OUI* caller) {
	int width = 300, height = 200;
	measure_size(&width, &height);
	padding.set(10, 10);
	UIDialog::create(width, height, caller, DialogButtonSet::OK_Cancel);

	set_title(L"Effect");
	document = NULL;
	bInvalidate = false;
	bUpdateView = false;
	bViewUpdated = false;
	bTerminate = false;
	bRenderThreadRunning = false;

	chkPreview.select(bPreview = true);

	chkPreview.create(10, 10, 80, 20, this);
	chkPreview.set_text(L"Preview");
	chkPreview.select(true);
}

void UIDEffect::on_resize(int width, int height) {
	UIDialog::on_resize(width, height);
	chkPreview.move(0, contentArea.height - chkPreview.boxModel.height);
}

void UIDEffect::process_event(OUI* element, uint32_t message, uint64_t param, bool bubbleUp) {
	if (element == &chkPreview && (message == Event::Select || message == Event::Deselect)) {
		chkPreview.select(message == Event::Select);
		bPreview = chkPreview.bSelected;
		bInvalidate = bPreview;
		parent->process_event(this, Event::Click, bPreview, true);
	}
	UIDialog::process_event(element, message, param, bubbleUp);
}

void UIDEffect::set_document(Document* document) {
	this->document = document;
}

void UIDEffect::cancel() {
	stop_render_thread();
}

void UIDEffect::stop_render_thread() {
	bTerminate = true;
	while (bRenderThreadRunning) {
	}
}

void UIDEffect::close(uint32_t wmsg) {
	kill_timer(0);

	if (wmsg == DialogButtonId::Cancel) {
		cancel();
	}
	else {
		stop_render_thread();
	}

	UIDialog::close(wmsg);
}

void UIDEffect::show(bool bShow) {
	if (bShow == false) {
		cancel();
	}
	else if (!bRenderThreadRunning) {
		bRenderThreadRunning = true;
		bTerminate = false;
		bInvalidate = true;
		std::thread thread(&UIDEffect::render_thread, this);
		thread.detach();
	}
	else {
		bTerminate = false;
		bInvalidate = true;
	}

	set_timer(0, 100);
	show_window(bShow);
}

void UIDEffect::wait_for_painting() {
	while (bUpdateView && !bTerminate) {
	}
}

void UIDEffect::render_thread() {

	while (bRenderThreadRunning) {

		using namespace std::chrono_literals;
		if (bInvalidate) {
			bInvalidate = false;

			if (document) {

				auto srcImage = document->get_image();
				auto dstImage = document->get_frame();

				size_t verticalBlockIndex, horizontalBlockIndex;
				int blockLeft, blockTop, blockRight, blockBottom, blockHeight, blockWidth;

				blockWidth = int(ceil((double)srcImage->w / (double)horizontalBlockCount));
				blockHeight = int(ceil((double)srcImage->h / (double)verticalBlockCount));
				blockTop = 0;

				for (verticalBlockIndex = 0; verticalBlockIndex < verticalBlockCount; ++verticalBlockIndex, blockTop += blockHeight) {
					blockBottom = blockTop + blockHeight;
					if (verticalBlockIndex == verticalBlockCount - 1)
						blockBottom = srcImage->h;

					for (horizontalBlockIndex = 0; horizontalBlockIndex < horizontalBlockCount; ++horizontalBlockIndex) {

						blockLeft = (int)horizontalBlockIndex * blockWidth;
						blockRight = blockLeft + blockWidth;
						if (horizontalBlockIndex == horizontalBlockCount - 1)
							blockRight = srcImage->w;

						wait_for_painting();

						if (bInvalidate || bTerminate)
							break;

						render(srcImage, dstImage, blockLeft, blockTop, blockRight, blockBottom);
						bUpdateView = true;

						if (bInvalidate || bTerminate)
							break;

					}

					if (bInvalidate || bTerminate)
						break;

				}
			}
		}

		if (bTerminate)
			break;
	}

	bRenderThreadRunning = false;
}

void UIDEffect::on_timer(uint32_t nTimer) {

	// bViewUpdated means that the actual paiting on the screen has been done so 
	// we signal the thread to continue;
	if (bViewUpdated) {
		bUpdateView = false;
		bViewUpdated = false;
		return;
	}

	// first, thread will set this to true to invalidate the MainView
	// and set the flag bViewUpdated to true so when the next
	// timer callback is called we know that we were done paiting 
	// and should signal the image processing thread to continue to next block or reset rendering again
	if (bUpdateView) {
		invalidate();
		bViewUpdated = true;
		if (parent) parent->process_event(this, UIDEFFECT_UPDATE, 0, true);
	}
}