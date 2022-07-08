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

	red = green = blue = 0;
	cRed.config((double)red, 1, -200, 200, 60);
	cGreen.config((double)green, 1, -200, 200, 60);
	cBlue.config((double)blue, 1, -200, 200, 60);

	chkMono.set_text(L"Monochrome");
	chkPreserveLum.set_text(L"Preserve Luminosity");

	chkMono.select(bMono = false);
	chkPreserveLum.select(bPreserveLum = true);

	selPreset.set_title(L"Preset", true);
	selOutChannel.set_title(L"Output Channel", true);

	int optH = 30, sepH = 10;
	selPreset.add_option<UIButton>(L"Default", optH);
	selPreset.add_option<UISeparator>(L"", sepH);
	selPreset.add_option<UIButton>(L"Gray Infrared (RGB)", optH);
	selPreset.add_option<UIButton>(L"Gray With Blue Filter (RGB)", optH);
	selPreset.add_option<UIButton>(L"Gray With Green Filter (RGB)", optH);
	selPreset.add_option<UIButton>(L"Gray With Orange Filter (RGB)", optH);
	selPreset.add_option<UIButton>(L"Gray With Red Filter (RGB)", optH);
	selPreset.add_option<UIButton>(L"Gray With Yellow Filter (RGB)", optH);
	selPreset.add_option<UISeparator>(L"", sepH);
	selPreset.add_option<UIButton>(L"Custom", optH);

	selOutChannel.add_option<UIButton>(L"Red", optH);
	selOutChannel.add_option<UIButton>(L"Green", optH);
	selOutChannel.add_option<UIButton>(L"Blue", optH);
	selOutChannel.add_option<UIButton>(L"Gray", optH);

	selPreset.select_option(0);
	selOutChannel.select_option(0);

	reset_output_channel_menu();
}

void UIDChannelMixer::reset_output_channel_menu() {
	if (bMono) {
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

void UIDChannelMixer::process_event(OUI* element, uint32_t message, uint64_t param, bool bubbleUp) {
	if (element == &cRed) {
		red = cRed.get_value();
		bInvalidate = true;
	}
	else if (element == &cGreen) {
		green = cGreen.get_value();
		bInvalidate = true;
	}
	else if (element == &cBlue) {
		blue = cBlue.get_value();
		bInvalidate = true;
	}
	else if (element == &chkMono || element == &chkPreserveLum) {
		if (message == Event::Select || message == Event::Deselect) {
			if (element == &chkMono) {
				auto prevState = bMono;
				chkMono.select(message == Event::Select);
				bMono = chkMono.bSelected;
				bInvalidate = bMono != prevState;
				if (bInvalidate)
					reset_output_channel_menu();
			}
			else if (element == &chkPreserveLum) {
				auto prevState = bPreserveLum;
				chkPreserveLum.select(message == Event::Select);
				bPreserveLum = chkPreserveLum.bSelected;
				bInvalidate = bPreserveLum != prevState;
			}
		}
	}
	else {
		UIDEffect::process_event(element, message, param, bubbleUp);
	}
}

int UIDChannelMixer::render(Sheet* srcImage, Sheet* dstImage, int blockLeft, int blockTop, int blockRight, int blockBottom)
{
	return ImageEffect::channel_mixer(srcImage, dstImage, blob, bMono, bPreserveLum, red, green, blue,
		blockLeft, blockTop, blockRight, blockBottom);
}
