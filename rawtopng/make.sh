rm -rf rawtopng
arm-linux-gnueabihf-gcc -I/usr/include/libpng16 -lpng -I/usr/include -lz main.c rawtopng.c pngtoraw.c -o rawtopng
