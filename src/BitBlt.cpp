#include "BitBlt.h"

void bit_blt_fill_rest(Rect* area, Sheet* dstSheet, int dstLeft, int dstTop, int dstWidth, int dstHeight,
	Sheet& srcSheet, int srcLeft, int srcTop, int srcWidth, int srcHeight,
	BlendingMode blendingMode, InterpolationType interpolationType, Color& fillColor)
{
	if (IS_NULL(dstSheet->data) || INT_IS_ZERO(dstSheet->w) || INT_IS_ZERO(dstSheet->h) ||
		IS_NULL(srcSheet.data) || INT_IS_ZERO(srcSheet.w) || INT_IS_ZERO(srcSheet.h)
		) return;

	dstLeft += area->left;
	dstTop += area->top;

	int dx, dy, dsx, dsy, dex, dey, dp, dw, dr, db, dh, drw, drh, drex, drey, drsx, drsy,
		sx, sy, ssx, ssy, sex, sey, sp, sw, sr, sb, sh, sxu, sxrem;
	int trim_dsx, trim_dsy, trim_dex, trim_dey, sr_exclude, sb_exclude;
	pyte dst, src, d, s, srow, drow;
	byte sred, sgrn, sblu;

	sred = fillColor.r;
	sgrn = fillColor.g;
	sblu = fillColor.b;

	drsx = area->left;
	drsy = area->top;
	drex = area->left + area->width;
	drey = area->top + area->height;

	dst = dstSheet->data;
	dp = dstSheet->pitch;
	drw = dstSheet->w;
	drh = dstSheet->h;
	dr = Min(drw - 1, area->left + area->width - 1);
	db = Min(drh - 1, area->top + area->height - 1);
	src = srcSheet.data;
	sp = srcSheet.pitch;
	sr_exclude = srcSheet.w;
	sb_exclude = srcSheet.h;
	sr = sr_exclude - 1;
	sb = sb_exclude - 1;

	dsx = dstLeft;
	dsy = dstTop;
	dex = dsx + dstWidth;
	dey = dsy + dstHeight;
	ssx = srcLeft;
	ssy = srcTop;
	sex = ssx + srcWidth;
	sey = ssy + srcHeight;

	trim_dsx = CLAMP3(area->left, dsx, dr);
	trim_dsy = CLAMP3(area->top, dsy, db);
	trim_dex = CLAMP3(area->left, dex, dr);
	trim_dey = CLAMP3(area->top, dey, db);

	dw = dex - dsx;
	dh = dey - dsy;
	sw = sex - ssx;
	sh = sey - ssy;

	int px = fillColor.to_int();
	int _mx = trim_dsx * sw - dsx * sw + ssx * dw, mx, my;
	int _3drsx = 3 * drsx;

	_mx = trim_dsx * sw - dsx * sw + ssx * dw;
	my = trim_dsy * sh - dsy * sh + ssy * dh;
	pyte d_end, d_start;
	int len1, len2, len3;
	len1 = (trim_dsx - drsx) * 3;
	len2 = (drex - trim_dex) * 3;
	len3 = (drex - drsx) * 3;

	if (srcSheet.nbpp == OUI_BGR) {
		drow = dst + drsy * dp;
		for (dy = drsy; dy < drey; ++dy, drow += dp) {
			d = drow + _3drsx;
			if (dy >= trim_dsy && dy < trim_dey) {

				d_end = len1 + d;
				while (d < d_end) {
					*d++ = sblu;
					*d++ = sgrn;
					*d++ = sred;
				}

				sy = my / dh;
				srow = src + sy * sp;
				dx = (trim_dex - trim_dsx) * sw + _mx;
				for (mx = _mx; mx < dx; mx += sw) {
					s = srow + mx / dw * 3;
					*((int*)(d)) = *((int*)(s));
					d += 3;
				}
				my += sh;

				d_end = len2 + d;
				while (d < d_end) {
					*d++ = sblu;
					*d++ = sgrn;
					*d++ = sred;
				}
			}
			else {
				d_end = len3 + d;
				while (d < d_end) {
					*d++ = sblu;
					*d++ = sgrn;
					*d++ = sred;
				}
			}
		}
	}
}




