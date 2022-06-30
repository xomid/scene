#pragma once
#include <oui_sheet.h>
#include "History.h"
#include "FileManager.h"
#include "ImageTranscoder.h"

class Document
{
	Sheet image;
	History history;
	FileManager* fileMgr;
	bool bInvalidated;
	std::wstring filePath;
	FileType fileType;

public:
	Document();

	std::wstring get_alternative_file_path() const;
	bool is_invalidated() const;
	bool is_open() const;
	std::wstring get_file_path();
	void set_file_manager(FileManager* fileMgr);
	FileManager* get_file_manager() const;
	Sheet* get_image();
	History* get_history();

	int load(std::wstring path);
	int save(std::wstring path);
	int close(bool bForceClose);
	int snap_shot(std::wstring title);
	void invalidate();
	void undo();
	void redo();
	void checkout(size_t historyIndex);
};

