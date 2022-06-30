#pragma once
#include <oui_sheet.h>

enum class BlendingMode {
	NORMAL, OVERLAY
};

enum class InterpolationType {
	NEAREST,
	BILINEAR,
	BICUBIC
};

void bit_blt_fill_rest(Rect* area, Sheet* dstSheet, int dstLeft, int dstTop, int dstWidth, int dstHeight,
	Sheet& srcSheet, int srcLeft, int srcTop, int srcWidth, int srcHeight,
	BlendingMode blendingMode, InterpolationType interpolationType, Color& fillColor);

void bit_blt(Rect* area, Sheet* dstSheet, int dstLeft, int dstTop, int dstWidth, int dstHeight,
	Sheet& srcSheet, int srcLeft, int srcTop, int srcWidth, int srcHeight,
	BlendingMode blendingMode, InterpolationType interpolationType);