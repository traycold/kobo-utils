#include "pngtoraw.h"

#include <stdint.h>
#include <stdlib.h>

#include <zlib.h>
#include <png.h>


int convert_raw_file(const char* filename, const char* png_filename, int width, int height, int widthPng, int heightPng)
{
    int x, y;
    uint32_t pixel;
    png_structp png_ptr;
    png_infop info_ptr;
    png_bytep row_pointer, p;
    gzFile fp;
    FILE* outfp;

    if ((fp = gzopen(filename, "rb")) == NULL)
    {
        perror("gzopen");
        return -1;
    }

    if ((outfp = fopen(png_filename, "wb")) == NULL)
    {
        perror("fopen");
        return -1;
    }


    if ((png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL)) == NULL)
    {
        fprintf(stderr, "png_create_write_struct: NULL\n");
        return -1;
    }

    if ((info_ptr = png_create_info_struct(png_ptr)) == NULL)
    {
        fprintf(stderr, "png_create_info_struct: NULL\n");
        png_destroy_write_struct(&png_ptr, NULL);
        return -1;
    }

    png_init_io(png_ptr, outfp);
    png_set_compression_level(png_ptr, Z_BEST_COMPRESSION);

    /* We will be outputting a 8-bit RGB image. */
    png_set_IHDR(png_ptr, info_ptr, widthPng, heightPng, 8, PNG_COLOR_TYPE_RGB,
                 PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT,
                 PNG_FILTER_TYPE_DEFAULT);

    png_write_info(png_ptr, info_ptr);

    row_pointer = (png_bytep)malloc(widthPng * 3 * sizeof(png_byte));

    for (y = 0; y < heightPng; y += 1)
    {
        p = row_pointer;
        for (x = 0; x < width; x += 1)
        {
            gzread(fp, &pixel, sizeof(uint32_t));

            if(x < widthPng){
                p[0] = (pixel & 0xFF0000) >> 16;
                p[1] = (pixel & 0xFF00) >> 8;
                p[2] = (pixel & 0xFF);

                p += 3;
            }
        }
        png_write_row(png_ptr, row_pointer);
    }

    free(row_pointer);

    png_write_end(png_ptr, info_ptr);
    png_destroy_write_struct(&png_ptr, &info_ptr);

    gzclose(fp);
    fclose(outfp);

    return 0;
}
