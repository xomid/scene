#include "ImageTranscoder.h"
#include <libpng16/png.h>
#include <jpeglib.h>
#include <jerror.h>

struct jpegErrorManager {
	/* "public" fields */
	struct jpeg_error_mgr pub;
	/* for return to caller */
	jmp_buf setjmp_buffer;
};

char jpegLastErrorMsg[JMSG_LENGTH_MAX];

void jpegErrorExit(j_common_ptr cinfo)
{
	jpegErrorManager* myerr = (jpegErrorManager*)cinfo->err;
	longjmp(myerr->setjmp_buffer, 1);
}

int read_jpeg(FILE* file, Sheet* img) {
	if (file == NULL || !img) return 1;
	fseek(file, SEEK_SET, 0);

	jpeg_decompress_struct info;
	jpegErrorManager jerr;
	unsigned long int imgWidth, imgHeight;
	int numComponents;
	unsigned char* lpRowBuffer[1];

	info.err = jpeg_std_error(&jerr.pub);
	jerr.pub.error_exit = jpegErrorExit;

	if (setjmp(jerr.setjmp_buffer)) {
		/* If we get here, the JPEG code has signaled an error. */
		jpeg_destroy_decompress(&info);
		return 1;
	}

	jpeg_create_decompress(&info);
	jpeg_stdio_src(&info, file);
	jpeg_read_header(&info, TRUE);
	jpeg_start_decompress(&info);
	imgWidth = info.output_width;
	imgHeight = info.output_height;
	numComponents = info.num_components;

	if (img->create(imgWidth, imgHeight, numComponents))
		return 4;

	/* Read scanline by scanline */
	while (info.output_scanline < info.output_height) {
		lpRowBuffer[0] = &img->data[info.output_scanline * img->pitch];
		jpeg_read_scanlines(&info, lpRowBuffer, 1);

		// rgb to bgr
		auto d = lpRowBuffer[0];
		for (int x = 0; x < img->w; ++x) {
			auto t = *d;
			d[0] = d[2];
			d[2] = t;
			d += 3;
		}
	}

	jpeg_finish_decompress(&info);
	jpeg_destroy_decompress(&info);

	return 0;
}

int read_png(FILE* file, Sheet* img) {
	if (file == NULL || img == NULL) return 1;

	png_structp png = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (!png)
		return 1;

	png_infop info = png_create_info_struct(png);
	if (info == NULL) {
		png_destroy_write_struct(&png, &info);
		return 1;
	}

	if (setjmp(png_jmpbuf(png)))
		return 3;

	fseek(file, SEEK_SET, 0);

	png_init_io(png, file);

	png_read_info(png, info);

	png_uint_32 width, height, y;
	png_byte color_type, bit_depth;
	png_bytep* row_pointers = NULL;

	width = png_get_image_width(png, info);
	height = png_get_image_height(png, info);
	color_type = png_get_color_type(png, info);
	bit_depth = png_get_bit_depth(png, info);

	// Read any color_type into 8bit depth, RGBA format.
	// See http://www.libpng.org/pub/png/libpng-manual.txt

	if (bit_depth == 16)
		png_set_strip_16(png);

	if (color_type == PNG_COLOR_TYPE_PALETTE)
		png_set_palette_to_rgb(png);

	// PNG_COLOR_TYPE_GRAY_ALPHA is always 8 or 16bit depth.
	if (color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8)
		png_set_expand_gray_1_2_4_to_8(png);

	if (png_get_valid(png, info, PNG_INFO_tRNS))
		png_set_tRNS_to_alpha(png);

	// These color_type don't have an alpha channel then fill it with 0xff.
	if (color_type == PNG_COLOR_TYPE_RGB ||
		color_type == PNG_COLOR_TYPE_GRAY ||
		color_type == PNG_COLOR_TYPE_PALETTE)
		png_set_filler(png, 0xFF, PNG_FILLER_AFTER);

	if (color_type == PNG_COLOR_TYPE_GRAY ||
		color_type == PNG_COLOR_TYPE_GRAY_ALPHA)
		png_set_gray_to_rgb(png);

	png_read_update_info(png, info);
	int nbpp = png_get_channels(png, info); // default is rgb

	if (img->create((int)width, (int)height, nbpp))
		return 4;

	auto pitch = png_get_rowbytes(png, info);
	row_pointers = (png_bytep*)malloc(sizeof(png_bytep) * height);

	if (row_pointers && img->data != NULL) {
		for (y = 0; y < height; y++)
			row_pointers[y] = (png_byte*)malloc(pitch);

		png_read_image(png, row_pointers);
		for (y = 0; y < height; ++y) {
			auto d = img->data + y * img->pitch;
			memcpy(d, row_pointers[y], pitch);
			free(row_pointers[y]);
			row_pointers[y] = NULL;
		}
	}

	free(row_pointers);
	png_destroy_read_struct(&png, &info, NULL);
	return 0;
}

