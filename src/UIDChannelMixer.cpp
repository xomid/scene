#include "UIDChannelMixer.h"
#include "ImageEffect.h"
#include "UISeparator.h"
#include <oui_uix.h>

void gradient_func_black_red_white(void* param, double t, Color& res) {
	int r = int(t * 510 + .5);

	if (r > 255) {
		res.g = res.b = r - 255;
		res.r = 255;
	}
	else {
		res.r = CLAMP255(r);
		res.b = res.g = 0;
	}
}

void gradient_func_black_green_white(void* param, double t, Color& res) {
	int g = int(t * 510 + .5);
	if (g > 255) {
		res.r = res.b = g - 255;
		res.g = 255;
	}
	else {
		res.g = CLAMP255(g);
		res.r = res.b = 0;
	}
}

void gradient_func_black_blue_white(void* param, double t, Color& res) {
	int b = int(t * 510 + .5);
	if (b > 255) {
		res.r = res.g = b - 255;
		res.b = 255;
	}
	else {
		res.b = CLAMP255(b);
		res.r = res.g = 0;
	}
}

void UIDChannelMixer::measure_size(int* width, int* height) {
	if (width) *width = 320;
	if (height) *height = 360;
}

void UIDChannelMixer::on_init() {
	set_title(L"Channel Mixer");


	selPreset.create(this);
	selOutChannel.create(this);
	cRed.create(this);
	cGreen.create(this);
	cBlue.create(this);
	chkMono.create(this);
	chkPreserveLum.create(this);

	cRed.set_text(L"Red");
	cGreen.set_text(L"Green");
	cBlue.set_text(L"Blue");

	cRed.set_gradient(gradient_func_black_red_white);
	cGreen.set_gradient(gradient_func_black_green_white);
	cBlue.set_gradient(gradient_func_black_blue_white);

	cRed.config(0., 1, -200, 200, 60);
	cGreen.config(0., 1, -200, 200, 60);
	cBlue.config(0., 1, -200, 200, 60);

	chkMono.set_text(L"Monochrome");
	chkPreserveLum.set_text(L"Preserve Luminosity");

	chkMono.select(isMonochromatic = false);
	chkPreserveLum.select(shouldPreserveLum = true);

	selPreset.set_title(L"Preset", true);
	selOutChannel.set_title(L"Output Channel", true);

	int optH = 30, sepH = 10;
	selPreset.add_option<UIButton>(L"Default", optH);
	selPreset.add_option<UISeparator>(L"", sepH, true);
	selPreset.add_option<UIButton>(L"Gray Infrared (RGB)", optH);
	selPreset.add_option<UIButton>(L"Gray With Blue Filter (RGB)", optH);
	selPreset.add_option<UIButton>(L"Gray With Green Filter (RGB)", optH);
	selPreset.add_option<UIButton>(L"Gray With Orange Filter (RGB)", optH);
	selPreset.add_option<UIButton>(L"Gray With Red Filter (RGB)", optH);
	selPreset.add_option<UIButton>(L"Gray With Yellow Filter (RGB)", optH);
	selPreset.add_option<UISeparator>(L"", sepH, true);
	selPreset.add_option<UIButton>(L"Custom", optH);

	selOutChannel.add_option<UIButton>(L"Red", optH);
	selOutChannel.add_option<UIButton>(L"Green", optH);
	selOutChannel.add_option<UIButton>(L"Blue", optH);
	selOutChannel.add_option<UIButton>(L"Gray", optH);

	selPreset.select_option(0);
	selOutChannel.select_option(0);

	load_preset(0);
	reset_output_channel_menu();
}

void UIDChannelMixer::reset_output_channel_menu() {
	if (isMonochromatic) {
		selOutChannel.show_option(0, false);
		selOutChannel.show_option(1, false);
		selOutChannel.show_option(2, false);
		selOutChannel.show_option(3, true);
		selOutChannel.select_option(0);
	}
	else {
		selOutChannel.show_option(0, true);
		selOutChannel.show_option(1, true);
		selOutChannel.show_option(2, true);
		selOutChannel.show_option(3, false);
		selOutChannel.invalidate();
		selOutChannel.select_option(0);
	}
}

