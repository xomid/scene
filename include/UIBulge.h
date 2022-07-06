#pragma once
#include <oui_check.h>

class UIBulge : public OUI
{
	Sheet img, imgGrid;
	UICheck chkStretch;
	bool bStretch;
	double amount;

public:
	void set_amount(double amount);
	bool is_stretched() const;
	virtual void fill_image();
	void on_init() override;
	void on_update() override;
	void on_resize(int width, int height) override;
	void process_event(OUI* element, uint32_t message, uint64_t param, bool bubbleUp) override;
};


