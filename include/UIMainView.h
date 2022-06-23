#pragma once
#include "menu_common.h"
class UIMainView : public UIMenu
{
	Sheet image;
public:
	void on_init() override;
	bool load(std::string filePath);
	bool save(std::string filePath);
};
