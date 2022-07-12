#include "UIMFilter.h"
#include "UIMAdjust.h"
#include "Thumbnails_1.h"
#include "Thumbnails_2.h"
#include "UIButtonWithThumbnail.h"

void UIMFilter::on_init() {
	boxModel.width = 350;
	boxModel.height = 300;

	set_title(L"Filters");
	UIComMenu::on_init();

	std::initializer_list<ItemInfo> itemInfos = {
		{ L"Add Noise ...", 0, Thmb_AddNoise },
		{ L"Blur", 0, Thmb_Blur },
		{ L"Bulge ...", 0 , Thmb_Bulge },
		{ L"Bump", 0, Thmb_Bump },
		{ L"Crystalize ...", 0 , Thmb_Crystalize },
		{ L"Despeckle ...", 0 , Thmb_Despeckle },
		{ L"Gaussian Blur ...", 0 , Thmb_GaussianBlur },
		{ L"Gain ...", 0 , Thmb_Gain },
		{ L"Glow ...", 0 , Thmb_Glow },
		{ L"Marble ...", 0 , Thmb_Marble },
		{ L"Maximum", 0, Thmb_Maximum },
		{ L"Median ...", 0 , Thmb_Median },
		{ L"Minimum", 0, Thmb_Minimum },
		{ L"Motion Blur ...", 0 , Thmb_MotionBlur },
		{ L"Oil Paint ...", 0 , Thmb_OilPaint },
		{ L"Outline ...", 0 , Thmb_Outline },
		{ L"Pencil Sketch ...", 0 , Thmb_PencilSketch },
		{ L"Pixelate ...", 0 , Thmb_Pixelate },
		{ L"Radial Blur ...", 0 , Thmb_RadialBlur },
		{ L"Random Jitter ...", 0 , Thmb_RandomJitter },
		{ L"Reduce Noise", 0, Thmb_ReduceNoise },
		{ L"Ripple ...", 0 , Thmb_Ripple },
		{ L"Sharpen", 0, Thmb_Sharpen },
		{ L"Smart Blur ...", 0 , Thmb_SmartBlur },
		{ L"Smear ...", 0 , Thmb_Smear },
		{ L"Soft Portrait ...", 0 , Thmb_SoftPortrait },
		{ L"Stamp ...", 0 , Thmb_Stamp },
		{ L"Surface Blur ...", 0 , Thmb_SurfaceBlur },
		{ L"Swirl ...", 0 , Thmb_Swirl },
		{ L"Tile Glass ...", 0 , Thmb_TileGlass },
		{ L"Unsharp", 0, Thmb_Unsharp },
		{ L"Water ...", 0 , Thmb_Water },
		{ L"Wave ...", 0 , Thmb_Wave },
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