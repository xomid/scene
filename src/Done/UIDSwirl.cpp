#include "UIDSwirl.h"
#include "ImageEffect.h"

void UIDSwirl::measure_size(int* width, int* height) {
	if (width) *width = 320;
	if (height) *height = 280;
}

void UIDSwirl::on_init() {
	set_title(L"Swirl");
	cSwirl.create(this);
	cAngle.create(this);
	cAngle.set_text(L"Angle");
	angle = 0;
	shouldStretch = false;
	cAngle.config(angle, 1, -45, 45, 80);
}

void UIDSwirl::on_resize(int width, int height) {
	UIDEffect::on_resize(width, height);

	int w, h, l, t, bottomMargin;
	w = contentArea.width, h = 160;
	l = 0;
	t = 0;
	bottomMargin = 5;

	cSwirl.move(l, t, w, h); t += h + bottomMargin + 5; 
	h = 50; 
	cAngle.move(l, t, w, h);
}

void UIDSwirl::process_event(OUI* element, uint32_t message, uint64_t param, bool bubbleUp) {
	if (element == &cAngle) {
		angle = cAngle.get_value();
		cSwirl.set_angle(angle);
		bInvalidate = true;
	}
	else if (element == &cSwirl) {
		shouldStretch = cSwirl.is_stretched();
		bInvalidate = true;
	}
	else {
		UIDEffect::process_event(element, message, param, bubbleUp);
	}
}

int UIDSwirl::render(Sheet* srcImage, Sheet* dstImage, int blockLeft, int blockTop, int blockRight, int blockBottom)
{
	return ImageEffect::swirl(srcImage, dstImage, angle / 180. * PI, shouldStretch, .5, .5,
		blockLeft, blockTop, blockRight, blockBottom);
}
