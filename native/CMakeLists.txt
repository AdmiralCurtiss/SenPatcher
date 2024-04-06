cmake_minimum_required(VERSION 3.24 FATAL_ERROR)

project(SenPatcher LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

option(SENPATCHER_BUILD_P3A "build P3A packer/extractor executable" ON)
option(SENPATCHER_BUILD_CS1HOOK "build CS1 injection dll" OFF)
option(SENPATCHER_BUILD_CS2HOOK "build CS2 injection dll" OFF)
option(SENPATCHER_BUILD_CS3HOOK "build CS3 injection dll" OFF)
option(SENPATCHER_BUILD_CS4HOOK "build CS4 injection dll" OFF)
option(SENPATCHER_BUILD_CS1TEST "build CS1 asset builder executable" OFF)
option(SENPATCHER_BUILD_CS2TEST "build CS2 asset builder executable" OFF)
option(SENPATCHER_BUILD_CS3TEST "build CS3 asset builder executable" OFF)
option(SENPATCHER_BUILD_CS4TEST "build CS4 asset builder executable" OFF)
option(SENPATCHER_BUILD_SHA1TEST "build SHA1 test executable" OFF)

function(senpatcher_generate_embeddable_file SOURCE_PATH TARGET_NAME)
	file(READ "${SOURCE_PATH}" TMP_SENPATCHER_FILE HEX)
	string(REGEX REPLACE "([0-9A-Fa-f][0-9A-Fa-f])" ",(char)0x\\1" TMP_SENPATCHER_FILE "${TMP_SENPATCHER_FILE}")
	string(SUBSTRING "${TMP_SENPATCHER_FILE}" 1 -1 TMP_SENPATCHER_FILE)
	file(GENERATE OUTPUT "${CMAKE_BINARY_DIR}/senpatcher_embeds/${TARGET_NAME}" CONTENT "${TMP_SENPATCHER_FILE}")
endfunction()

file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/senpatcher_embeds")
if (NOT EXISTS "${CMAKE_CURRENT_BINARY_DIR}/senpatcher_embeds/senpatcher_version.h")
	file(TOUCH "${CMAKE_CURRENT_BINARY_DIR}/senpatcher_embeds/senpatcher_version.h")
endif()

add_custom_target(
	GenerateGitRevisionHeader
	${CMAKE_COMMAND} -P "${CMAKE_CURRENT_SOURCE_DIR}/GitRevisionHeader.cmake"
	BYPRODUCTS "${CMAKE_CURRENT_BINARY_DIR}/senpatcher_embeds/senpatcher_version.h"
	VERBATIM
)

set(SOURCES_LZ4
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
)
set(SOURCES_ZSTD_BASE
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
)
set(SOURCES_ZSTD_COMPRESSION
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
)
set(SOURCES_ZSTD_DICTBUILDER
	zstd/dictBuilder/cover.c
	zstd/dictBuilder/divsufsort.c
	zstd/dictBuilder/fastcover.c
	zstd/dictBuilder/zdict.c
	zstd/dictBuilder/cover.h
	zstd/dictBuilder/divsufsort.h
)
set(SOURCES_LZMA
	lzma2301/7zTypes.h
	lzma2301/Compiler.h
	lzma2301/LzmaDec.c
	lzma2301/LzmaDec.h
	lzma2301/Precomp.h
)
set(SOURCES_RAPIDJSON
	rapidjson/allocators.h
	rapidjson/cursorstreamwrapper.h
	rapidjson/document.h
	rapidjson/encodedstream.h
	rapidjson/encodings.h
	rapidjson/filereadstream.h
	rapidjson/filewritestream.h
	rapidjson/fwd.h
	rapidjson/istreamwrapper.h
	rapidjson/memorybuffer.h
	rapidjson/memorystream.h
	rapidjson/ostreamwrapper.h
	rapidjson/pointer.h
	rapidjson/prettywriter.h
	rapidjson/rapidjson.h
	rapidjson/reader.h
	rapidjson/schema.h
	rapidjson/stream.h
	rapidjson/stringbuffer.h
	rapidjson/uri.h
	rapidjson/writer.h
	rapidjson/error/en.h
	rapidjson/error/error.h
	rapidjson/internal/biginteger.h
	rapidjson/internal/clzll.h
	rapidjson/internal/diyfp.h
	rapidjson/internal/dtoa.h
	rapidjson/internal/ieee754.h
	rapidjson/internal/itoa.h
	rapidjson/internal/meta.h
	rapidjson/internal/pow10.h
	rapidjson/internal/regex.h
	rapidjson/internal/stack.h
	rapidjson/internal/strfunc.h
	rapidjson/internal/strtod.h
	rapidjson/internal/swap.h
	rapidjson/msinttypes/inttypes.h
	rapidjson/msinttypes/stdint.h
)

if (SENPATCHER_BUILD_P3A)
	add_executable(p3a)
	target_sources(p3a PRIVATE
		main_p3a_archiver.cpp

		align.h
		align.cpp
		file.h
		file.cpp
		util/text.cpp
		util/text.h

		p3a/pack.cpp
		p3a/pack.h
		p3a/packfs.cpp
		p3a/packfs.h
		p3a/packjson.cpp
		p3a/packjson.h
		p3a/structs.h
		p3a/unpackfs.cpp
		p3a/unpackfs.h

		${SOURCES_LZ4}
		${SOURCES_ZSTD_BASE}
		${SOURCES_ZSTD_COMPRESSION}
		${SOURCES_ZSTD_DICTBUILDER}
		${SOURCES_RAPIDJSON}
	)
	target_compile_definitions(p3a
		PUBLIC P3A_PACKER_WITH_STD_FILESYSTEM FILE_WRAPPER_WITH_STD_FILESYSTEM
	)
	target_include_directories(p3a
		PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}"
	)
