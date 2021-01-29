set(libpng_sources png.c pngerror.c pngget.c pngmem.c pngpread.c pngread.c pngrio.c pngrtran.c pngrutil.c pngset.c
	pngtrans.c pngwio.c pngwrite.c pngwtran.c pngwutil.c)
list(TRANSFORM libpng_sources PREPEND lib/sys/libpng/)
add_library(png_static STATIC ${libpng_sources})
target_compile_options(png_static PRIVATE -fPIC)
target_link_libraries(png_static PUBLIC zstatic)
