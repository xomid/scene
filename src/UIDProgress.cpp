#include "UIDProgress.h"

void UIDProgress::create(int width, int height, OUI* caller) {
	UIDialog::create(width, height, caller, DialogButtonSet::None);
	//show_buttons(false, false, false);
}

void UIDProgress::set_progress(double progress) {
	this->progress = progress;
	invalidate();
}

double UIDProgress::get_progress() const {
	return progress;
}

void UIDProgress::on_update() {
	UIDialog::on_update();
	Color crProgBack(backgroundColor.bright(-20)), crProgHandle(Colors::purple);
	Rect rcProgBack, rcProgHandle;

	rcProgBack.left = 10;
	rcProgBack.width = boxModel.width - 20;
	rcProgBack.height = 6;
	rcProgBack.top = (contentArea.height - rcProgBack.height + 1) / 2;

	rcProgHandle.set(rcProgBack);
	rcProgHandle.width = int(progress * rcProgBack.width);

	canvas.clear(&rcProgBack, &crProgBack);
	canvas.clear(&rcProgHandle, &crProgHandle);
}