endif()

if (SENPATCHER_BUILD_CS1HOOK OR SENPATCHER_BUILD_CS1TEST)
	senpatcher_generate_embeddable_file("../SenLib/Sen1/FileFixes/ed8m2150.bin" "embed_sen1_ed8m2150.h")
	senpatcher_generate_embeddable_file("../SenLib/Sen1/FileFixes/ed8m4097.bin" "embed_sen1_ed8m4097.h")
	senpatcher_generate_embeddable_file("../SenLib/Sen1/FileFixes/ed8m4217.bin" "embed_sen1_ed8m4217.h")
	senpatcher_generate_embeddable_file("../SenLib/Sen1/FileFixes/pc8v02551_4.bin" "embed_sen1_pc8v02551_4.h")
	senpatcher_generate_embeddable_file("../SenLib/Sen1/FileFixes/pc8v10286_15.bin" "embed_sen1_pc8v10286_15.h")
	senpatcher_generate_embeddable_file("../SenLib/Sen1/FileFixes/vctiming_jp_3.bin" "embed_sen1_vctiming_jp_3.h")
	senpatcher_generate_embeddable_file("../SenLib/Sen1/FileFixes/vctiming_jp_4.bin" "embed_sen1_vctiming_jp_4.h")
	senpatcher_generate_embeddable_file("../SenLib/Sen1/FileFixes/vctiming_us_3.bin" "embed_sen1_vctiming_us_3.h")
	senpatcher_generate_embeddable_file("../SenLib/Sen1/FileFixes/vctiming_us_4.bin" "embed_sen1_vctiming_us_4.h")
