#pragma once
#include "UIDHiddenEffect.h"

class UIDMinimum : public UIDHiddenEffect
{
protected:
	int render(Sheet* srcImage, Sheet* dstImage, int blockLeft, int blockTop, int blockRight, int blockBottom) override;
};
