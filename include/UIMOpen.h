#pragma once
#include "UIComMenu.h"
#include "UIButtonWithBorderOnHover.h"
#include "UISeparator.h"
#include "FileManager.h"

#define UIOPEN_UPDATE (Event::_last + 1)

class UIButtonWithBorderOnHoverWithFileInfo : public UIButtonWithBorderOnHover {
public:
	const File* fileInfo;
};

class UIMOpen : public UIComMenu
{
	UIButtonWithBorderOnHover btnOpen, btnClear;
	UISeparator sp1, sp2;
	std::vector<UIButtonWithBorderOnHoverWithFileInfo*> recentItems;
	int itemHeight;
	const File* selectedRecentFile;

public:
	const File* get_selected_recent_file() const;
	void populate_recent_files_list(FileManager* fileMgr);
	void on_init() override;
	void process_event(OUI* element, uint32_t message, uint64_t param, bool bubbleUp = true) override;
};