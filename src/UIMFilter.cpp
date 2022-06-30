#include "UIMFilter.h"

void UIMFilter::on_init() {
	boxModel.width = MENU_WIDTH;
	boxModel.height = MENU_HEIGHT;

	set_title(L"Filters");
	UIComMenu::on_init();
}