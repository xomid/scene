#pragma once
#include "UIComMenu.h"

struct ItemInfo {
	std::wstring title;
	int id;
	byte* thumbnailData;
};

class UIMAdjust : public UIComMenu
{
public:
	void on_init() override;
};