endif()
if (SENPATCHER_BUILD_CS3HOOK OR SENPATCHER_BUILD_CS3TEST)
	senpatcher_generate_embeddable_file("../SenLib/Sen3/FileFixes/cvis0061.bin" "embed_sen3_cvis0061.h")
	senpatcher_generate_embeddable_file("../SenLib/Sen3/FileFixes/cvis1008.bin" "embed_sen3_cvis1008.h")
	senpatcher_generate_embeddable_file("../SenLib/Sen3/FileFixes/insa_05.bin" "embed_sen3_insa_05.h")
	senpatcher_generate_embeddable_file("../SenLib/Sen3/FileFixes/insa_08.bin" "embed_sen3_insa_08.h")
	senpatcher_generate_embeddable_file("../SenLib/Sen3/FileFixes/insa_09.bin" "embed_sen3_insa_09.h")
	senpatcher_generate_embeddable_file("../SenLib/Sen3/FileFixes/jump009.bin" "embed_sen3_jump009.h")
	senpatcher_generate_embeddable_file("../SenLib/Sen3/FileFixes/t0250_dat.bin" "embed_sen3_t0250_dat.h")
	senpatcher_generate_embeddable_file("../SenLib/Sen3/FileFixes/v00_e0427.bin" "embed_sen3_v00_e0427.h")
	senpatcher_generate_embeddable_file("../SenLib/Sen3/FileFixes/v00_e0441.bin" "embed_sen3_v00_e0441.h")
	senpatcher_generate_embeddable_file("../SenLib/Sen3/FileFixes/v00_e1032.bin" "embed_sen3_v00_e1032.h")
	senpatcher_generate_embeddable_file("../SenLib/Sen3/FileFixes/v00_e1054.bin" "embed_sen3_v00_e1054.h")
	senpatcher_generate_embeddable_file("../SenLib/Sen3/FileFixes/v00_s0027.bin" "embed_sen3_v00_s0027.h")
	senpatcher_generate_embeddable_file("../SenLib/Sen3/FileFixes/v00_s0028.bin" "embed_sen3_v00_s0028.h")
	senpatcher_generate_embeddable_file("../SenLib/Sen3/FileFixes/v00_s0081.bin" "embed_sen3_v00_s0081.h")
	senpatcher_generate_embeddable_file("../SenLib/Sen3/FileFixes/v00_s0264.bin" "embed_sen3_v00_s0264.h")
	senpatcher_generate_embeddable_file("../SenLib/Sen3/FileFixes/v20_e0066.bin" "embed_sen3_v20_e0066.h")
	senpatcher_generate_embeddable_file("../SenLib/Sen3/FileFixes/v27_e0042.bin" "embed_sen3_v27_e0042.h")
	senpatcher_generate_embeddable_file("../SenLib/Sen3/FileFixes/v31_e0070.bin" "embed_sen3_v31_e0070.h")
	senpatcher_generate_embeddable_file("../SenLib/Sen3/FileFixes/v43_e0023.bin" "embed_sen3_v43_e0023.h")
	senpatcher_generate_embeddable_file("../SenLib/Sen3/FileFixes/v49_e0004.bin" "embed_sen3_v49_e0004.h")
	senpatcher_generate_embeddable_file("../SenLib/Sen3/FileFixes/v52_e0109.bin" "embed_sen3_v52_e0109.h")
	senpatcher_generate_embeddable_file("../SenLib/Sen3/FileFixes/v52_e0119.bin" "embed_sen3_v52_e0119.h")
	senpatcher_generate_embeddable_file("../SenLib/Sen3/FileFixes/v55_e0018.bin" "embed_sen3_v55_e0018.h")
	senpatcher_generate_embeddable_file("../SenLib/Sen3/FileFixes/v55_e0062.bin" "embed_sen3_v55_e0062.h")
	senpatcher_generate_embeddable_file("../SenLib/Sen3/FileFixes/v56_e0080.bin" "embed_sen3_v56_e0080.h")
	senpatcher_generate_embeddable_file("../SenLib/Sen3/FileFixes/v56_e0092.bin" "embed_sen3_v56_e0092.h")
	senpatcher_generate_embeddable_file("../SenLib/Sen3/FileFixes/v57_e0020.bin" "embed_sen3_v57_e0020.h")
	senpatcher_generate_embeddable_file("../SenLib/Sen3/FileFixes/v64_e0040.bin" "embed_sen3_v64_e0040.h")
	senpatcher_generate_embeddable_file("../SenLib/Sen3/FileFixes/v72_e0005.bin" "embed_sen3_v72_e0005.h")
	senpatcher_generate_embeddable_file("../SenLib/Sen3/FileFixes/v72_e0011.bin" "embed_sen3_v72_e0011.h")
	senpatcher_generate_embeddable_file("../SenLib/Sen3/FileFixes/v72_e0012.bin" "embed_sen3_v72_e0012.h")
	senpatcher_generate_embeddable_file("../SenLib/Sen3/FileFixes/v93_e0300.bin" "embed_sen3_v93_e0300.h")
endif()

