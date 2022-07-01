#include "UIDEffect.h"

void UIDEffect::create(OUI* caller) {
	int width = 300, height = 200;
	measure_size(&width, &height);
	padding.set(10, 10);
	UIDialog::create(width, height, caller, DialogButtonSet::OK_Cancel);

	set_title(L"Effect");
	document = NULL;
	bInvalidate = false;
	bRunning = false;
	bTerminate = false;
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

void UIDEffect::config(size_t inx, size_t iny, size_t ixpart, size_t iypart, size_t iylevel)
{
	nx = inx;
	ny = iny;
	xpart = ixpart;
	ypart = iypart;
	ylevel = iylevel;
}

void UIDEffect::cancel() {
	bTerminate = true;
	while (bRunning) {
	}

	if (document == NULL) return;
	auto image = document->get_image();
	auto frame = document->get_frame();
	if (frame != NULL && image != NULL)
		frame->copy(image);
}

void UIDEffect::close(uint32_t wmsg) {
	if (wmsg == DialogButtonId::Cancel) {
		cancel();
	}
	else if (wmsg == DialogButtonId::OK) {
		apply();
	}
	UIDialog::close(wmsg);
}

void UIDEffect::apply() {

}

void UIDEffect::render() {

}

void UIDEffect::show(bool bShow) {
	if (bShow == false) {
		cancel();
	}
	else if (!bRunning) {
		bRunning = true;
		bTerminate = false;
		std::thread f(&UIDEffect::Foo, this);
		f.detach();
	}
	else {
		bTerminate = false;
		bInvalidate = true;
	}

	show_window(bShow);
}

void UIDEffect::Foo() {
	bInvalidate = true;
	while (bRunning) {
		using namespace std::chrono_literals;
		//std::cout << "From the other side\n";
		//std::this_thread::sleep_for(1000ms);
		if (bInvalidate) {
			render();
			//std::cout << "Frop the top...\n";
			bInvalidate = false;
		}
		if (bTerminate)
			break;
	}
	bRunning = false;
}