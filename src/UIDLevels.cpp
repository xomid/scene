#include "UIDLevels.h"
#include "ImageEffect.h"
#include "UISeparator.h"

void UIDLevels::measure_size(int* width, int* height) {
	if (width) *width = 320;
	if (height) *height = 390;
}

void UIDLevels::reset_image() {
	cHisto.set_image(document ? document->get_image() : 0);
}

void UIDLevels::on_destroy() {
	if (blob) delete blob;
	blob = NULL;
}

void UIDLevels::on_init() {
	set_title(L"Levels");

	blob = new LevelsBlob();
	currentChannelInfo = &rgbInfo;

	selPreset.create(this);
	selChannel.create(this);
	cHisto.create(this);
	numLow.create(this);
	numMid.create(this);
	numHigh.create(this);
	numMin.create(this);
	numMax.create(this);
	slRange.create(this);
	slOutput.create(this);

	lblOutput.create(this);
	lblOutput.set_text(L"Output Levels");

	RangedFloat::debugMode = true;

	slRange.set_bound_range(0., 1., .01);
	slRange.set_range(0., 255.);
	slRange.set_value(0, 0.);
	slRange.set_value(2, 255.);
	slRange.set_value(1, .5);

	slOutput.set_range(0., 255.);
	slOutput.set_value(0, 0.);
	slOutput.set_value(1, 255.);
	slOutput.set_gradient_func(gradient_func_black_white);
	slOutput.set_handle_color(0, Colors::black);
	slOutput.set_handle_color(1, Colors::white);

	numLow.set_range(0., 1., 0., 255.);
	numMid.set_range("1.", "0.01", "0.01", "9.99");
	numHigh.set_range(255., 1., 0., 255.);
	numMin.set_range(0., 1., 0., 255.);
	numMax.set_range(255., 1., 0., 255.);

	slOutput.set_gradient_func(gradient_func_black_white);

	selPreset.set_title(L"Preset", true);
	selChannel.set_title(L"Channel", true);

	int optH = 30, sepH = 10;
	selPreset.add_option<UIButton>(L"Default", optH);
	selPreset.add_option<UISeparator>(L"", sepH, true);
	selPreset.add_option<UIButton>(L"Darker", optH);
	selPreset.add_option<UIButton>(L"Increase Contrast 1", optH);
	selPreset.add_option<UIButton>(L"Increase Contrast 2", optH);
	selPreset.add_option<UIButton>(L"Increase Contrast 3", optH);
	selPreset.add_option<UIButton>(L"Lighten Shadows", optH);
	selPreset.add_option<UIButton>(L"Lighter", optH);
	selPreset.add_option<UIButton>(L"Midtones Brighter", optH);
	selPreset.add_option<UIButton>(L"Midtones Darker", optH);
	selPreset.add_option<UISeparator>(L"", sepH, true);
	selPreset.add_option<UIButton>(L"Custom", optH);

	selChannel.add_option<UIButton>(L"RGB", optH);
	selChannel.add_option<UISeparator>(L"", sepH, true);
	selChannel.add_option<UIButton>(L"Red", optH);
	selChannel.add_option<UIButton>(L"Green", optH);
	selChannel.add_option<UIButton>(L"Blue", optH);

	selPreset.select_option(0);
	selChannel.select_option(0);

	Border bor;
	bor.set(1, backgroundColor.bright(-30));
	numLow.border = bor;
	numMid.border = bor;
	numHigh.border = bor;
	numMin.border = bor;
	numMax.border = bor;
}

void UIDLevels::on_resize(int width, int height) {
	UIDEffect::on_resize(width, height);

	int w, h, l, t, botttomMargin;
	w = contentArea.width,
		h = 30;
	l = 0;
	t = 0;
	botttomMargin = 5;
	int selChannelW = 100;

	selPreset.move(l, t, w, h); t += h + botttomMargin;
	selChannel.move(l, t, selChannelW, h); t += h + botttomMargin;

	h = 130;
	cHisto.move(l, t, w, h); t += h + botttomMargin;

	int numW = 70,
		numH = 30;

	numLow.move(l, t, numW, numH);
	numMid.move((w - numW + 1) / 2, t, numW, numH);
	numHigh.move(w - numW, t, numW, numH);

	t += numH + botttomMargin;
	h = 20;
	slRange.move(l, t, w, h); t += h + botttomMargin;

	int labelW = 120;
	int margin = 5;
	lblOutput.move(l, t, labelW, numH);
	numMin.move(w - numW * 2 - margin, t, numW, numH);
	numMax.move(w - numW, t, numW, numH); t += numH + botttomMargin;

	slOutput.move(l, t, w, h);
}