void UIDChannelMixer::on_resize(int width, int height) {
	UIDEffect::on_resize(width, height);

	int w, h, l, t, botttomMargin;
	w = contentArea.width,
		h = 30;
	l = 0;
	t = 0;
	botttomMargin = 5;

	selPreset.move(l, t, w, h); t += h + botttomMargin;
	selOutChannel.move(l, t, w, h); t += h + botttomMargin + 5;

	h = 50;
	cRed.move(l, t, w, h); t += h + botttomMargin;
	cGreen.move(l, t, w, h); t += h + botttomMargin;
	cBlue.move(l, t, w, h); t += h + botttomMargin + 5;

	h = 30;
	chkMono.move(l, t, 80, h); t += h;
	chkPreserveLum.move(l, t, 80, h);
}

ChannelMixInfo* UIDChannelMixer::get_channel_info(bool isMonochromatic, size_t index) {
	if (isMonochromatic) {
		return &gray;
	}
	else {
		switch (index) {
		case 0: return &red;
		case 1: return &green;
		default: return &blue;
		}
	}
}

void UIDChannelMixer::load_preset(size_t index) {
	if (index > 6) return;

	red.set(1., 0., 0.);
	green.set(0., 1., 0.);
	blue.set(0., 0., 1.);
	gray.set(.4, .4, .2);
	isMonochromatic = true;

	switch (index) {
	case 0:
		isMonochromatic = false;
		break;
	case 1: 
		gray.red = -.7;
		gray.green = 2.;
		gray.blue = -.3;
		break;
	case 2:
		gray.blue = 1.;
		break;
	case 3:
		gray.green = 1.;
		break;
	case 4:
		gray.red = .5;
		gray.green = .5;
		break;
	case 5:
		gray.red = 1.;
		break;
	case 6:
		gray.red = .34;
		gray.green = .66;
		break;
	}

	chkMono.select(isMonochromatic);
	reset_output_channel_menu();
	cRed.set_value(gray.red * 100.);
	cGreen.set_value(gray.green * 100.);
	cBlue.set_value(gray.blue * 100.);

	shouldInvalidate = true;
}

void UIDChannelMixer::process_event(OUI* element, uint32_t message, uint64_t param, bool bubbleUp) {

	auto idx = selOutChannel.get_selected_option_index();
	auto currChannelInfo = get_channel_info(isMonochromatic, idx);

	if (element == &cRed || element == &cGreen || element == &cBlue) {
		selPreset.select_option(7);

		if (element == &cRed) {
			currChannelInfo->red = cRed.get_value() / 100.;
			shouldInvalidate = true;
		}
		else if (element == &cGreen) {
			currChannelInfo->green = cGreen.get_value() / 100.;
			shouldInvalidate = true;
		}
		else if (element == &cBlue) {
			currChannelInfo->blue = cBlue.get_value() / 100.;
			shouldInvalidate = true;
		}
	}
	else if (element == &selPreset) {
		load_preset(selPreset.get_selected_option_index());
		shouldInvalidate = true;
	}
	else if (element == &chkMono || element == &chkPreserveLum) {
		if (message == Event::Select || message == Event::Deselect) {
			if (element == &chkMono) {
				auto prevState = isMonochromatic;
				chkMono.select(message == Event::Select);
				isMonochromatic = chkMono.isSelected;
				shouldInvalidate = isMonochromatic != prevState;
				if (shouldInvalidate)
					reset_output_channel_menu();
				load_preset(7);
			}
			else if (element == &chkPreserveLum) {
				auto prevState = shouldPreserveLum;
				chkPreserveLum.select(message == Event::Select);
				shouldPreserveLum = chkPreserveLum.isSelected;
				shouldInvalidate = shouldPreserveLum != prevState;
			}
		}
	}
	else {
		UIDEffect::process_event(element, message, param, bubbleUp);
	}
}

int UIDChannelMixer::render(Sheet* srcImage, Sheet* dstImage, int blockLeft, int blockTop, int blockRight, int blockBottom)
{
	return ImageEffect::channel_mixer(srcImage, dstImage, gray, red, green, blue, isMonochromatic, shouldPreserveLum,
		blockLeft, blockTop, blockRight, blockBottom);
}
