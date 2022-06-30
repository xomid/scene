#pragma once
#include <oui_sheet.h>

struct HistoryEntry {
	Sheet* image;
	std::wstring title;

	void free() {
		if (image) {
			image->free();
			delete image;
		}
	}
};

class History
{
	size_t head;
	std::vector<HistoryEntry*> list;
public:
	void clear();
	void push(Sheet* srcImage, std::wstring title);

	HistoryEntry* get_current_entry();

	std::vector<HistoryEntry*>::iterator begin();
	std::vector<HistoryEntry*>::iterator end();

	size_t size() const;
	size_t get_head_index() const;

	// get_head_index() should be compared with your desired index
	// if they match, it means nothing needs to change
	// da!!!
	HistoryEntry* get(size_t index);
	// if returns null, it means nothing can change,
	HistoryEntry* undo();
	HistoryEntry* redo();
};