void UIDLevels::process_event(OUI* element, uint32_t message, uint64_t param, bool bubbleUp) {
	if (element == &selPreset) {
		load_preset(selPreset.get_selected_option_index());
	}
	else if (element == &selChannel) {
		currentChannelInfo = get_channel_info(selChannel.get_selected_option_index());
		
		slRange.set_value(0, currentChannelInfo->minRange);
		slRange.set_value(2, currentChannelInfo->maxRange);
		slOutput.set_value(0, currentChannelInfo->outputMin);
		slOutput.set_value(1, currentChannelInfo->outputMax);
		numMin.set_value(currentChannelInfo->outputMin);
		numMax.set_value(currentChannelInfo->outputMax);
		numLow.set_value(currentChannelInfo->minRange);
		numHigh.set_value(currentChannelInfo->maxRange);
		numMid.set_value(currentChannelInfo->gamma);

		auto f = currentChannelInfo->gamma;
		auto t = 1. - pow((f - 0.01) / 9.98, 0.299998);
		slRange.set_value(1, t);
	}
	else if (message == Event::Update) {
		if (element == &numLow) {
			slRange.set_value(0, atof(numLow.get_number().c_str()));
		}
		else if (element == &numMid) {
			auto f = atof(numMid.get_number().c_str());
			auto t = 1. - pow((f - 0.01) / 9.98, 0.299998);
			slRange.set_value(1, t);
		}
		else if (element == &numHigh) {
			slRange.set_value(1, atof(numHigh.get_number().c_str()));
		}
		else if (element == &numMin) {
			slOutput.set_value(0, atof(numMin.get_number().c_str()));
		}
		else if (element == &numMax) {
			slOutput.set_value(1, atof(numMax.get_number().c_str()));
		}
		else if (element == &slRange) {
			numLow.set_value(slRange.get_value(0));
			numHigh.set_value(slRange.get_value(2));
			auto t = slRange.get_value(1); // [0-1]
			// convert to [.01-9.99]
			auto f = pow(1. - t, 1 / 0.299998) * 9.98 + 0.01;
			currentChannelInfo->gamma = f;
			numMid.set_value(f);
		}
		else if (element == &slOutput) {
			numMin.set_value(slOutput.get_value(0));
			numMax.set_value(slOutput.get_value(1));
		}

		selPreset.select_option(9);
		currentChannelInfo->minRange = CLAMP255(slRange.get_value(0));
		currentChannelInfo->maxRange = CLAMP255(slRange.get_value(2));
		currentChannelInfo->outputMin = CLAMP255(slOutput.get_value(0));
		currentChannelInfo->outputMax = CLAMP255(slOutput.get_value(1));
		if (currentChannelInfo != &rgbInfo) currentChannelInfo->fill_lookup(&rgbInfo);

		shouldInvalidate = true;
	}
	else {
		UIDEffect::process_event(element, message, param, bubbleUp);
	}
}

ChannelLevelInfo* UIDLevels::get_channel_info(size_t index) {
	if (index == 1)
		return &redInfo;
	if (index == 2)
		return &greenInfo;
	if (index == 3)
		return &blueInfo;
	return &rgbInfo;
}

void UIDLevels::load_preset(size_t presetId) {
	ChannelLevelInfo* channelInfo;

	if (presetId > 8) return;

	for (size_t i = 0; i < 4; i++)
	{
		channelInfo = get_channel_info(i);
		channelInfo->minRange = 0;
		channelInfo->maxRange = 255;
		channelInfo->gamma = 1.;
		channelInfo->outputMin = 0;
		channelInfo->outputMax = 255;
	}

	switch (presetId)
	{
	case 1:
		rgbInfo.minRange = 15;
		break;
	case 2:
		rgbInfo.minRange = 10;
		rgbInfo.maxRange = 245;
		break;
	case 3:
		rgbInfo.minRange = 20;
		rgbInfo.maxRange = 235;
		break;
	case 4:
		rgbInfo.minRange = 30;
		rgbInfo.maxRange = 225;
		break;
	case 5:
		rgbInfo.gamma = 1.6;
		break;
	case 6:
		rgbInfo.maxRange = 230;
		break;
	case 7:
		rgbInfo.gamma = 1.25;
		break;
	case 8:
		rgbInfo.gamma = 0.75;
		break;

	default:
		break;
	}

	redInfo.fill_lookup(&rgbInfo);
	greenInfo.fill_lookup(&rgbInfo);
	blueInfo.fill_lookup(&rgbInfo);

	slRange.set_value(1, 1. - pow((currentChannelInfo->gamma - 0.01) / 9.98, 0.299998));
	slRange.set_range(currentChannelInfo->minRange, currentChannelInfo->maxRange);
	isNew = true;

	numLow.set_value(currentChannelInfo->minRange);
	numMid.set_value(currentChannelInfo->gamma);
	numHigh.set_value(currentChannelInfo->maxRange);
	slRange.set_value(0, currentChannelInfo->minRange);
	slRange.set_value(2, currentChannelInfo->maxRange);

	numMin.set_value(currentChannelInfo->outputMin);
	numMax.set_value(currentChannelInfo->outputMax);
	slOutput.set_value(0, currentChannelInfo->outputMin);
	slOutput.set_value(1, currentChannelInfo->outputMax);

	shouldInvalidate = true;
}

int UIDLevels::render(Sheet* srcImage, Sheet* dstImage, int blockLeft, int blockTop, int blockRight, int blockBottom)
{
	return ImageEffect::levels(srcImage, dstImage, blob, rgbInfo, redInfo, greenInfo, blueInfo,
		blockLeft, blockTop, blockRight, blockBottom);
}
