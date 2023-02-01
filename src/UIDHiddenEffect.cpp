#include "UIDHiddenEffect.h"

void UIDHiddenEffect::show(bool bShow) {
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

	set_timer(0, 100);
	close(DialogButtonId::OK);
}
