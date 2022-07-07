#include "ImageEffect.h"

#define CLAMP3F(minValue, value, maxValue) fmin(fmax(value, minValue), maxValue)

#define VALIDATE_IMAGES()\
if (srcImage == NULL || dstImage == NULL || srcImage->is_useless() || dstImage->is_useless() || \
srcImage->w != srcImage->w || srcImage->h != srcImage->h || srcImage->nbpp != srcImage->nbpp || srcImage->pitch != srcImage->pitch)\
return 1;

#define DECLARE_VARIABLES() \
int x, y, w, h, p, right, bottom;\
pyte src, dst, d, s;\
w = srcImage->w;\
h = srcImage->h;\
p = srcImage->pitch;\
src = srcImage->data;\
dst = dstImage->data;\
right = w - 1;\
bottom = h - 1;

#define CLAMP_BLOCK()\
blockLeft = CLAMP3(0, blockLeft, w - 1);\
blockRight = CLAMP3(0, blockRight, w);\
blockTop = CLAMP3(0, blockTop, h - 1);\
blockBottom = CLAMP3(0, blockBottom, h);

