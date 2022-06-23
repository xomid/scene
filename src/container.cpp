#include "container.h"
#include "UISideView.h"
#include "UIMainView.h"
#include "ImageTranscoder.h"

ImageTranscoder transcoder;

void UIContainer::on_init()
{
	OUITheme::primary.set(0xf0, 0xf0, 0xf0);
	set_background_color(OUITheme::primary);

	int w = boxModel.width;
	int sw = 45;
	int h = boxModel.height;

	sideView.create(0, 0, sw, h, this);
	mainView.create(sw, 0, w - sw, h, this);

	mainView.set_background_color(OUITheme::primary.bright(-20));
	mainView.load("C:\\6.bmp");
}

void UIContainer::process_event(OUI* element, uint32_t message, uint64_t param, bool bubbleUp) {
	
}