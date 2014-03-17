SET(lzo_DEFINITIONS "-DPIC -fPIC -DMALLOC_PRODUCTION -D__isthreaded=true")
MESSAGE(STATUS "lzo_DEFINITIONS=${lzo_DEFINITIONS}")
ADD_DEFINITIONS("${lzo_DEFINITIONS}")

ADD_DEFINITIONS("-fPIC -msse2")

INCLUDE_DIRECTORIES(
	${IZENELIB_SOURCE_DIR}/include/3rdparty/compression
	${IZENELIB_SOURCE_DIR}/include/3rdparty/compression/lz4
	${IZENELIB_SOURCE_DIR}/include/3rdparty/compression/lz77
	${IZENELIB_SOURCE_DIR}/include/3rdparty/compression/lzfx
	${IZENELIB_SOURCE_DIR}/include/3rdparty/compression/minilzo
	${IZENELIB_SOURCE_DIR}/include/3rdparty/compression/snappy
	${IZENELIB_SOURCE_DIR}/include/3rdparty/compression/lzend
	${IZENELIB_SOURCE_DIR}/include/3rdparty/compression/femtozip
	${IZENELIB_SOURCE_DIR}/include/3rdparty/compression/blosc
)

FILE(GLOB compressor_SRC
	"lz4/*.c"
	"lz77/*.c"
	"lzfx/*.c"
	"minilzo/*.c"
	"compressor.cpp"
	"femtozip/*.cpp"
	"femtozip/*.c"
	"blosc/*.c"
	"snappy/*.cc"
)

MACRO_ADD_LIBRARY(compressor
	SRCS
	${compressor_SRC}
)
