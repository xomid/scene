#pragma once
#include "UIDEffect.h"
#include "UIComplexS.h"
#include <oui_select.h>


class UIDCrystalize : public UIDEffect
{
	UISelect<UISelectDefaultMenu> selMode;
	UICheck chkFadeEdges;
	UIComplexS cEdgeThinkness, cRandomness, cDistancePower, cScale;
	double edgeThickness, randomness, distancePower, scale;
	bool shouldFadeEdges;
	CrystalizeMode mode;

public:
	void measure_size(int* width, int* height) override;
	void on_init() override;
	void on_resize(int width, int height) override;
	void process_event(OUI* element, uint32_t message, uint64_t param, bool bubbleUp) override;

protected:
	int render(Sheet* srcImage, Sheet* dstImage, int blockLeft, int blockTop, int blockRight, int blockBottom) override;
};
