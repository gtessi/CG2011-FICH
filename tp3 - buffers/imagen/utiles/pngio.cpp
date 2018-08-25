#include "imagen.h"
#ifdef _USE_LIBPNG

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#include <png.h>

bool abort_(const char * s, ...)
{
	va_list args;
	va_start(args, s);
	vfprintf(stderr, s, args);
	fprintf(stderr, "\n");
	va_end(args);
	return false;
}

static int y;

static int width, height;
static png_byte color_type;
static png_byte bit_depth;

static png_structp png_ptr;
static png_infop info_ptr;
static int number_of_passes;
static png_bytep * row_pointers;

bool imagen::read_png (const char *filename) {
	if (filename) strcpy(nombre,filename);
	char *ext=ext_begin(nombre);
	if (!ext) strcat(nombre,".png"); else strcpy(ext,"png");
	
	png_byte header[8];	// 8 is the maximum size that can be checked
	
	/* open file and test for it being a png */
	FILE *fp = fopen(filename, "rb");
	if (!fp)
		return abort_("[read_png_file] File %s could not be opened for reading", filename);
	fread(header, 1, 8, fp);
	if (png_sig_cmp(header, 0, 8))
		return abort_("[read_png_file] File %s is not recognized as a PNG file", filename);
	
	
	/* initialize stuff */
	png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	
	if (!png_ptr)
		return abort_("[read_png_file] png_create_read_struct failed");
	
	info_ptr = png_create_info_struct(png_ptr);
	if (!info_ptr)
		return abort_("[read_png_file] png_create_info_struct failed");
	
	if (setjmp(png_jmpbuf(png_ptr)))
		return abort_("[read_png_file] Error during init_io");
	
	png_init_io(png_ptr, fp);
	png_set_sig_bytes(png_ptr, 8);
	
	png_read_info(png_ptr, info_ptr);
	
	w = width = info_ptr->width;
	h = height = info_ptr->height;
	color_type = info_ptr->color_type;
	bit_depth = info_ptr->bit_depth;
	
	number_of_passes = png_set_interlace_handling(png_ptr);
	
	if (color_type & PNG_COLOR_MASK_ALPHA) {
		png_set_strip_alpha(png_ptr);
	}
	if (bit_depth > 8) {
		png_set_strip_16(png_ptr);
	}
	if (color_type == PNG_COLOR_TYPE_GRAY ||
		color_type == PNG_COLOR_TYPE_GRAY_ALPHA) {
			png_set_gray_to_rgb(png_ptr);
		}
	if (color_type == PNG_COLOR_TYPE_PALETTE) {
		png_set_palette_to_rgb(png_ptr);
	}
	
	png_read_update_info(png_ptr, info_ptr);
	
	/* read file */
	if (setjmp(png_jmpbuf(png_ptr)))
		return abort_("[read_png_file] Error during read_image");
	
	row_pointers = (png_bytep*) malloc(sizeof(png_bytep) * height);
	for (y=0; y<height; y++)
		row_pointers[y] = (png_byte*) malloc(info_ptr->rowbytes);
	
	png_read_image(png_ptr, row_pointers);
	
	if (buffer) delete [] buffer;
	buffer = new unsigned char[height*info_ptr->rowbytes];
	for (y=0; y<height; y++) {
		memcpy(&buffer[(y)*info_ptr->rowbytes],row_pointers[y],info_ptr->rowbytes);
		free (row_pointers[y]);
	}
	free (row_pointers);
	color=true;
	fclose(fp);
	return true;
}


bool imagen::write_png (const char *filename) {
	if (filename) strcpy(nombre,filename);
	char *ext=ext_begin(nombre);
	if (!ext) strcat(nombre,".png"); else strcpy(ext,"png");
	/* create file */
	FILE *fp = fopen(filename, "wb");
	if (!fp)
		return abort_("[write_png_file] File %s could not be opened for writing", filename);

	/* initialize stuff */
	png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	
	if (!png_ptr)
		return abort_("[write_png_file] png_create_write_struct failed");
	
	info_ptr = png_create_info_struct(png_ptr);
	if (!info_ptr)
		return abort_("[write_png_file] png_create_info_struct failed");
	
	if (setjmp(png_jmpbuf(png_ptr)))
		return abort_("[write_png_file] Error during init_io");
	
	png_init_io(png_ptr, fp);
	
	/* write header */
	if (setjmp(png_jmpbuf(png_ptr)))
		return abort_("[write_png_file] Error during writing header");
	
	png_set_IHDR(png_ptr, info_ptr, w, h,
		8, PNG_COLOR_TYPE_RGB, PNG_INTERLACE_NONE,
		PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);
	
	png_write_info(png_ptr, info_ptr);
	
	/* write bytes */
	if (setjmp(png_jmpbuf(png_ptr)))
		return abort_("[write_png_file] Error during writing bytes");
	
	row_pointers=new png_byte*[h];
	for (y=0;y<h;y++)
		row_pointers[y]=(png_byte*)&(buffer[y*w*3]);
	
	png_write_image(png_ptr, row_pointers);
	
	
	/* end write */
	if (setjmp(png_jmpbuf(png_ptr)))
		return abort_("[write_png_file] Error during end of write");
	
	png_write_end(png_ptr, NULL);
	
	/* cleanup heap allocation */
	delete [] row_pointers;
	
	fclose(fp);
	return true;
}

#endif