FILE* get_file(std::wstring& filePath, const wchar* mode) {
	FILE* file;
	_wfopen_s(&file, filePath.c_str(), mode);
	if (file == NULL) {
		fwprintf(stderr, L"ImageTranscoder: can't open \"%s\"\n", filePath.c_str());
		return 0;
	}
	return file;
}

int save_jpeg(FILE* file, Sheet* img, int quality) {
	if (img == NULL || img->is_useless() || img->nbpp != 3) return 1;

	jpeg_compress_struct cinfo;
	jpegErrorManager jerr;
	cinfo.err = jpeg_std_error(&jerr.pub);
	jerr.pub.error_exit = jpegErrorExit;

	if (setjmp(jerr.setjmp_buffer))
	{
		jpeg_destroy_compress(&cinfo);
		return 1;
	}

	jpeg_create_compress(&cinfo);
	jpeg_stdio_dest(&cinfo, file);

	cinfo.image_width = img->w;
	cinfo.image_height = img->h;
	cinfo.input_components = 3;
	cinfo.in_color_space = JCS_EXT_BGR;
	jpeg_set_defaults(&cinfo);
	jpeg_set_quality(&cinfo, quality, TRUE);
	jpeg_start_compress(&cinfo, TRUE);

	while (cinfo.next_scanline < cinfo.image_height) {
		auto buffer = &img->data[cinfo.next_scanline * img->pitch];
		jpeg_write_scanlines(&cinfo, &buffer, 1);
	}

	jpeg_finish_compress(&cinfo);
	jpeg_destroy_compress(&cinfo);
	return 0;
}

int save_png(FILE* file, Sheet* img) {
	if (img == NULL || file == NULL) return 1;

	png_structp png = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (!png)
		return 1;

	png_infop info = png_create_info_struct(png);
	if (NULL == info) {
		png_destroy_write_struct(&png, &info);
		return 1;
	}

	if (setjmp(png_jmpbuf(png)))
		return 3;

	png_set_IHDR(png,
		info,
		img->w,
		img->h,
		8,
		PNG_COLOR_TYPE_RGB,
		PNG_INTERLACE_NONE,
		PNG_COMPRESSION_TYPE_BASE,
		PNG_FILTER_TYPE_BASE);

	png_bytep* row_pointers = NULL;
	row_pointers = (png_bytep*)malloc(sizeof(png_bytep) * img->h);

	for (int y = 0; y < img->h; ++y) {
		row_pointers[y] = img->data + y * img->pitch;
	}

	png_init_io(png, file);
	png_set_rows(png, info, row_pointers);
	png_write_png(png, info, PNG_TRANSFORM_BGR, NULL);

	return 0;
}

int ImageTranscoder::load(std::wstring filePath, Sheet* img, FileType& fileType) {
	if (img == NULL) return 1;

	auto file = get_file(filePath, L"rb");
	if (file == NULL) return 1;

	fileType = FileType::JPEG;
	if (read_jpeg(file, img)) {
		// cannot read jpeg
		// going for png
		fileType = FileType::PNG;
		if (read_png(file, img)) {
			// last resort failed, 
			// this is definitly a failure in reading the image
			// either we cannot read it or the file is not a healthy image
			fileType = FileType::OTHER;
		}
	}

	if (file) fclose(file);
	return fileType == FileType::OTHER ? 1 : 0;
}

int ImageTranscoder::save(std::wstring filePath, Sheet* img, FileType fileType, int quality) {
	if (img == NULL) return 1;

	auto file = get_file(filePath, L"wb");
	if (file == NULL) return 1;

	int res = 0;
	switch (fileType) {
	case FileType::JPEG:
		res = save_jpeg(file, img, quality);
		break;
	case FileType::PNG:
		res = save_png(file, img);
		break;
	default:
		res = 1;
	}

	if (file) fclose(file);
	return res;
}
