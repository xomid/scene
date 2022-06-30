#include "FileManager.h"
#include <ocom.h>
#include <fstream>

const std::vector<File>& FileManager::get_recent_files() const {
	return recentFiles;
}

void FileManager::load_db(std::wstring dbPath) {
	ocom::wtrim(dbPath);
	auto len = dbPath.length();
	if (len < 2) return;
	auto pos = dbPath.find_last_not_of(L'\\');
	if (pos == len - 1)
		dbPath.append(L"\\");
	else if (pos != len - 2)
		dbPath.append(L"\\\\");

	dbPath += L"fileMgr.db";
	this->dbPath = dbPath;

	std::wifstream infile;
	infile.open(dbPath);

	if (infile.is_open()) {
		std::wstring name, path;
		while (std::getline(infile, name) && 
			std::getline(infile, path)) {
			File recentFile;
			recentFile.name = name;
			recentFile.path = path;
			recentFiles.push_back(recentFile);
		}
	}
}

void FileManager::append_recent_file(std::wstring name, std::wstring recentFilePath) {
	for (auto& recentFile : recentFiles) {
		if (recentFile.name == name && recentFile.path == recentFilePath)
			return;
	}

	File recentFile;
	recentFile.name = name;
	recentFile.path = recentFilePath;
	recentFiles.push_back(recentFile);

	flush_db();
}

void FileManager::clear_recent_file_list() {
	recentFiles.clear();
	flush_db();
}

void FileManager::flush_db() {
	std::wfstream myfile;
	myfile.open(dbPath, std::wfstream::out);
	if (myfile.is_open()) {
		for (auto& recentFile : recentFiles) {
			myfile << recentFile.name << std::endl;
			myfile << recentFile.path << std::endl;
		}
		myfile.close();
	}
}