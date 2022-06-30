#pragma once
#include <oui_button.h>

class UIButtonWithThumbnail : public UIButton
{
	Sheet thumb;
	Rect rcThumb;
	Spacing thumbPadding;
	bool big;

public:
	~UIButtonWithThumbnail();
	void set_thumbnail(byte* thumbData, int width, int height, int pitch, int nbpp);
	void on_update() override;
	void on_init() override;
	void transform(bool big);
	bool on_mouse_wheel(int x, int y, int zDelta, uint32_t param) override;
	void set_thumb_padding(Spacing thumbPadding);
};

