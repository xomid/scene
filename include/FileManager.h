#pragma once
#include <string>
#include <vector>

struct File {
	std::wstring name, path;
};

class FileManager
{
	std::vector<File> recentFiles;
	std::wstring dbPath;

public:
	const std::vector<File>& get_recent_files() const;
	void load_db(std::wstring dbPath);
	void append_recent_file(std::wstring name, std::wstring recentFilePath);
	void clear_recent_file_list();
	void flush_db();
};

