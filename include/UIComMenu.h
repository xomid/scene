#pragma once
#include <oui_menu.h>
#include <oui_label.h>

#define MENU_WIDTH 200
#define MENU_HEIGHT 200
#define SMA_WIDTH 128
#define SMA_HEIGHT 96

class UIComMenu : public UIMenu
{
protected:
	UIStack list;
	UILabel lblTitle;
public:
	UIStack* get_list();
	void set_title(std::wstring title);
	void on_init() override;
	void on_resize(int width, int height) override;
	void apply_theme(bool shouldInvalidate) override;
	virtual void enable_element(uint32_t index, bool enable);
};


