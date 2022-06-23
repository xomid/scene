#pragma once
#include <oui_sheet.h>

class ImageTranscoder
{
public:
	bool load(std::string filePath, Sheet& sheet);
	bool save(std::string filePath, Sheet& sheet);
};

