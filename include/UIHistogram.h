#pragma once
#include <oui_button.h>
#include "ImageEffect.h"

class UIHistogram : public UIButton
{
protected:
	Sheet img, *srcImage;
	HistogramBlob histogramBlob;
	Color fillColor;
	int invertStart, invertEnd;

	virtual void fill_image();
public:
	void set_fill_color(Color fillColor);
	void set_image(Sheet* srcImage);
	void on_init() override;
	void on_update() override;
	void on_resize(int width, int height) override;
	void on_mouse_move(int x, int y, uint32_t flags) override;
	void on_mouse_down(int x, int y, uint32_t flags) override;
	void on_mouse_up(int x, int y, uint32_t flags) override;
	void on_dbl_click(int x, int y, uint32_t flags) override;
};
