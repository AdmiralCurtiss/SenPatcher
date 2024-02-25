cmake_minimum_required(VERSION 3.24 FATAL_ERROR)

project(SenPatcher LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_executable(p3a_archiver)
target_sources(p3a_archiver PRIVATE
	main_p3a_archiver.cpp

	align.h
	align.cpp
	file.h
	file.cpp

	p3a/pack.cpp
	p3a/pack.h
	p3a/structs.h
	p3a/unpack.cpp
	p3a/unpack.h

	lz4/lz4.c
	lz4/lz4file.c
	lz4/lz4frame.c
	lz4/lz4hc.c
	lz4/xxhash.c
	lz4/lz4.h
	lz4/lz4file.h
	lz4/lz4frame.h
	lz4/lz4frame_static.h
	lz4/lz4hc.h
	lz4/xxhash.h

	zstd/zdict.h
	zstd/zstd.h
	zstd/zstd_errors.h
	zstd/common/debug.c
	zstd/common/entropy_common.c
	zstd/common/error_private.c
	zstd/common/fse_decompress.c
	zstd/common/pool.c
	zstd/common/threading.c
	zstd/common/xxhash.c
	zstd/common/zstd_common.c
	zstd/common/allocations.h
	zstd/common/bits.h
	zstd/common/bitstream.h
	zstd/common/compiler.h
	zstd/common/cpu.h
	zstd/common/debug.h
	zstd/common/error_private.h
	zstd/common/fse.h
	zstd/common/huf.h
	zstd/common/mem.h
	zstd/common/pool.h
	zstd/common/portability_macros.h
	zstd/common/threading.h
	zstd/common/xxhash.h
	zstd/common/zstd_deps.h
	zstd/common/zstd_internal.h
	zstd/common/zstd_trace.h
	zstd/decompress/huf_decompress.c
	zstd/decompress/zstd_ddict.c
	zstd/decompress/zstd_decompress.c
	zstd/decompress/zstd_decompress_block.c
	zstd/decompress/zstd_ddict.h
	zstd/decompress/zstd_decompress_block.h
	zstd/decompress/zstd_decompress_internal.h

	zstd/compress/fse_compress.c
	zstd/compress/hist.c
	zstd/compress/huf_compress.c
	zstd/compress/zstd_compress.c
	zstd/compress/zstd_compress_literals.c
	zstd/compress/zstd_compress_sequences.c
	zstd/compress/zstd_compress_superblock.c
	zstd/compress/zstd_double_fast.c
	zstd/compress/zstd_fast.c
	zstd/compress/zstd_lazy.c
	zstd/compress/zstd_ldm.c
	zstd/compress/zstd_opt.c
	zstd/compress/zstdmt_compress.c
	zstd/compress/clevels.h
	zstd/compress/hist.h
	zstd/compress/zstd_compress_internal.h
	zstd/compress/zstd_compress_literals.h
	zstd/compress/zstd_compress_sequences.h
	zstd/compress/zstd_compress_superblock.h
	zstd/compress/zstd_cwksp.h
	zstd/compress/zstd_double_fast.h
	zstd/compress/zstd_fast.h
	zstd/compress/zstd_lazy.h
	zstd/compress/zstd_ldm.h
	zstd/compress/zstd_ldm_geartab.h
	zstd/compress/zstd_opt.h
	zstd/compress/zstdmt_compress.h
	zstd/dictBuilder/cover.c
	zstd/dictBuilder/divsufsort.c
	zstd/dictBuilder/fastcover.c
	zstd/dictBuilder/zdict.c
	zstd/dictBuilder/cover.h
	zstd/dictBuilder/divsufsort.h
 )

