#include "UIDCrystalize.h"
#include "ImageEffect.h"

void UIDCrystalize::measure_size(int* width, int* height) {
	if (width) *width = 320;
	if (height) *height = 320;
}

void UIDCrystalize::on_init() {
	set_title(L"Crystalize");

	selMode.create(this);
	chkFadeEdges.create(this);
	cEdgeThinkness.create(this);
	cRandomness.create(this);
	cDistancePower.create(this);
	cScale.create(this);

	chkFadeEdges.set_text(L"Fade Edges");
	cEdgeThinkness.set_text(L"Edge Thickness");
	cRandomness.set_text(L"Randomness");
	cDistancePower.set_text(L"Distance Power");
	cScale.set_text(L"Scale");

	int numFieldW = 90;
	edgeThickness = 0;
	randomness = 0;
	distancePower = 0;
	scale = 0;
	shouldFadeEdges = true;
	cEdgeThinkness.config("0", ".001", "0", "1.000", numFieldW);
	cRandomness.config("0", ".001", "0", "1.000", numFieldW);
	cDistancePower.config("0.01", "0.01", "0.01", "10.00", numFieldW);
	cScale.config("0.01", "0.01", "0.01", "256.00", numFieldW);
	chkFadeEdges.select(shouldFadeEdges);

	int optH = 30;
	selMode.add_option<UIButton>(L"Random", optH);
	selMode.add_option<UIButton>(L"Square", optH);
	selMode.add_option<UIButton>(L"Hexagonal", optH);
	selMode.add_option<UIButton>(L"Octagonal", optH);
	selMode.add_option<UIButton>(L"Triangle", optH);
	selMode.set_title(L"Mode");
	selMode.select_option(0);
	mode = CrystalizeMode::Random;
}

void UIDCrystalize::on_resize(int width, int height) {
	UIDEffect::on_resize(width, height);

	int w, h, l, t, bottomMargin;
	w = contentArea.width,
		h = 30;
	l = 0;
	t = 0;
	bottomMargin = 5;

	int marginR = bottomMargin;
	int chkFadeWidth = 100;
	selMode.move(l, t, w - chkFadeWidth - marginR, h);
	chkFadeEdges.move(w - chkFadeWidth, t, chkFadeWidth, h); t += h + bottomMargin;

	h = 50;
	cEdgeThinkness.move(l, t, w, h); t += h + bottomMargin;
	cRandomness.move(l, t, w, h); t += h + bottomMargin;
	cDistancePower.move(l, t, w, h); t += h + bottomMargin;
	cScale.move(l, t, w, h);
}

void UIDCrystalize::process_event(OUI* element, uint32_t message, uint64_t param, bool bubbleUp) {
	if (element == &selMode) {
		auto index = selMode.get_selected_option_index();
		switch (index) {
		case 1:
			mode = CrystalizeMode::Square;
			break;
		case 2:
			mode = CrystalizeMode::Hexagonal;
			break;
		case 3:
			mode = CrystalizeMode::Octagonal;
			break;
		case 4:
			mode = CrystalizeMode::Triangular;
			break;
		default:
			mode = CrystalizeMode::Random;
			break;
		}
		bInvalidate = true;
	}
	else if (element == &cEdgeThinkness) {
		edgeThickness = cEdgeThinkness.get_value();
		bInvalidate = true;
	}
	else if (element == &cRandomness) {
		randomness = cRandomness.get_value();
		bInvalidate = true;
	}
	else if (element == &cDistancePower) {
		distancePower = cDistancePower.get_value();
		bInvalidate = true;
	}
	else if (element == &cScale) {
		scale = cScale.get_value();
		bInvalidate = true;
	}
	else if (element == &chkFadeEdges) {
		if (message == Event::Select || message == Event::Deselect) {
			chkFadeEdges.select(message == Event::Select);
			shouldFadeEdges = chkFadeEdges.bSelected;
			bInvalidate = true;
		}
	}
	else {
		UIDEffect::process_event(element, message, param, bubbleUp);
	}
}

int UIDCrystalize::render(Sheet* srcImage, Sheet* dstImage, int blockLeft, int blockTop, int blockRight, int blockBottom)
{
	return ImageEffect::crystalize(srcImage, dstImage, mode, shouldFadeEdges,
		edgeThickness, randomness, distancePower, scale,
		blockLeft, blockTop, blockRight, blockBottom);
}
