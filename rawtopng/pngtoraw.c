#include "pngtoraw.h"

#include <stdint.h>
#include <zlib.h>
#include <png.h>

int convert_png_file(const char* filename, const char* raw_filename, int compress)
{
    unsigned char header_buf[8];
    int bytes, width, height, x, y;
    uint16_t pixel;
    png_structp png_ptr;
    png_infop info_ptr;
    png_bytep* row_pointers;
    png_bytep p;
    FILE *fp, *outfp = NULL;
    gzFile gzoutfp = NULL;

    if ((fp = fopen(filename, "rb")) == NULL)
    {
        perror("fopen");
        return -1;
    }

    if (compress)
    {
        if ((gzoutfp = gzopen(raw_filename, "wb")) == NULL)
        {
            perror("gzopen");
            return -1;
        }
    }
    else
    {
        if ((outfp = fopen(raw_filename, "wb")) == NULL)
        {
            perror("fopen");
            return -1;
        }
    }

    bytes = fread(header_buf, 1, sizeof(header_buf), fp);
    if (png_sig_cmp(header_buf, 0, bytes) != 0)
    {
        fprintf(stderr, "png_sig_cmp: not a PNG file\n");
        return -1;
    }

    if ((png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL)) == NULL)
    {
        fprintf(stderr, "png_create_read_struct: NULL\n");
        return -1;
    }

    if ((info_ptr = png_create_info_struct(png_ptr)) == NULL)
    {
        fprintf(stderr, "png_create_info_struct: NULL\n");
        png_destroy_read_struct(&png_ptr, NULL, NULL);
        return -1;
    }

    png_init_io(png_ptr, fp);
    png_set_sig_bytes(png_ptr, bytes);

    /* Make sure we get an 8-bits RGB image by converting 1, 2, 4 and 16-bit
     * samples and removing the alpha channel if present. */
    png_read_png(png_ptr, info_ptr, PNG_TRANSFORM_STRIP_16 |
                                    PNG_TRANSFORM_PACKING |
                                    PNG_TRANSFORM_STRIP_ALPHA |
                                    PNG_TRANSFORM_GRAY_TO_RGB, NULL);

    row_pointers = png_get_rows(png_ptr, info_ptr);
    width = png_get_image_width(png_ptr, info_ptr);
    height = png_get_image_height(png_ptr, info_ptr);

    for (y = 0; y < height; y += 1)
    {
        p = row_pointers[y];
        for (x = 0; x < width; x += 1)
        {
            pixel = (p[0] & 0xF8) << 8 | (p[1] & 0xFC) << 3 | (p[2] & 0xF8) >> 3;
            p += 3;

            if (compress)
                gzwrite(gzoutfp, &pixel, sizeof(uint16_t));
            else
                fwrite(&pixel, sizeof(uint16_t), 1, outfp);
        }
    }

    png_destroy_read_struct(&png_ptr, &info_ptr, NULL);

    fclose(fp);

    if (compress)
        gzclose(gzoutfp);
    else
        fclose(outfp);

    return 0;
}