void bit_blt(Rect* area, Sheet* dstSheet, int dstLeft, int dstTop, int dstWidth, int dstHeight,
	Sheet& srcSheet, int srcLeft, int srcTop, int srcWidth, int srcHeight,
	BlendingMode blendingMode, InterpolationType interpolationType)
{
	if (IS_NULL(dstSheet) || IS_NULL(dstSheet->data) || INT_IS_ZERO(dstSheet->w) || INT_IS_ZERO(dstSheet->h)) return;
	int x, y, sx, sy, ex, ey, cx, cy;

	if (area) {
		dstLeft += area->left;
		dstTop += area->top;
	}

	if (dstLeft < 0) {
		dstWidth += dstLeft;
		dstLeft = 0;
	}
	if (dstTop < 0) {
		dstHeight += dstTop;
		dstTop = 0;
	}

	dstWidth = Min(dstLeft + dstWidth, dstLeft + -srcLeft + srcSheet.w) - dstLeft + Min(0, srcLeft);
	dstHeight = Min(dstTop + dstHeight, dstTop + -srcTop + srcSheet.h) - dstTop + Min(0, srcTop);

	if (srcLeft < 0) {
		dstLeft += -srcLeft;
		srcLeft = 0;
	}
	else srcLeft += -Min(0, dstLeft);

	if (srcTop < 0) {
		dstTop += -srcTop;
		srcTop = 0;
	}
	else srcTop += -Min(0, dstTop);

	x = dstLeft;
	y = dstTop;
	cx = dstWidth;
	cy = dstHeight;

	ex = CLAMP2(cx + x, dstSheet->w);
	ey = CLAMP2(cy + y, dstSheet->h);
	sx = CLAMP3(0, x, dstSheet->w - 1);
	sy = CLAMP3(0, y, dstSheet->h - 1);

	if (ex - sx < 1 || ey - sy < 1 || srcLeft >= srcSheet.w || srcTop >= srcSheet.h || srcLeft + srcSheet.w < 1 || srcTop + srcSheet.h < 1)
		return;

	int dx, dy, count;
	byte cla, clp, a, p, * d, * s;
	byte* __restrict src, * __restrict dst;

	int
		dlenx = dstSheet->nbpp * (ex - sx),
		slenx = srcSheet.nbpp * (ex - sx),
		dbegx = dstSheet->nbpp * sx,
		dremx = (dstSheet->pitch - dstSheet->nbpp * ex),
		dremp = dbegx + dremx;

	dst = dstSheet->data + sy * dstSheet->pitch + dbegx;

	for (y = sy, dy = sy; y < ey; ++y, ++dy) {
		dx = sx;
		dstSheet->clip(dx, dy, &count, &cla);
		if (INT_IS_ZERO(cla)) {
			dst += dlenx + dremp;
			continue;
		}
		clp = 0xff - cla;
		d = dst;

		for (x = sx; x < ex; ++x, ++dx, d += 3, --count) {
			if (count < 1) {
				dstSheet->clip(dx, &count, &cla);
				if (INT_IS_ZERO(cla)) {
					int rem = Max(count - 1, 0);
					x += rem;
					dx += rem;
					d += 3 * rem;
					count = 1;
					continue;
				}
				clp = 0xff - cla;
			}

			int _sx = (x - dstLeft) * srcWidth / dstWidth;
			int _sy = (y - dstTop) * srcHeight / dstHeight;
			s = srcSheet.data + _sy * srcSheet.pitch + 3 * _sx;

			if (BYTE_OPAQUE(cla)) {
				*(short*)d = *(short*)s;
				d[2] = s[2];
			}
			else {
				d[0] = CLAMP255(DIV255(s[0] * cla + (d[0] * clp)));
				d[1] = CLAMP255(DIV255(s[1] * cla + (d[1] * clp)));
				d[2] = CLAMP255(DIV255(s[2] * cla + (d[2] * clp)));
			}
		}

		dst += dlenx + dremp;
	}

	//int _mx = trim_dsx * sw - dsx * sw + ssx * dw, mx, my;
	//int _3drsx = 3 * drsx;

	//_mx = trim_dsx * sw - dsx * sw + ssx * dw;
	//my = trim_dsy * sh - dsy * sh + ssy * dh;
	//pyte d_end, d_start;
	//int len1, len2, len3;
	//len1 = (trim_dsx - drsx) * 3;
	//len2 = (drex - trim_dex) * 3;
	//len3 = (drex - drsx) * 3;

	//if (srcSheet.nbpp == OUI_BGR) {
	//	drow = dst + trim_dsy * dp;
	//	for (dy = trim_dsy; dy < trim_dey; ++dy, drow += dp) {
	//		d = drow + _3drsx + len1;
	//		sy = my / dh;
	//		srow = src + sy * sp;
	//		dx = (trim_dex - trim_dsx) * sw + _mx;
	//		for (mx = _mx; mx < dx; mx += sw) {
	//			s = srow + mx / dw * 3;
	//			*((int*)(d)) = *((int*)(s));
	//			d += 3;
	//		}
	//		my += sh;
	//	}
	//}
}

