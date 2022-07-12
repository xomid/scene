#include "UIMAdjust.h"
#include "Thumbnails_2.h"
#include "UIButtonWithThumbnail.h"


void UIMAdjust::on_init() {
	boxModel.width = 350;
	boxModel.height = 300;

	set_title(L"Adjustments");
	UIComMenu::on_init();

	std::initializer_list<ItemInfo> itemInfos = {
		{ L"Auto Contrast", 0, Thmb_AutoContrast },
		{ L"Brightness / Contrast ...", 0, Thmb_Brightness },
		{ L"Channel Mixer ...", 0, Thmb_ChannelMixer },
		{ L"Color Balance ...", 0, Thmb_ColorBalance },
		{ L"Curves ...", 0, Thmb_Curves },
		{ L"Desaturate", 0, Thmb_Desaturate },
		{ L"Hue / Saturation ...", 0 , Thmb_HueSat },
		{ L"Levels ...", 0 , Thmb_Levels },
		{ L"Negative", 0 , Thmb_Negative },
		{ L"Posterize ...", 0 , Thmb_Posterize },
		{ L"Threshold ...", 0 , Thmb_Threshold }
	};

	list.mode = UIStackMode::STACKHOR;
	int w = SMA_WIDTH + 20;
	int h = SMA_HEIGHT + 20;
	h = 70;
	w = boxModel.width - 20;
	bool big = false;

	Spacing thumbPadding;
	thumbPadding.set(4);

	for (auto& itemInfo : itemInfos) {
		auto item = new UIButtonWithThumbnail();
		item->create(0, 0, w, h, &list);
		item->set_text(itemInfo.title);
		item->set_background_color(backgroundColor);
		item->set_thumbnail(itemInfo.thumbnailData, SMA_WIDTH, SMA_HEIGHT, SMA_WIDTH * 3, 3);
		item->transform(big);
		item->set_thumb_padding(thumbPadding);
	}
}