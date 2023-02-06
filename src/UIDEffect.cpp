#include "UIDEffect.h"

size_t UIDEffect::horizontalBlockCount = 1;
size_t UIDEffect::verticalBlockCount = 1;

void UIDEffect::config(size_t horizontalBlockCount, size_t verticalBlockCount)
{
	UIDEffect::horizontalBlockCount = horizontalBlockCount;
	UIDEffect::verticalBlockCount = verticalBlockCount;
}

void UIDEffect::reset_image() {
}

void UIDEffect::create(OUI* caller) {
	int width = 300, height = 200;
	measure_size(&width, &height);
	padding.set(10, 10);
	UIDialog::create(width, height, caller, DialogButtonSet::OK_Cancel);

	set_title(L"Effect");
	document = NULL;
	shouldInvalidate = false;
	shouldUpdateView = false;
	isViewUpdated = false;
	shouldTerminate = false;
	isRenderThreadRunning = false;
	shouldCompleteRendering = false;

	chkPreview.select(shouldPreview = true);

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
		shouldPreview = chkPreview.isSelected;
		shouldInvalidate = shouldPreview;
		parent->process_event(this, UIDEFFECT_PREVIEW, shouldPreview, true);
	}
	UIDialog::process_event(element, message, param, bubbleUp);
}

void UIDEffect::set_document(Document* document) {
	this->document = document;
	reset_image();
}

void UIDEffect::cancel() {
	stop_render_thread();
}

void UIDEffect::stop_render_thread() {
	shouldTerminate = true;
	while (isRenderThreadRunning) {
	}
}

bool UIDEffect::is_rendering_done() const {
	return !isRenderThreadRunning;
}

double UIDEffect::get_progress() const {
	return progress;
}

void UIDEffect::close(uint32_t wmsg) {
	kill_timer(0);

	if (wmsg == DialogButtonId::Cancel) {
		cancel();
		UIDialog::close(wmsg);
	}
	else {
		shouldCompleteRendering = true;
		UIDialog::close(wmsg);
	}
}

void UIDEffect::show(bool bShow) {
	if (bShow == false) {
		cancel();
	}
	else {
		chkPreview.select(true);
		if (!isRenderThreadRunning) {
			isRenderThreadRunning = true;
			shouldCompleteRendering = false;
			shouldTerminate = false;
			shouldInvalidate = true;
			std::thread thread(&UIDEffect::render_thread, this);
			thread.detach();
		}
		else {
			shouldCompleteRendering = false;
			shouldTerminate = false;
			shouldInvalidate = true;
		}
	}

	set_timer(0, 100);
	show_window(bShow);
}

void UIDEffect::wait_for_painting() {
	volatile int t = 0;
	while (shouldUpdateView && !shouldTerminate && !shouldCompleteRendering) {
		t = t * 2 - 3; 
		// this is done so compiler doenst accidentally optimize away this loop
		// the purpose of this loop is to wait until the view is updated before 
		// rendering the next block
	}
}

void UIDEffect::render_thread() {

	bool isDone = false;

	while (isRenderThreadRunning) {

		using namespace std::chrono_literals;
		if (shouldInvalidate) {
			shouldInvalidate = false;

			if (document) {

				auto srcImage = document->get_image();
				auto dstImage = document->get_frame();

				if (srcImage == NULL || dstImage == NULL)
					return;

				size_t verticalBlockIndex, horizontalBlockIndex;
				int blockLeft, blockTop, blockRight, blockBottom, blockHeight, blockWidth;

				blockWidth = int(ceil((double)srcImage->w / (double)horizontalBlockCount));
				blockHeight = int(ceil((double)srcImage->h / (double)verticalBlockCount));
				blockTop = 0;
				isDone = false;
				double blockMax = double(verticalBlockCount * horizontalBlockCount - 1);
				double blockIndex = 0;

				for (verticalBlockIndex = 0; verticalBlockIndex < verticalBlockCount && !isDone; ++verticalBlockIndex, blockTop += blockHeight) {
					blockBottom = blockTop + blockHeight;
					if (verticalBlockIndex == verticalBlockCount - 1)
						blockBottom = srcImage->h;

					for (horizontalBlockIndex = 0; horizontalBlockIndex < horizontalBlockCount && !isDone; ++horizontalBlockIndex) {

						blockLeft = (int)horizontalBlockIndex * blockWidth;
						blockRight = blockLeft + blockWidth;
						if (horizontalBlockIndex == horizontalBlockCount - 1)
							blockRight = srcImage->w;

						wait_for_painting();

						if (shouldInvalidate || shouldTerminate)
							break;

						auto res = render(srcImage, dstImage, blockLeft, blockTop, blockRight, blockBottom);
						if (res == IMAGE_EFFECT_RESULT_ERROR || res == IMAGE_EFFECT_RESULT_WHOLE_IMAGE) {
							isDone = true;
							if (res == IMAGE_EFFECT_RESULT_ERROR)
								isRenderThreadRunning = false;
						}

						progress = fmin(++blockIndex / blockMax, 1.);
						shouldUpdateView = true;

						if (shouldInvalidate || shouldTerminate)
							break;
					}

					if (shouldInvalidate || shouldTerminate)
						break;
				}

				if (isDone) shouldInvalidate = false;
			}
		}

		if (shouldTerminate)
			break;

		if (shouldCompleteRendering && !shouldInvalidate) {
			shouldCompleteRendering = false;
			break;
		}
	}

	isRenderThreadRunning = false;
}

void UIDEffect::on_timer(uint32_t nTimer) {

	// isViewUpdated means that the actual paiting on the screen has been done so 
	// we signal the thread to continue;
	if (isViewUpdated) {
		shouldUpdateView = false;
		isViewUpdated = false;
		return;
	}

	// first, thread will set this to true to invalidate the MainView
	// and set the flag isViewUpdated to true so when the next
	// timer callback is called we know that we were done paiting 
	// and should signal the image processing thread to continue to next block or reset rendering again
	if (shouldUpdateView) {
		invalidate();
		isViewUpdated = true;
		if (parent) parent->process_event(this, UIDEFFECT_UPDATE, 0, true);
	}
}