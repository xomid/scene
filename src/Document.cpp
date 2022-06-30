#include "Document.h"
#include "ImageTranscoder.h"

Document::Document() 
	: bInvalidated(false), fileMgr(0) {}

void Document::set_file_manager(FileManager* fileMgr) {
	this->fileMgr = fileMgr;
}

FileManager* Document::get_file_manager() const {
	return fileMgr;
}

Sheet* Document::get_image() {
	return &image;
}

std::wstring Document::get_file_path() {
	return filePath;
}

History* Document::get_history() {
	return &history;
}

int Document::close(bool bForceClose) {
	if (!bForceClose && bInvalidated) return 1;
	image.free();
	history.clear();
	filePath = L"";
	fileType = FileType::OTHER;
	bInvalidated = false;
	return 0;
}

bool Document::is_invalidated() const {
	return bInvalidated;
}

bool Document::is_open() const {
	return !image.is_useless();
}

std::wstring Document::get_alternative_file_path() const {
	std::wstring ulterredFilePath = filePath;
	auto pos = ulterredFilePath.find_last_of(L'.');
	std::wstring ext = L"";
	if (pos != std::wstring::npos) {
		ext = ulterredFilePath.substr(pos);
	}
	auto first = ulterredFilePath.substr(0, pos);
	auto mid = L"-edited";
	ulterredFilePath = first + mid + ext;
	return ulterredFilePath;
}

void Document::invalidate() {
	bInvalidated = true;
}

int Document::snap_shot(std::wstring title) {
	history.push(&image, title);
	bInvalidated = true;
	return 0;
}

int Document::load(std::wstring filePath) {
	if (close(false)) return 1;

	bInvalidated = false;
	ImageTranscoder transcoder;
	if (transcoder.load(filePath, image, fileType))
		return 1;

	this->filePath = filePath;
	std::wstring fileName = L"";
	auto pos = filePath.find_last_of(L"\\\\");
	if (pos != std::wstring::npos) {
		fileName = filePath.substr(pos + 1);
	}
	fileMgr->append_recent_file(fileName, filePath);
	snap_shot(L"Original");

	// dummy history
	{
		for (int i = 0; i < image.h * image.pitch; ++i) {
			image.data[i] = 0xff - image.data[i];
		}
		snap_shot(L"Negative");

		/*for (int y = 0; y < image.h; ++y) {
			for (int x = 0; x < image.w; ++x) {
				pyte d = image.data + x * 3 + y * image.pitch;
				byte g = (d[0] + d[1] + d[2]) / 3;
				d[0] = d[1] = d[2] = g;
			}
		}
		snap_shot(L"Grayscale");*/
	}

	return 0;
}

int Document::save(std::wstring filePath) {
	ImageTranscoder transcoder;
	if (transcoder.save(filePath, image, fileType, 100))
		return 1;
	bInvalidated = false;
	return 0;
}

void Document::undo() {
	auto chkImage = history.undo();
	if (!chkImage) return;
	image.copy(chkImage->image);
	bInvalidated = true;
}

void Document::redo() {
	auto chkImage = history.redo();
	if (!chkImage) return;
	image.copy(chkImage->image);
	bInvalidated = true;
}

void Document::checkout(size_t historyIndex) {
	if (historyIndex == history.get_head_index()) return;
	auto chkImage = history.get(historyIndex);
	if (!chkImage) return;
	image.copy(chkImage->image);
	bInvalidated = true;
}