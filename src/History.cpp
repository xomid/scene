#include "History.h"

void History::clear() {
	for (size_t i = 0; i < list.size(); ++i) {
		list[i]->free();
		delete list[i];
	}
	list.clear();
	head = 0;
}

void History::push(Sheet* srcImage, std::wstring title) {
	if (head + 1 < list.size()) {
		for (size_t i = head + 1; i < list.size(); ++i) {
			list[i]->free();
			delete list[i];
		}
		list.erase(list.begin() + head + 1, list.end());
	}

	HistoryEntry* newEntry = new HistoryEntry();
	newEntry->image = new Sheet();
	newEntry->title = title;
	newEntry->image->clone(srcImage);
	list.push_back(newEntry);
	head = list.size() - 1;
}

size_t History::get_head_index() const {
	return head;
}

HistoryEntry* History::get(size_t index) {
	if (list.size() == 0) return 0;
	if (index < list.size()) head = index;
	else head = list.size() - 1;
	return get_current_entry();
}

HistoryEntry* History::undo() {
	if (head > 0) {
		head = head - 1;
		return get_current_entry();
	}
	return 0;
}

HistoryEntry* History::redo() {
	if (head + 1 < list.size()) {
		head = head + 1;
		return get_current_entry();
	}
	return 0;
}

HistoryEntry* History::get_current_entry() {
	if (list.size())
		return list[head];
	return 0;
}

std::vector<HistoryEntry*>::iterator History::begin() {
	return list.begin();
}

std::vector<HistoryEntry*>::iterator History::end() {
	return list.end();
}

size_t History::size() const {
	return list.size();
}