#pragma once
#include <oui_check.h>
#include "ImageEffect.h"

class UIRadialBlur : public UIButton
{
	Sheet img, imgPattern;
	UICheck chkStretch;
	RadialBlurMode blurMode;
	double cx, cy; //[0,1]
	double amount; //[1-100]

	void calc_offset();

public:
	
	void get_pivot_point(double& cx, double& cy);
	void set_amount(double amout); //[1-100]
	void set_blur_mode(RadialBlurMode blurMode);
	virtual void fill_image();
	void on_init() override;
	void on_update() override;
	void on_resize(int width, int height) override;

	void on_mouse_move(int x, int y, uint32_t flags) override;
	void on_mouse_down(int x, int y, uint32_t flags) override;
	void on_mouse_up(int x, int y, uint32_t flags) override;
	void on_dbl_click(int x, int y, uint32_t flags) override;
};