set(SOURCES_CS1_FIXES
	sen1/file_fixes/missing_audio_files.cpp
	sen1/file_fixes/scripts_book_dat_us_book00_dat.cpp
	sen1/file_fixes/scripts_book_dat_us_book00_dat_shared.h
	sen1/file_fixes/scripts_book_dat_us_book01_dat.cpp
	sen1/file_fixes/scripts_book_dat_us_book02_dat.cpp
	sen1/file_fixes/scripts_book_dat_us_book03_dat.cpp
	sen1/file_fixes/scripts_scena_dat_us_a0006_dat.cpp
	sen1/file_fixes/scripts_scena_dat_us_c0100_dat.cpp
	sen1/file_fixes/scripts_scena_dat_us_c0110_dat.cpp
	sen1/file_fixes/scripts_scena_dat_us_m0040_dat.cpp
	sen1/file_fixes/scripts_scena_dat_us_m2130_dat.cpp
	sen1/file_fixes/scripts_scena_dat_us_m3008_dat.cpp
	sen1/file_fixes/scripts_scena_dat_us_r0600_dat.cpp
	sen1/file_fixes/scripts_scena_dat_us_r0601_dat.cpp
	sen1/file_fixes/scripts_scena_dat_us_r0610_dat.cpp
	sen1/file_fixes/scripts_scena_dat_us_r0800_dat.cpp
	sen1/file_fixes/scripts_scena_dat_us_t0000_dat.cpp
	sen1/file_fixes/scripts_scena_dat_us_t0000c_dat.cpp
	sen1/file_fixes/scripts_scena_dat_us_t0010_dat.cpp
	sen1/file_fixes/scripts_scena_dat_us_t0020_dat.cpp
	sen1/file_fixes/scripts_scena_dat_us_t0031_dat.cpp
	sen1/file_fixes/scripts_scena_dat_us_t0032_dat.cpp
	sen1/file_fixes/scripts_scena_dat_us_t0050_dat.cpp
	sen1/file_fixes/scripts_scena_dat_us_t0060_dat.cpp
	sen1/file_fixes/scripts_scena_dat_us_t0070_dat.cpp
	sen1/file_fixes/scripts_scena_dat_us_t0080_dat.cpp
	sen1/file_fixes/scripts_scena_dat_us_t0090_dat.cpp
	sen1/file_fixes/scripts_scena_dat_us_t1000_dat.cpp
	sen1/file_fixes/scripts_scena_dat_us_t1010_dat.cpp
	sen1/file_fixes/scripts_scena_dat_us_t1020_dat.cpp
	sen1/file_fixes/scripts_scena_dat_us_t1030_dat.cpp
	sen1/file_fixes/scripts_scena_dat_us_t1040_dat.cpp
	sen1/file_fixes/scripts_scena_dat_us_t1050_dat.cpp
	sen1/file_fixes/scripts_scena_dat_us_t1110_dat.cpp
	sen1/file_fixes/scripts_scena_dat_us_t1500_dat.cpp
	sen1/file_fixes/scripts_scena_dat_us_t3500_dat.cpp
	sen1/file_fixes/scripts_scena_dat_us_t4610_dat.cpp
	sen1/file_fixes/scripts_scena_dat_us_t5660_dat.cpp
	sen1/file_fixes/scripts_talk_dat_us_tk_angelica_dat.cpp
	sen1/file_fixes/scripts_talk_dat_us_tk_becky_dat.cpp
	sen1/file_fixes/scripts_talk_dat_us_tk_beryl_dat.cpp
	sen1/file_fixes/scripts_talk_dat_us_tk_edel_dat.cpp
	sen1/file_fixes/scripts_talk_dat_us_tk_heinrich_dat.cpp
	sen1/file_fixes/scripts_talk_dat_us_tk_laura_dat.cpp
	sen1/file_fixes/scripts_talk_dat_us_tk_thomas_dat.cpp
	sen1/file_fixes/scripts_talk_dat_us_tk_vandyck_dat.cpp
	sen1/file_fixes/se_wav_ed8m2123_wav.cpp
	sen1/file_fixes/text_dat_t_item_tbl.cpp
	sen1/file_fixes/text_dat_t_voice_tbl.cpp
	sen1/file_fixes/text_dat_us_t_item_tbl_t_magic_tbl.cpp
	sen1/file_fixes/text_dat_us_t_item_tbl_t_magic_tbl.h
	sen1/file_fixes/text_dat_us_t_notecook_tbl.cpp
	sen1/file_fixes/text_dat_us_t_voice_tbl.cpp
	sen1/file_fixes.cpp
	sen1/file_fixes.h
	sen1/tbl.cpp
	sen1/tbl.h
)

