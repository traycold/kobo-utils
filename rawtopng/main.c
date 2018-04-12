#include "pngtoraw.h"
#include "rawtopng.h"

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <libgen.h>

#define DEFAULT_WIDTH 1088
#define DEFAULT_HEIGHT 1536
#define DEFAULT_PNG_WIDTH 1080
#define DEFAULT_PNG_HEIGHT 1440
#define RAW_TO_PNG 1
#define PNG_TO_RAW 2

void remove_ext(char* filename, const char* ext)
{
    char *p;
    if ((p = strrchr(filename, '.')) != NULL && strcasecmp(p, ext) == 0)
        *p = '\0';
}

const char* get_raw_filename(const char* png_filename, int compress)
{
    static char filename[128];

    /* 7 = strlen(".raw.gz") */
    strncpy(filename, png_filename, sizeof(filename) - 7 - 1);
    filename[sizeof(filename) - 7] = '\0';

    remove_ext(filename, ".png");
    strcat(filename, compress ? ".raw.gz" : ".raw");

    return filename;
}

const char* get_png_filename(const char* raw_filename)
{
    static char filename[128];

    /* 4 = strlen(".png") */
    strncpy(filename, raw_filename, sizeof(filename) - 4 - 1);
    filename[sizeof(filename) - 4] = '\0';

    remove_ext(filename, ".gz");
    remove_ext(filename, ".raw");
    strcat(filename, ".png");

    return filename;
}

void usage(const char* prog, int direction)
{
    printf("Usage: %s [options] INPUT... OUTPUT\n"
           "Options:\n"
           "  -e  Encode Raw to PNG %s\n"
           "  -d  Decode PNG to Raw %s\n"
           "  -w  Image width (only needed for encode, default: %d)\n"
           "  -h  Image height (only needed for encode, default: %d)\n"
           "  -z  Compress Raw files using gzip\n"
           "\n"
           "If more than one input is given, the output should be a directory.\n"
           "\n",
           prog,
           direction == RAW_TO_PNG ? "(default)" : "",
           direction == PNG_TO_RAW ? "(default)" : "",
           DEFAULT_WIDTH, DEFAULT_HEIGHT
    );
}

int main(int argc, char** argv)
{
    int width = DEFAULT_WIDTH, height = DEFAULT_HEIGHT, widthPng = DEFAULT_PNG_WIDTH, heightPng = DEFAULT_PNG_HEIGHT;
    int direction, default_direction, compress = 0, verbose = 0;
    char *output, *p, input_file[255], output_file[255], cwd[255];
    int i, c;
    int output_is_dir;
    struct stat st;

    if (strcmp(basename(argv[0]), "pngtoraw") == 0)
        default_direction = direction = PNG_TO_RAW;
    else
        default_direction = direction = RAW_TO_PNG;

    while ((c = getopt(argc, argv, "edzvw:h:?")) != -1)
    {
        switch (c)
        {
            case 'e':
                direction = RAW_TO_PNG;
                break;

            case 'd':
                direction = PNG_TO_RAW;
                break;

            case 'z':
                compress = 1;
                break;

            case 'v':
                verbose += 1;
                break;

            case 'w':
                width = atoi(optarg);
                break;

            case 'h':
                height = atoi(optarg);
                break;

            case '?':
                usage(argv[0], default_direction);
                return 1;
        }
    }

    if (optind >= argc)
    {
        fprintf(stderr, "No input files specified.\n");
        return 1;
    }

    if (optind >= argc - 1)
    {
        fprintf(stderr, "Note: no output destination specified, assuming current directory.\n");
        output = getcwd(cwd, sizeof(cwd));
        return 1;
    }
    else
    {
        output = argv[argc - 1];
    }

    if (stat(output, &st) == 0)
    {
        output_is_dir = st.st_mode & S_IFDIR;
    }
    else if (errno == ENOENT)
    {
        if (optind - argc > 2)
        {
            if (verbose)
                fprintf(stderr, "Creating directory '%s'\n", output);

            if (mkdir(output, 0777) != 0)
            {
                perror("mkdir");
                fprintf(stderr, "Error: '%s' does not exist, and unable to create.\n", output);
                return 1;
            }
            output_is_dir = 1;
        }
        else
        {
            output_is_dir = 0;
        }
    }
    else
    {
        perror("stat");
        return 1;
    }

    for (i = optind; i < argc - 1; i++)
    {
        strncpy(output_file, output, sizeof(output_file));

        if (output_is_dir)
        {
            strncpy(input_file, argv[i], sizeof(input_file));
            p = basename(input_file);

            strcat(output_file, "/");
            if (direction == PNG_TO_RAW)
                strcat(output_file, get_raw_filename(p, compress));
            else
                strcat(output_file, get_png_filename(p));
        }

        if (verbose)
            fprintf(stderr, "%s => %s\n", argv[i], output_file);

        if (direction == PNG_TO_RAW)
        {
            if (convert_png_file(argv[i], output_file, compress) == -1)
            {
                printf("Unable to convert '%s'.\n", argv[i]);
                continue;
            }
        }
        else
        {
            if (convert_raw_file(argv[i], output_file, width, height, widthPng, heightPng) == -1)
            {
                printf("Unable to convert '%s'.\n", argv[i]);
                continue;
            }
        }
    }

    return 0;
}
