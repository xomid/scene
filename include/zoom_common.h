#pragma once

struct ZoomInfo {
	double scale, l, t, w, h, minLeft, maxLeft, minTop, maxTop;
	ZoomInfo() {
		reset();
	}
	void reset() {
		l = 0.;
		t = 0.;
		w = 1.;
		h = 1.;
		minLeft = 0.;
		maxLeft = 0.;
		minTop = 0.;
		maxTop = 0.;
		scale = 1;
	}
};