set(SOURCES_CS2_FIXES
	sen1/file_fixes/scripts_book_dat_us_book00_dat_shared.h
	sen2/file_fixes/scripts_book_dat_us_book00_dat.cpp
	sen2/file_fixes/scripts_book_dat_us_book03_dat.cpp
	sen2/file_fixes/scripts_book_dat_us_book04_dat.cpp
	sen2/file_fixes/scripts_book_dat_us_book05_dat.cpp
	sen2/file_fixes/scripts_scena_dat_us_e7050_dat.cpp
	sen2/file_fixes/scripts_scena_dat_us_e7060_dat.cpp
	sen2/file_fixes/scripts_scena_dat_us_e7090_dat.cpp
	sen2/file_fixes/scripts_scena_dat_us_e7101_dat.cpp
	sen2/file_fixes/scripts_scena_dat_us_e7110_dat.cpp
	sen2/file_fixes/scripts_scena_dat_us_r0920_dat.cpp
	sen2/file_fixes/scripts_scena_dat_us_r1010_dat.cpp
	sen2/file_fixes/scripts_scena_dat_us_system_dat.cpp
	sen2/file_fixes/scripts_scena_dat_us_t0001_dat.cpp
	sen2/file_fixes/scripts_scena_dat_us_t0010_dat.cpp
	sen2/file_fixes/scripts_scena_dat_us_t1010_dat.cpp
	sen2/file_fixes/scripts_scena_dat_us_t3060_dat.cpp
	sen2/file_fixes/scripts_scena_dat_us_t3500_dat.cpp
	sen2/file_fixes/scripts_scena_dat_us_t3740_dat.cpp
	sen2/file_fixes/scripts_scena_dat_us_t4000_dat.cpp
	sen2/file_fixes/scripts_scena_dat_us_t4080_dat.cpp
	sen2/file_fixes/scripts_scena_dat_us_t5501_dat.cpp
	sen2/file_fixes/scripts_scena_dat_us_t6500_dat.cpp
	sen2/file_fixes/scripts_talk_dat_us_tk_beryl_dat.cpp
	sen2/file_fixes/scripts_talk_dat_us_tk_emily_dat.cpp
	sen2/file_fixes/scripts_talk_dat_us_tk_heinrich_dat.cpp
	sen2/file_fixes/scripts_talk_dat_us_tk_monica_dat.cpp
	sen2/file_fixes/scripts_talk_dat_us_tk_rosine_dat.cpp
	sen2/file_fixes/text_dat_us_t_item_tbl_t_magic_tbl.cpp
	sen2/file_fixes/text_dat_us_t_item_tbl_t_magic_tbl.h
	sen2/file_fixes/text_dat_us_t_notecook_tbl.cpp
	sen2/file_fixes/text_dat_us_t_voice_tbl.cpp
	sen2/file_fixes.cpp
	sen2/file_fixes.h
	sen2/tbl.cpp
	sen2/tbl.h
)

set(SOURCES_CS3_FIXES
	sen3/FileFixes/alchr022_dat.cpp
	sen3/FileFixes/a0417_dat.cpp
	sen3/FileFixes/book05_dat.cpp
	sen3/FileFixes/book06_dat.cpp
	sen3/FileFixes/book07_dat.cpp
	sen3/FileFixes/c0000_dat.cpp
	sen3/FileFixes/c0200_dat.cpp
	sen3/FileFixes/c0250_dat.cpp
	sen3/FileFixes/c0400_dat.cpp
	sen3/FileFixes/c0420_dat.cpp
	sen3/FileFixes/c0430_dat.cpp
	sen3/FileFixes/c0820_dat.cpp
	sen3/FileFixes/c0830_dat.cpp
	sen3/FileFixes/c2430_dat.cpp
	sen3/FileFixes/c2440_dat.cpp
	sen3/FileFixes/c2610_dat.cpp
	sen3/FileFixes/c3000_dat.cpp
	sen3/FileFixes/c3010_dat.cpp
	sen3/FileFixes/c3210_dat.cpp
	sen3/FileFixes/c3610_dat.cpp
	sen3/FileFixes/f0010_dat.cpp
	sen3/FileFixes/f2000_dat.cpp
	sen3/FileFixes/I_CVIS0061_pkg.cpp
	sen3/FileFixes/I_CVIS1008_pkg.cpp
	sen3/FileFixes/I_JMP009_pkg.cpp
	sen3/FileFixes/insa05.cpp
	sen3/FileFixes/insa08.cpp
	sen3/FileFixes/insa09.cpp
	sen3/FileFixes/m0000_dat.cpp
	sen3/FileFixes/m0100_dat.cpp
	sen3/FileFixes/m0300_dat.cpp
	sen3/FileFixes/m0600_dat.cpp
	sen3/FileFixes/m3000_dat.cpp
	sen3/FileFixes/m3420_dat.cpp
	sen3/FileFixes/m3430_dat.cpp
	sen3/FileFixes/m4004_dat.cpp
	sen3/FileFixes/r0210_dat.cpp
	sen3/FileFixes/r2290_dat.cpp
	sen3/FileFixes/r3000_dat.cpp
	sen3/FileFixes/r3090_dat.cpp
	sen3/FileFixes/r3200_dat.cpp
	sen3/FileFixes/r3430_dat.cpp
	sen3/FileFixes/r4200_dat.cpp
	sen3/FileFixes/r4290_dat.cpp
	sen3/FileFixes/t_item.cpp
	sen3/FileFixes/t_jump.cpp
	sen3/FileFixes/t_magic.cpp
	sen3/FileFixes/t_mons.cpp
	sen3/FileFixes/t_mstqrt.cpp
	sen3/FileFixes/t_name.cpp
	sen3/FileFixes/t_notecook.cpp
	sen3/FileFixes/t_place.cpp
	sen3/FileFixes/t_text.cpp
	sen3/FileFixes/t_vctiming_us.cpp
	sen3/FileFixes/t0000_dat.cpp
	sen3/FileFixes/t0010_dat.cpp
	sen3/FileFixes/t0080_dat.cpp
	sen3/FileFixes/t0100_dat.cpp
	sen3/FileFixes/t0200_dat.cpp
	sen3/FileFixes/t0210_dat.cpp
	sen3/FileFixes/t0250_dat.cpp
	sen3/FileFixes/t0260_dat.cpp
	sen3/FileFixes/t0400_dat.cpp
	sen3/FileFixes/t0410_dat.cpp
	sen3/FileFixes/t3000_dat.cpp
	sen3/FileFixes/t3200_dat.cpp
	sen3/FileFixes/t3220_dat.cpp
	sen3/FileFixes/t3400_dat.cpp
	sen3/FileFixes/t3510_dat.cpp
	sen3/FileFixes/t3600_dat.cpp
	sen3/FileFixes/tk_ada_dat.cpp
	sen3/FileFixes/tk_linde_dat.cpp
	sen3/FileFixes/tk_patrick_dat.cpp
	sen3/FileFixes/tk_stark_dat.cpp
	sen3/FileFixes/tk_tovar_dat.cpp
	sen3/FileFixes/tk_zessica_dat.cpp
	sen3/FileFixes/v0010_dat.cpp
	sen3/FileFixes/v0030_dat.cpp
	sen3/FileFixes/v0050_dat.cpp
	sen3/FileFixes/voice_opus_ps4_103.cpp
	sen3/FileFixes/voice_opus_v00e0441.cpp
	sen3/FileFixes/voice_opus_v00s2728.cpp
	sen3/file_fixes.cpp
	sen3/file_fixes.h
	sen3/patch_pkg.cpp
	sen3/patch_pkg.h
	sen3/tbl.cpp
	sen3/tbl.h
)

