#pragma once
#include "UIDHiddenEffect.h"

class UIDAutoContrast : public UIDHiddenEffect
{
protected:
	AutoContrastBlob blob;
	int render(Sheet* srcImage, Sheet* dstImage, int blockLeft, int blockTop, int blockRight, int blockBottom) override;
};
