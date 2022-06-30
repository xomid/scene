#pragma once
#include <oui_sheet.h>

enum class FileType {
	JPEG, 
	PNG,
	BMP,
	GIF,
	OTHER
};

class ImageTranscoder
{
public:
	int load(std::wstring filePath, Sheet& img, FileType& fileType);
	int save(std::wstring filePath, Sheet& img, FileType fileType, int quality);
};

