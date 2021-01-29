set(ZLIB_SRCS adler32.c compress.c crc32.c deflate.c gzclose.c gzlib.c gzread.c gzwrite.c inflate.c infback.c
	inftrees.c inffast.c trees.c uncompr.c zutil.c)
list(TRANSFORM ZLIB_SRCS PREPEND lib/sys/zlib/)
add_library(zstatic STATIC ${ZLIB_SRCS})
target_compile_options(zstatic PRIVATE -fPIC)