set(SOURCES_CS4_FIXES
	sen4/file_fixes/f4200_dat.cpp
	sen4/file_fixes/m9031_dat.cpp
	sen4/file_fixes/t_item.cpp
	sen4/file_fixes/t_mstqrt.cpp
	sen4/file_fixes/t_text.cpp
	sen4/file_fixes/t3600_dat.cpp
	sen4/file_fixes.cpp
	sen4/file_fixes.h
	sen4/tbl.cpp
	sen4/tbl.h
)

set(SOURCES_BASE_FIXES
	align.h
	align.cpp
	bps.h
	bps.cpp
	crc32.c
	crc32.h
	decompress_helper.cpp
	decompress_helper.h
	file.h
	file.cpp
	logger.cpp
	logger.h
	sha1.cpp
	sha1.h

	util/endian.h
	util/memread.h
	util/memwrite.h
	util/stream.cpp
	util/stream.h
	util/text.cpp
	util/text.h
	util/vector.h

	p3a/p3a.cpp
	p3a/p3a.h
	p3a/pack.cpp
	p3a/pack.h
	p3a/structs.h
	p3a/util.cpp
	p3a/util.h

	sen/asset_patch.cpp
	sen/asset_patch.h
	sen/book_table.cpp
	sen/book_table.h
	sen/file_getter.h
	sen/pkg.cpp
	sen/pkg.h
	sen/sen_script_patcher.cpp
	sen/sen_script_patcher.h

	${SOURCES_LZMA}
)

if (SENPATCHER_BUILD_CS1TEST)
	add_executable(cs1test)
	target_sources(cs1test PRIVATE
		main_cs1_test.cpp

		${SOURCES_BASE_FIXES}
		${SOURCES_CS1_FIXES}

		${SOURCES_LZ4}
		${SOURCES_ZSTD_BASE}
		${SOURCES_ZSTD_COMPRESSION}
	)
	target_include_directories(cs1test
		PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_BINARY_DIR}/senpatcher_embeds"
	)
	add_dependencies(cs1test GenerateGitRevisionHeader)
endif()

