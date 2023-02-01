#include "ImageEffect.h"

HistogramBlob::HistogramBlob()
	: grayHisto{ 0 }, bHisto{ 0 }, gHisto{ 0 }, rHisto{ 0 } {
}

int ImageEffect::get_histo(Sheet* srcImage, HistogramBlob* blob)
{
	if (srcImage == NULL || blob == NULL) return IMAGE_EFFECT_RESULT_ERROR;

	int i, x, y, nbpp, p, w, h;
	byte gray, *data, *d;
	data = srcImage->data;
	nbpp = srcImage->nbpp;
	w = srcImage->w;
	h = srcImage->h;
	p = srcImage->pitch;

	for (i = 0; i < 256; i++) {
		blob->grayHisto[i] = 0;
		blob->bHisto[i] = 0;
		blob->gHisto[i] = 0;
		blob->rHisto[i] = 0;
	}

	for (y = 0; y < h; y++) {
		d = data + y * p;
		for (x = 0; x < w; x++) {
			gray = CLAMP255(int((double)d[0] * 0.114 + (double)d[1] * 0.587 + (double)d[2] * 0.299));
			blob->grayHisto[gray]++;
			blob->bHisto[*d++]++;
			blob->gHisto[*d++]++;
			blob->rHisto[*d++]++;
		}
	}

	blob->rMax = 0;
	blob->gMax = 0;
	blob->bMax = 0;
	blob->grayMax = 0;

	for (x = 0; x < 256; x++) {
		blob->grayMax = Max(blob->grayHisto[x], blob->grayMax);
		blob->bMax = Max(blob->bHisto[x], blob->bMax);
		blob->gMax = Max(blob->gHisto[x], blob->gMax);
		blob->rMax = Max(blob->rHisto[x], blob->rMax);
	}

	for (x = 0; x < 256; x++) {
		blob->grayHisto[x] = blob->bMax ? blob->grayHisto[x] * 255 / blob->bMax : 0;
		blob->bHisto[x] = blob->bMax ? blob->bHisto[x] * 255 / blob->bMax : 0;
		blob->gHisto[x] = blob->gMax ? blob->gHisto[x] * 255 / blob->gMax : 0;
		blob->rHisto[x] = blob->rMax ? blob->rHisto[x] * 255 / blob->rMax : 0;
	}

	return IMAGE_EFFECT_RESULT_OK;
}