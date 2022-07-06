#pragma once
#include <oui_check.h>

class UISwirl : public OUI
{
	Sheet img;
	UICheck chkStretch;
	bool bStretch;
	double angle;

public:
	void set_angle(double angle);
	bool is_stretched() const;
	virtual void fill_image();
	void on_init() override;
	void on_update() override;
	void on_resize(int width, int height) override;
	void process_event(OUI* element, uint32_t message, uint64_t param, bool bubbleUp) override;
};