if (SENPATCHER_BUILD_CS1HOOK)
	add_library(cs1hook SHARED)
	target_sources(cs1hook PRIVATE
		dinput8.def
		main_cs1.cpp
		ini.cpp
		ini.h
		modload/loaded_mods.cpp
		modload/loaded_mods.h
		x86/emitter.cpp
		x86/emitter.h
		x86/inject_jump_into.h
		x86/page_unprotect.cpp
		x86/page_unprotect.h

		sen1/exe_patch.h
		sen1/exe_patch_add_senpatcher_version_to_title.cpp
		sen1/exe_patch_allow_r2_notebook_shortcut.cpp
		sen1/exe_patch_change_turbo_mode_button.cpp
		sen1/exe_patch_deglobalize_mutexes.cpp
		sen1/exe_patch_disable_mouse_capture.cpp
		sen1/exe_patch_disable_pause_on_focus_loss.cpp
		sen1/exe_patch_fix_arts_support_cutin.cpp
		sen1/exe_patch_fix_textbox_advance_prompt.cpp
		sen1/exe_patch_fix_voice_tables.cpp
		sen1/exe_patch_force_no_kerning.cpp
		sen1/exe_patch_force_xinput.cpp
		sen1/exe_patch_remove_turbo_auto_skip.cpp
		sen1/exe_patch_show_mouse_cursor.cpp
		sen1/exe_patch_thor_mq_string.cpp
		sen1/inject_modloader.h
		sen1/inject_modloader.cpp

		${SOURCES_BASE_FIXES}
		${SOURCES_CS1_FIXES}

		${SOURCES_LZ4}
		${SOURCES_ZSTD_BASE}
		${SOURCES_ZSTD_COMPRESSION}
	)
	target_include_directories(cs1hook
		PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_BINARY_DIR}/senpatcher_embeds"
	)
	add_dependencies(cs1hook GenerateGitRevisionHeader)
endif()

if (SENPATCHER_BUILD_CS2TEST)
	add_executable(cs2test)
	target_sources(cs2test PRIVATE
		main_cs2_test.cpp

		${SOURCES_BASE_FIXES}
		${SOURCES_CS2_FIXES}

		${SOURCES_LZ4}
		${SOURCES_ZSTD_BASE}
		${SOURCES_ZSTD_COMPRESSION}
	)
	target_include_directories(cs2test
		PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_BINARY_DIR}/senpatcher_embeds"
	)
	add_dependencies(cs2test GenerateGitRevisionHeader)
endif()

if (SENPATCHER_BUILD_CS2HOOK)
	add_library(cs2hook SHARED)
	target_sources(cs2hook PRIVATE
		dinput8.def
		main_cs2.cpp
		ini.cpp
		ini.h
		modload/loaded_mods.cpp
		modload/loaded_mods.h
		x86/emitter.cpp
		x86/emitter.h
		x86/inject_jump_into.h
		x86/page_unprotect.cpp
		x86/page_unprotect.h

		sen2/exe_patch.h
		sen2/exe_patch_add_cs2_to_title_bar.cpp
		sen2/exe_patch_add_senpatcher_version_to_title.cpp
		sen2/exe_patch_deglobalize_mutexes.cpp
		sen2/exe_patch_disable_mouse_capture.cpp
		sen2/exe_patch_disable_pause_on_focus_loss.cpp
		sen2/exe_patch_fix_arts_support_cutin.cpp
		sen2/exe_patch_fix_battle_scope_crash.cpp
		sen2/exe_patch_fix_controller_mappings.cpp
		sen2/exe_patch_fix_gog_galaxy.cpp
		sen2/exe_patch_fix_voice_tables.cpp
		sen2/exe_patch_force_no_kerning.cpp
		sen2/exe_patch_force_xinput.cpp
		sen2/exe_patch_patch_music_fade_timing.cpp
		sen2/exe_patch_patch_music_queueing.cpp
		sen2/exe_patch_remove_debug_leftovers.cpp
		sen2/exe_patch_remove_turbo_auto_skip.cpp
		sen2/exe_patch_show_mouse_cursor.cpp
		sen2/inject_modloader.h
		sen2/inject_modloader.cpp

		${SOURCES_BASE_FIXES}
		${SOURCES_CS2_FIXES}

		${SOURCES_LZ4}
		${SOURCES_ZSTD_BASE}
		${SOURCES_ZSTD_COMPRESSION}
	)
	target_include_directories(cs2hook
		PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_BINARY_DIR}/senpatcher_embeds"
	)
	add_dependencies(cs2hook GenerateGitRevisionHeader)
endif()

