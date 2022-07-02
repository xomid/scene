#pragma once
#include <oui_window.h>
class UIDProgress : public UIDialog
{
	double progress;
public:
	void create(int width, int height, OUI* caller);
	void set_progress(double t);
	double get_progress() const;
	void on_update() override;
};