if (SENPATCHER_BUILD_CS3TEST)
	add_executable(cs3test)
	target_sources(cs3test PRIVATE
		main_cs3_test.cpp

		${SOURCES_BASE_FIXES}
		${SOURCES_CS3_FIXES}

		${SOURCES_LZ4}
		${SOURCES_ZSTD_BASE}
		${SOURCES_ZSTD_COMPRESSION}
	)
	target_include_directories(cs3test
		PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_BINARY_DIR}/senpatcher_embeds"
	)
	add_dependencies(cs3test GenerateGitRevisionHeader)
endif()

if (SENPATCHER_BUILD_CS3HOOK)
	add_library(cs3hook SHARED)
	target_sources(cs3hook PRIVATE
		dinput8.def
		main_cs3.cpp
		ini.cpp
		ini.h
		modload/loaded_mods.cpp
		modload/loaded_mods.h
		x64/emitter.cpp
		x64/emitter.h
		x64/inject_jump_into.h
		x64/page_unprotect.cpp
		x64/page_unprotect.h

		sen3/exe_patch.h
		sen3/exe_patch_add_senpatcher_version_to_title.cpp
		sen3/exe_patch_allow_switch_to_nightmare.cpp
		sen3/exe_patch_deglobalize_mutexes.cpp
		sen3/exe_patch_disable_mouse_capture.cpp
		sen3/exe_patch_disable_pause_on_focus_loss.cpp
		sen3/exe_patch_fix_controller_mappings.cpp
		sen3/exe_patch_fix_in_game_button_mapping.cpp
		sen3/exe_patch_force_xinput.cpp
		sen3/exe_patch_show_mouse_cursor.cpp
		sen3/exe_patch_swap_broken_mq_values.cpp
		sen3/inject_modloader.h
		sen3/inject_modloader.cpp

		${SOURCES_BASE_FIXES}
		${SOURCES_CS3_FIXES}

		${SOURCES_LZ4}
		${SOURCES_ZSTD_BASE}
		${SOURCES_ZSTD_COMPRESSION}
	)
	target_include_directories(cs3hook
		PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_BINARY_DIR}/senpatcher_embeds"
	)
	add_dependencies(cs3hook GenerateGitRevisionHeader)
endif()

if (SENPATCHER_BUILD_CS4TEST)
	add_executable(cs4test)
	target_sources(cs4test PRIVATE
		main_cs4_test.cpp

		${SOURCES_BASE_FIXES}
		${SOURCES_CS4_FIXES}

		${SOURCES_LZ4}
		${SOURCES_ZSTD_BASE}
		${SOURCES_ZSTD_COMPRESSION}
	)
	target_include_directories(cs4test
		PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_BINARY_DIR}/senpatcher_embeds"
	)
	add_dependencies(cs4test GenerateGitRevisionHeader)
endif()

if (SENPATCHER_BUILD_CS4HOOK)
	add_library(cs4hook SHARED)
	target_sources(cs4hook PRIVATE
		dinput8.def
		main_cs4.cpp
		ini.cpp
		ini.h
		modload/loaded_mods.cpp
		modload/loaded_mods.h
		x64/emitter.cpp
		x64/emitter.h
		x64/inject_jump_into.h
		x64/page_unprotect.cpp
		x64/page_unprotect.h

		sen4/exe_patch.h
		sen4/exe_patch_add_senpatcher_version_to_title.cpp
		sen4/exe_patch_allow_switch_to_nightmare.cpp
		sen4/exe_patch_deglobalize_mutexes.cpp
		sen4/exe_patch_disable_mouse_capture.cpp
		sen4/exe_patch_disable_pause_on_focus_loss.cpp
		sen4/exe_patch_fix_pc_confirm_cancel_when_swapped.cpp
		sen4/exe_patch_force_swap_confirm_cancel.cpp
		sen4/exe_patch_show_mouse_cursor.cpp
		sen4/inject_modloader.cpp
		sen4/inject_modloader.h

		${SOURCES_BASE_FIXES}
		${SOURCES_CS4_FIXES}

		${SOURCES_LZ4}
		${SOURCES_ZSTD_BASE}
		${SOURCES_ZSTD_COMPRESSION}
	)
	target_include_directories(cs4hook
		PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_BINARY_DIR}/senpatcher_embeds"
	)
	add_dependencies(cs4hook GenerateGitRevisionHeader)
endif()

if (SENPATCHER_BUILD_SHA1TEST)
	add_executable(sha1_test)
	target_sources(sha1_test PRIVATE
		main_sha1_test.cpp
		sha1.cpp
		sha1.h
		file.cpp
		file.h
		util/text.cpp
		util/text.h
	)
	target_include_directories(sha1_test
		PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}"
	)
endif()
