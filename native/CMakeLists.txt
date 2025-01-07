cmake_minimum_required(VERSION 3.24 FATAL_ERROR)

project(SenPatcher LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

option(SENPATCHER_BUILD_TOOLS "build tools executable" ON)
option(SENPATCHER_BUILD_CS1HOOK "build CS1 injection dll" OFF)
option(SENPATCHER_BUILD_CS2HOOK "build CS2 injection dll" OFF)
option(SENPATCHER_BUILD_CS3HOOK "build CS3 injection dll" OFF)
option(SENPATCHER_BUILD_CS4HOOK "build CS4 injection dll" OFF)
option(SENPATCHER_BUILD_REVHOOK "build Reverie injection dll" OFF)
option(SENPATCHER_BUILD_TXHOOK "build Tokyo Xanadu injection dll" OFF)
option(SENPATCHER_BUILD_CS1TEST "build CS1 asset builder executable" OFF)
option(SENPATCHER_BUILD_CS2TEST "build CS2 asset builder executable" OFF)
option(SENPATCHER_BUILD_CS3TEST "build CS3 asset builder executable" OFF)
option(SENPATCHER_BUILD_CS4TEST "build CS4 asset builder executable" OFF)
option(SENPATCHER_BUILD_REVTEST "build Reverie asset builder executable" OFF)
option(SENPATCHER_BUILD_TXTEST "build Tokyo Xanadu asset builder executable" OFF)
option(SENPATCHER_BUILD_GTESTS "build tests" ON)

option(SENPATCHER_WITH_IMGUI "enable imgui GUI for tools" ON)

separate_arguments(SENPATCHER_BUILDFLAGS NATIVE_COMMAND "${SENPATCHER_BUILDFLAGS}")

function(senpatcher_generate_embeddable_file SOURCE_PATH TARGET_NAME)
	get_filename_component(TMP_SENPATCHER_FILEPATH "${SOURCE_PATH}" ABSOLUTE)
	file(SHA256 "${TMP_SENPATCHER_FILEPATH}" TMP_SENPATCHER_FILEHASH)
	file(WRITE "${CMAKE_BINARY_DIR}/senpatcher_embeds/${TARGET_NAME}.sha256.tmp" "${TMP_SENPATCHER_FILEHASH}")
	execute_process(COMMAND "${CMAKE_COMMAND}" -E compare_files "${CMAKE_BINARY_DIR}/senpatcher_embeds/${TARGET_NAME}.sha256" "${CMAKE_BINARY_DIR}/senpatcher_embeds/${TARGET_NAME}.sha256.tmp" RESULT_VARIABLE TMP_SENPATCHER_FILEHASH_EQUAL)
	if (NOT TMP_SENPATCHER_FILEHASH_EQUAL EQUAL 0)
		file(READ "${TMP_SENPATCHER_FILEPATH}" TMP_SENPATCHER_FILE HEX)
		string(REGEX REPLACE "([0-9A-Fa-f][0-9A-Fa-f])" ",(char)0x\\1" TMP_SENPATCHER_FILE "${TMP_SENPATCHER_FILE}")
		string(SUBSTRING "${TMP_SENPATCHER_FILE}" 1 -1 TMP_SENPATCHER_FILE)
		file(GENERATE OUTPUT "${CMAKE_BINARY_DIR}/senpatcher_embeds/${TARGET_NAME}" CONTENT "${TMP_SENPATCHER_FILE}")
		file(WRITE "${CMAKE_BINARY_DIR}/senpatcher_embeds/${TARGET_NAME}.sha256" "${TMP_SENPATCHER_FILEHASH}")
	endif()
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

set(SKIP_INSTALL_ALL ON)
set(ZLIB_BUILD_EXAMPLES OFF)
add_subdirectory(zlib)

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
set(DEFINES_ZSTD -DZSTD_LEGACY_SUPPORT=0 -DZSTD_DISABLE_ASM)
if (MSVC)
	set(DEFINES_ZSTD "${DEFINES_ZSTD} -DZSTD_HEAPMODE=0")
endif()
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

if (SENPATCHER_BUILD_TOOLS)
	add_executable(sentools)
	target_sources(sentools PRIVATE
		sentools/cli_tool.h
		sentools/common_paths.cpp
		sentools/common_paths.h
		sentools/hyouta_archive.cpp
		sentools/hyouta_archive.h
		sentools/main_sentools.cpp
		sentools/old_senpatcher_unpatch.cpp
		sentools/old_senpatcher_unpatch.h
		sentools/bra_extract/bra_extract.cpp
		sentools/bra_extract/bra_extract_main.h
		sentools/dirtree_create/dirtree_create.cpp
		sentools/dirtree_create/dirtree_create_main.h
		sentools/game_verify/game_verify.cpp
		sentools/game_verify/game_verify_main.h
		sentools/p3a_extract/p3a_extract.cpp
		sentools/p3a_extract/p3a_extract_main.h
		sentools/p3a_pack/p3a_pack.cpp
		sentools/p3a_pack/p3a_pack_main.h
		sentools/p3a_repack/p3a_repack.cpp
		sentools/p3a_repack/p3a_repack_main.h
		sentools/pka_extract/pka_extract.cpp
		sentools/pka_extract/pka_extract.h
		sentools/pka_extract/pka_extract_main.h
		sentools/pka_pack/pka_pack.cpp
		sentools/pka_pack/pka_pack_main.h
		sentools/pkg_extract/pkg_extract.cpp
		sentools/pkg_extract/pkg_extract.h
		sentools/pkg_extract/pkg_extract_main.h
		sentools/pkg_pack/pkg_pack.cpp
		sentools/pkg_pack/pkg_pack_main.h
		sentools/pkg_repack/pkg_repack.cpp
		sentools/pkg_repack/pkg_repack_main.h
		sentools/save_checksum_fix/save_checksum_fix.cpp
		sentools/save_checksum_fix/save_checksum_fix.h
		sentools/save_checksum_fix/save_checksum_fix_main.h
		sentools/type1_compress/type1_compress.cpp
		sentools/type1_compress/type1_compress.h
		sentools/type1_compress/type1_compress_main.h
		sentools/type1_decompress/type1_decompress.cpp
		sentools/type1_decompress/type1_decompress.h
		sentools/type1_decompress/type1_decompress_main.h

		util/hash/crc32.h
		util/hash/crc32.c
		util/hash/md5.h
		util/hash/sha1.h
		util/hash/sha1.cpp
		util/hash/sha256.h
		util/hash/sha256.cpp
		util/hash/util.h
		util/align.h
		util/align.cpp
		util/bps.h
		util/bps.cpp
		util/endian.h
		util/file.h
		util/file.cpp
		util/ini.h
		util/ini.cpp
		util/ini_writer.cpp
		util/ini_writer.h
		util/memread.h
		util/memwrite.h
		util/result.h
		util/scope.h
		util/stream.cpp
		util/stream.h
		util/text.cpp
		util/text.h
		util/vector.cpp
		util/vector.h

		p3a/pack.cpp
		p3a/pack.h
		p3a/packfs.cpp
		p3a/packfs.h
		p3a/packjson.cpp
		p3a/packjson.h
		p3a/structs.h
		p3a/unpackfs.cpp
		p3a/unpackfs.h

		sen/decompress_helper.cpp
		sen/decompress_helper.h
		sen/pka.cpp
		sen/pka.h
		sen/pka_to_pkg.cpp
		sen/pka_to_pkg.h
		sen/pkg.cpp
		sen/pkg.h
		sen/pkg_compress.cpp
		sen/pkg_compress.h
		sen/pkg_extract.cpp
		sen/pkg_extract.h
		sen1/system_data.cpp
		sen1/system_data.h
		sen2/system_data.cpp
		sen2/system_data.h

		dirtree/entry.h
		dirtree/tree.h

		dirtree/dirtree_cs1.cpp
		dirtree/dirtree_cs1.h
		dirtree/dirtree_cs2.cpp
		dirtree/dirtree_cs2.h
		dirtree/dirtree_cs3.cpp
		dirtree/dirtree_cs3.h
		dirtree/dirtree_cs4.cpp
		dirtree/dirtree_cs4.h
		dirtree/dirtree_reverie.cpp
		dirtree/dirtree_reverie.h
		dirtree/dirtree_tx.cpp
		dirtree/dirtree_tx.h
		dirtree/generated/internal_dirtree_cs1.h
		dirtree/generated/internal_dirtree_cs2.h
		dirtree/generated/internal_dirtree_cs3.h
		dirtree/generated/internal_dirtree_cs4.h
		dirtree/generated/internal_dirtree_reverie.h
		dirtree/generated/internal_dirtree_tx.h

		cpp-optparse/OptionParser.cpp
		cpp-optparse/OptionParser.h

		${SOURCES_LZ4}
		${SOURCES_LZMA}
		${SOURCES_ZSTD_BASE}
		${SOURCES_ZSTD_COMPRESSION}
		${SOURCES_ZSTD_DICTBUILDER}
		${SOURCES_RAPIDJSON}
	)

	target_compile_definitions(sentools
		PUBLIC P3A_PACKER_WITH_STD_FILESYSTEM FILE_WRAPPER_WITH_STD_FILESYSTEM ${DEFINES_ZSTD}
	)
	if (WIN32)
		target_compile_definitions(sentools PUBLIC BUILD_FOR_WINDOWS UNICODE _UNICODE)
	endif()
	target_compile_options(sentools PRIVATE ${SENPATCHER_BUILDFLAGS})
	target_include_directories(sentools
		PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_BINARY_DIR}/senpatcher_embeds"
	)
	target_link_libraries(sentools PUBLIC zlibstatic)

	if (SENPATCHER_WITH_IMGUI)
		target_sources(sentools PRIVATE
			sentools/gui/gui_file_browser.cpp
			sentools/gui/gui_file_browser.h
			sentools/gui/gui_fonts.cpp
			sentools/gui/gui_fonts.h
			sentools/gui/gui_senpatcher_cs1_system_data_window.cpp
			sentools/gui/gui_senpatcher_cs1_system_data_window.h
			sentools/gui/gui_senpatcher_cs2_system_data_window.cpp
			sentools/gui/gui_senpatcher_cs2_system_data_window.h
			sentools/gui/gui_senpatcher_compress_type1_window.cpp
			sentools/gui/gui_senpatcher_compress_type1_window.h
			sentools/gui/gui_senpatcher_decompress_type1_window.cpp
			sentools/gui/gui_senpatcher_decompress_type1_window.h
			sentools/gui/gui_senpatcher_extract_pka_window.cpp
			sentools/gui/gui_senpatcher_extract_pka_window.h
			sentools/gui/gui_senpatcher_extract_pkg_window.cpp
			sentools/gui/gui_senpatcher_extract_pkg_window.h
			sentools/gui/gui_senpatcher_fix_checksum_window.cpp
			sentools/gui/gui_senpatcher_fix_checksum_window.h
			sentools/gui/gui_senpatcher_main_window.cpp
			sentools/gui/gui_senpatcher_main_window.h
			sentools/gui/gui_senpatcher_patch_cs1_window.cpp
			sentools/gui/gui_senpatcher_patch_cs1_window.h
			sentools/gui/gui_senpatcher_patch_cs2_window.cpp
			sentools/gui/gui_senpatcher_patch_cs2_window.h
			sentools/gui/gui_senpatcher_patch_cs3_window.cpp
			sentools/gui/gui_senpatcher_patch_cs3_window.h
			sentools/gui/gui_senpatcher_patch_cs4_window.cpp
			sentools/gui/gui_senpatcher_patch_cs4_window.h
			sentools/gui/gui_senpatcher_patch_reverie_window.cpp
			sentools/gui/gui_senpatcher_patch_reverie_window.h
			sentools/gui/gui_senpatcher_patch_tx_window.cpp
			sentools/gui/gui_senpatcher_patch_tx_window.h
			sentools/gui/gui_senpatcher_patch_window_utils.cpp
			sentools/gui/gui_senpatcher_patch_window_utils.h
			sentools/gui/gui_state.cpp
			sentools/gui/gui_state.h
			sentools/gui/gui_window.cpp
			sentools/gui/gui_window.h
			sentools/gui/main_gui.cpp
			sentools/gui/main_gui.h

			imgui/imgui.cpp
			imgui/imgui_demo.cpp
			imgui/imgui_draw.cpp
			imgui/imgui_tables.cpp
			imgui/imgui_widgets.cpp
			imgui/misc/cpp/imgui_stdlib.cpp
		)
		target_include_directories(sentools
			PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/imgui" "${CMAKE_CURRENT_SOURCE_DIR}/imgui/backends"
		)
		target_compile_definitions(sentools PUBLIC SENTOOLS_WITH_IMGUI)

		if (WIN32)
			# on Windows we just use DX11 for this, all the Cold Steel games run on DX11
			# so it's kinda guaranteed to be supported for anyone who needs the patcher
			target_sources(sentools PRIVATE
				sentools/gui/gui_setup_dx11.cpp
				sentools/gui/gui_setup_dx11.h
				imgui/backends/imgui_impl_dx11.cpp
				imgui/backends/imgui_impl_win32.cpp
			)
			target_link_libraries(sentools PUBLIC d3d11.lib d3dcompiler.lib)
		else()
			# on Linux there's no obvious choice, so I just went with GLFW + Vulkan
			target_sources(sentools PRIVATE
				sentools/gui/gui_setup_glfw_vulkan.cpp
				sentools/gui/gui_setup_glfw_vulkan.h
				imgui/backends/imgui_impl_glfw.cpp
				imgui/backends/imgui_impl_vulkan.cpp
			)
			find_package(Vulkan REQUIRED)
			target_link_libraries(sentools PUBLIC Vulkan::Vulkan Vulkan::Headers)

			find_package(PkgConfig REQUIRED)
			pkg_check_modules(GLFW3 REQUIRED glfw3>=3.3)
			target_link_libraries(sentools PUBLIC ${GLFW3_LIBRARIES})
			target_include_directories(sentools PUBLIC ${GLFW3_INCLUDE_DIRS})
		endif()
	endif()

	add_dependencies(sentools GenerateGitRevisionHeader)
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
if (SENPATCHER_BUILD_TOOLS AND SENPATCHER_WITH_IMGUI)
	senpatcher_generate_embeddable_file("../fonts/Cuprum/Cuprum-Regular.ttf.bin" "cuprum.h")
	senpatcher_generate_embeddable_file("../fonts/Noto_Sans_JP/NotoSansJP-Medium.ttf.bin" "noto_sans_jp.h")
	senpatcher_generate_embeddable_file("../SenLib/Sen1/senpatcher_settings.ini" "ini_cs1.h")
	senpatcher_generate_embeddable_file("../SenLib/Sen2/senpatcher_settings.ini" "ini_cs2.h")
	senpatcher_generate_embeddable_file("../SenLib/Sen3/senpatcher_settings.ini" "ini_cs3.h")
	senpatcher_generate_embeddable_file("../SenLib/Sen4/senpatcher_settings.ini" "ini_cs4.h")
	senpatcher_generate_embeddable_file("../SenLib/Sen5/senpatcher_settings.ini" "ini_reverie.h")
	senpatcher_generate_embeddable_file("../SenLib/TX/senpatcher_settings.ini" "ini_tx.h")
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
	sen3/file_fixes/alchr022_dat.cpp
	sen3/file_fixes/a0417_dat.cpp
	sen3/file_fixes/book05_dat.cpp
	sen3/file_fixes/book06_dat.cpp
	sen3/file_fixes/book07_dat.cpp
	sen3/file_fixes/c0000_dat.cpp
	sen3/file_fixes/c0200_dat.cpp
	sen3/file_fixes/c0250_dat.cpp
	sen3/file_fixes/c0400_dat.cpp
	sen3/file_fixes/c0420_dat.cpp
	sen3/file_fixes/c0430_dat.cpp
	sen3/file_fixes/c0820_dat.cpp
	sen3/file_fixes/c0830_dat.cpp
	sen3/file_fixes/c2430_dat.cpp
	sen3/file_fixes/c2440_dat.cpp
	sen3/file_fixes/c2610_dat.cpp
	sen3/file_fixes/c3000_dat.cpp
	sen3/file_fixes/c3010_dat.cpp
	sen3/file_fixes/c3210_dat.cpp
	sen3/file_fixes/c3610_dat.cpp
	sen3/file_fixes/f0010_dat.cpp
	sen3/file_fixes/f2000_dat.cpp
	sen3/file_fixes/I_CVIS0061_pkg.cpp
	sen3/file_fixes/I_CVIS1008_pkg.cpp
	sen3/file_fixes/I_JMP009_pkg.cpp
	sen3/file_fixes/insa05.cpp
	sen3/file_fixes/insa08.cpp
	sen3/file_fixes/insa09.cpp
	sen3/file_fixes/m0000_dat.cpp
	sen3/file_fixes/m0100_dat.cpp
	sen3/file_fixes/m0300_dat.cpp
	sen3/file_fixes/m0600_dat.cpp
	sen3/file_fixes/m3000_dat.cpp
	sen3/file_fixes/m3420_dat.cpp
	sen3/file_fixes/m3430_dat.cpp
	sen3/file_fixes/m4004_dat.cpp
	sen3/file_fixes/r0210_dat.cpp
	sen3/file_fixes/r2290_dat.cpp
	sen3/file_fixes/r3000_dat.cpp
	sen3/file_fixes/r3090_dat.cpp
	sen3/file_fixes/r3200_dat.cpp
	sen3/file_fixes/r3430_dat.cpp
	sen3/file_fixes/r4200_dat.cpp
	sen3/file_fixes/r4290_dat.cpp
	sen3/file_fixes/t_item.cpp
	sen3/file_fixes/t_jump.cpp
	sen3/file_fixes/t_magic.cpp
	sen3/file_fixes/t_mons.cpp
	sen3/file_fixes/t_mstqrt.cpp
	sen3/file_fixes/t_name.cpp
	sen3/file_fixes/t_notecook.cpp
	sen3/file_fixes/t_place.cpp
	sen3/file_fixes/t_text.cpp
	sen3/file_fixes/t_vctiming_us.cpp
	sen3/file_fixes/t0000_dat.cpp
	sen3/file_fixes/t0010_dat.cpp
	sen3/file_fixes/t0080_dat.cpp
	sen3/file_fixes/t0100_dat.cpp
	sen3/file_fixes/t0200_dat.cpp
	sen3/file_fixes/t0210_dat.cpp
	sen3/file_fixes/t0250_dat.cpp
	sen3/file_fixes/t0260_dat.cpp
	sen3/file_fixes/t0400_dat.cpp
	sen3/file_fixes/t0410_dat.cpp
	sen3/file_fixes/t3000_dat.cpp
	sen3/file_fixes/t3200_dat.cpp
	sen3/file_fixes/t3220_dat.cpp
	sen3/file_fixes/t3400_dat.cpp
	sen3/file_fixes/t3510_dat.cpp
	sen3/file_fixes/t3600_dat.cpp
	sen3/file_fixes/tk_ada_dat.cpp
	sen3/file_fixes/tk_linde_dat.cpp
	sen3/file_fixes/tk_patrick_dat.cpp
	sen3/file_fixes/tk_stark_dat.cpp
	sen3/file_fixes/tk_tovar_dat.cpp
	sen3/file_fixes/tk_zessica_dat.cpp
	sen3/file_fixes/v0010_dat.cpp
	sen3/file_fixes/v0030_dat.cpp
	sen3/file_fixes/v0050_dat.cpp
	sen3/file_fixes/voice_opus_ps4_103.cpp
	sen3/file_fixes/voice_opus_v00e0441.cpp
	sen3/file_fixes/voice_opus_v00s2728.cpp
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

set(SOURCES_REVERIE_FIXES
	sen5/file_fixes/t_text.cpp
	sen5/file_fixes.cpp
	sen5/file_fixes.h
	sen5/tbl.cpp
	sen5/tbl.h
)

set(SOURCES_TX_FIXES
	tx/file_fixes/a0000.cpp
	tx/file_fixes.cpp
	tx/file_fixes.h
	tx/tbl.cpp
	tx/tbl.h
)

set(SOURCES_BASE_FIXES
	util/align.h
	util/align.cpp
	util/bps.h
	util/bps.cpp
	util/hash/crc32.c
	util/hash/crc32.h
	sen/decompress_helper.cpp
	sen/decompress_helper.h
	util/file.h
	util/file.cpp
	util/ini.cpp
	util/ini.h
	util/logger.cpp
	util/logger.h
	util/hash/sha1.cpp
	util/hash/sha1.h
	util/hash/util.h

	util/endian.h
	util/memread.h
	util/memwrite.h
	util/result.h
	util/stream.cpp
	util/stream.h
	util/text.cpp
	util/text.h
	util/vector.cpp
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
	sen/pka.cpp
	sen/pka.h
	sen/pkg.cpp
	sen/pkg.h
	sen/pkg_extract.cpp
	sen/pkg_extract.h
	sen/sen_script_patcher.cpp
	sen/sen_script_patcher.h

	${SOURCES_LZMA}
)

if (SENPATCHER_BUILD_CS1TEST)
	add_executable(cs1test)
	target_sources(cs1test PRIVATE
		sen1/main_cs1_test.cpp

		modload/loaded_mods.cpp
		modload/loaded_mods.h

		${SOURCES_BASE_FIXES}
		${SOURCES_CS1_FIXES}

		${SOURCES_LZ4}
		${SOURCES_ZSTD_BASE}
		${SOURCES_ZSTD_COMPRESSION}
	)
	target_compile_definitions(cs1test
		PUBLIC ${DEFINES_ZSTD}
	)
	if (WIN32)
		target_compile_definitions(cs1test PUBLIC BUILD_FOR_WINDOWS)
	endif()
	target_compile_options(cs1test PRIVATE ${SENPATCHER_BUILDFLAGS})
	target_include_directories(cs1test
		PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_BINARY_DIR}/senpatcher_embeds"
	)
	add_dependencies(cs1test GenerateGitRevisionHeader)
endif()

if (SENPATCHER_BUILD_CS1HOOK)
	add_library(cs1hook SHARED)
	target_sources(cs1hook PRIVATE
		dinput8.def
		sen1/main_cs1.cpp
		modload/loaded_mods.cpp
		modload/loaded_mods.h
		modload/loaded_pka.cpp
		modload/loaded_pka.h
		x86/emitter.cpp
		x86/emitter.h
		x86/inject_jump_into.h
		x86/page_unprotect.cpp
		x86/page_unprotect.h

		sen1/exe_patch.h
		sen1/exe_patch_add_senpatcher_version_to_title.cpp
		sen1/exe_patch_deglobalize_mutexes.cpp
		sen1/exe_patch_disable_mouse_capture.cpp
		sen1/exe_patch_disable_pause_on_focus_loss.cpp
		sen1/exe_patch_fix_arts_support_cutin.cpp
		sen1/exe_patch_fix_textbox_advance_prompt.cpp
		sen1/exe_patch_fix_voice_tables.cpp
		sen1/exe_patch_force_no_kerning.cpp
		sen1/exe_patch_force_xinput.cpp
		sen1/exe_patch_patch_music_queueing.cpp
		sen1/exe_patch_show_mouse_cursor.cpp
		sen1/exe_patch_thor_mq_string.cpp
		sen1/exe_patch_turbo_mode.cpp
		sen1/inject_modloader.h
		sen1/inject_modloader.cpp

		${SOURCES_BASE_FIXES}
		${SOURCES_CS1_FIXES}

		${SOURCES_LZ4}
		${SOURCES_ZSTD_BASE}
		${SOURCES_ZSTD_COMPRESSION}
	)
	target_compile_definitions(cs1hook
		PUBLIC ${DEFINES_ZSTD} BUILD_FOR_WINDOWS
	)
	target_compile_options(cs1hook PRIVATE ${SENPATCHER_BUILDFLAGS})
	target_include_directories(cs1hook
		PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_BINARY_DIR}/senpatcher_embeds"
	)
	add_dependencies(cs1hook GenerateGitRevisionHeader)
endif()

if (SENPATCHER_BUILD_CS2TEST)
	add_executable(cs2test)
	target_sources(cs2test PRIVATE
		sen2/main_cs2_test.cpp

		modload/loaded_mods.cpp
		modload/loaded_mods.h

		${SOURCES_BASE_FIXES}
		${SOURCES_CS2_FIXES}

		${SOURCES_LZ4}
		${SOURCES_ZSTD_BASE}
		${SOURCES_ZSTD_COMPRESSION}
	)
	target_compile_definitions(cs2test
		PUBLIC ${DEFINES_ZSTD}
	)
	if (WIN32)
		target_compile_definitions(cs2test PUBLIC BUILD_FOR_WINDOWS)
	endif()
	target_compile_options(cs2test PRIVATE ${SENPATCHER_BUILDFLAGS})
	target_include_directories(cs2test
		PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_BINARY_DIR}/senpatcher_embeds"
	)
	add_dependencies(cs2test GenerateGitRevisionHeader)
endif()

if (SENPATCHER_BUILD_CS2HOOK)
	add_library(cs2hook SHARED)
	target_sources(cs2hook PRIVATE
		dinput8.def
		sen2/main_cs2.cpp
		modload/loaded_mods.cpp
		modload/loaded_mods.h
		modload/loaded_pka.cpp
		modload/loaded_pka.h
		x86/emitter.cpp
		x86/emitter.h
		x86/inject_jump_into.h
		x86/page_unprotect.cpp
		x86/page_unprotect.h

		sen2/exe_patch.h
		sen2/exe_patch_add_cs2_to_title_bar.cpp
		sen2/exe_patch_add_senpatcher_version_to_title.cpp
		sen2/exe_patch_deglobalize_mutexes.cpp
		sen2/exe_patch_disable_crash_reporter.cpp
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
		sen2/exe_patch_show_mouse_cursor.cpp
		sen2/exe_patch_turbo_mode.cpp
		sen2/inject_modloader.h
		sen2/inject_modloader.cpp

		${SOURCES_BASE_FIXES}
		${SOURCES_CS2_FIXES}

		${SOURCES_LZ4}
		${SOURCES_ZSTD_BASE}
		${SOURCES_ZSTD_COMPRESSION}
	)
	target_compile_definitions(cs2hook
		PUBLIC ${DEFINES_ZSTD} BUILD_FOR_WINDOWS
	)
	target_compile_options(cs2hook PRIVATE ${SENPATCHER_BUILDFLAGS})
	target_include_directories(cs2hook
		PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_BINARY_DIR}/senpatcher_embeds"
	)
	add_dependencies(cs2hook GenerateGitRevisionHeader)
endif()

if (SENPATCHER_BUILD_TXTEST)
	add_executable(txtest)
	target_sources(txtest PRIVATE
		tx/bra.cpp
		tx/bra.h
		tx/bra_extract.cpp
		tx/bra_extract.h
		tx/main_tx_test.cpp

		modload/loaded_mods.cpp
		modload/loaded_mods.h

		${SOURCES_BASE_FIXES}
		${SOURCES_TX_FIXES}

		tx/file_fixes_sw.cpp
		tx/file_fixes_sw.h
		tx/file_fixes_sw/dungeon_names.h
		tx/file_fixes_sw/e0000.cpp
		tx/file_fixes_sw/e0100.cpp
		tx/file_fixes_sw/e3600.cpp
		tx/file_fixes_sw/e6000.cpp
		tx/file_fixes_sw/e9100.cpp
		tx/file_fixes_sw/m1100.cpp
		tx/file_fixes_sw/m2190.cpp
		tx/file_fixes_sw/m3190.cpp
		tx/file_fixes_sw/m3800.cpp
		tx/file_fixes_sw/m3801.cpp
		tx/file_fixes_sw/m5290.cpp
		tx/file_fixes_sw/m5319.cpp
		tx/file_fixes_sw/m5800.cpp
		tx/file_fixes_sw/m5801.cpp
		tx/file_fixes_sw/m6309.cpp
		tx/file_fixes_sw/m8209.cpp
		tx/file_fixes_sw/m8300.cpp
		tx/file_fixes_sw/m8331.cpp
		tx/file_fixes_sw/m8390.cpp
		tx/file_fixes_sw/m9090.cpp
		tx/file_fixes_sw/m9800.cpp
		tx/file_fixes_sw/m9801.cpp
		tx/file_fixes_sw/m9810.cpp
		tx/file_fixes_sw/m9820.cpp
		tx/file_fixes_sw/m9830.cpp
		tx/file_fixes_sw/m9860.cpp
		tx/file_fixes_sw/m9870.cpp
		tx/file_fixes_sw/s1000.cpp
		tx/file_fixes_sw/s1100.cpp
		tx/file_fixes_sw/s2000.cpp
		tx/file_fixes_sw/s2010.cpp
		tx/file_fixes_sw/s2100.cpp
		tx/file_fixes_sw/s2910.cpp
		tx/file_fixes_sw/s3000.cpp
		tx/file_fixes_sw/s3001.cpp
		tx/file_fixes_sw/s3002.cpp
		tx/file_fixes_sw/s3100.cpp
		tx/file_fixes_sw/s3110.cpp
		tx/file_fixes_sw/s3120.cpp
		tx/file_fixes_sw/s4000.cpp
		tx/file_fixes_sw/s4100.cpp
		tx/file_fixes_sw/s5000.cpp
		tx/file_fixes_sw/s5100.cpp
		tx/file_fixes_sw/s6000.cpp
		tx/file_fixes_sw/s6100.cpp
		tx/file_fixes_sw/s6110.cpp
		tx/file_fixes_sw/s7000.cpp
		tx/file_fixes_sw/s7100.cpp
		tx/file_fixes_sw/s7110.cpp
		tx/file_fixes_sw/s7120.cpp
		tx/file_fixes_sw/s7130.cpp
		tx/file_fixes_sw/s7200.cpp
		tx/file_fixes_sw/s8000.cpp
		tx/file_fixes_sw/system_dat.cpp
		tx/file_fixes_sw/t_active.cpp
		tx/file_fixes_sw/t_dlc.cpp
		tx/file_fixes_sw/t_dungeon.cpp
		tx/file_fixes_sw/t_hikitugi.cpp
		tx/file_fixes_sw/t_item.cpp
		tx/file_fixes_sw/t_itemhelp.cpp
		tx/file_fixes_sw/t_jump.cpp
		tx/file_fixes_sw/t_magic.cpp
		tx/file_fixes_sw/t_main_quest.cpp
		tx/file_fixes_sw/t_notechar.cpp
		tx/file_fixes_sw/t_notehelp.cpp
		tx/file_fixes_sw/t_notemons.cpp
		tx/file_fixes_sw/t_orblv.cpp
		tx/file_fixes_sw/t_place.cpp
		tx/file_fixes_sw/t_text.cpp
		tx/file_fixes_sw/t1000.cpp
		tx/file_fixes_sw/t1001.cpp
		tx/file_fixes_sw/t1110.cpp
		tx/file_fixes_sw/t1120.cpp
		tx/file_fixes_sw/t1130.cpp
		tx/file_fixes_sw/t1210.cpp
		tx/file_fixes_sw/t1310.cpp
		tx/file_fixes_sw/t1410.cpp
		tx/file_fixes_sw/t2000.cpp
		tx/file_fixes_sw/t2010.cpp
		tx/file_fixes_sw/t2100.cpp
		tx/file_fixes_sw/t2200.cpp
		tx/file_fixes_sw/t2300.cpp
		tx/file_fixes_sw/t2400.cpp
		tx/file_fixes_sw/t3000.cpp
		tx/file_fixes_sw/t3100.cpp
		tx/file_fixes_sw/t3200.cpp
		tx/file_fixes_sw/t3300.cpp
		tx/file_fixes_sw/t4000.cpp
		tx/file_fixes_sw/t4100.cpp
		tx/file_fixes_sw/t4200.cpp
		tx/file_fixes_sw/t4300.cpp
		tx/file_fixes_sw/t4400.cpp
		tx/file_fixes_sw/t5000.cpp
		tx/file_fixes_sw/t5110.cpp
		tx/file_fixes_sw/t5120.cpp
		tx/file_fixes_sw/t5130.cpp
		tx/file_fixes_sw/t6000.cpp
		tx/file_fixes_sw/t6010.cpp
		tx/file_fixes_sw/t6100.cpp
		tx/file_fixes_sw/t6200.cpp
		tx/file_fixes_sw/t6300.cpp
		tx/file_fixes_sw/t6400.cpp
		tx/file_fixes_sw/t6500.cpp
		tx/file_fixes_sw/t6600.cpp
		tx/file_fixes_sw/t6700.cpp
		tx/file_fixes_sw/tk_akie.cpp
		tx/file_fixes_sw/tk_gorou.cpp
		tx/file_fixes_sw/tk_haruhiko.cpp
		tx/file_fixes_sw/tk_hekiru.cpp
		tx/file_fixes_sw/tk_nodoka.cpp
		tx/file_fixes_sw/tk_rion.cpp

		${SOURCES_LZ4}
		${SOURCES_ZSTD_BASE}
		${SOURCES_ZSTD_COMPRESSION}
	)
	target_compile_definitions(txtest
		PUBLIC ${DEFINES_ZSTD} FILE_WRAPPER_WITH_STD_FILESYSTEM
	)
	if (WIN32)
		target_compile_definitions(txtest PUBLIC BUILD_FOR_WINDOWS)
	endif()
	target_compile_options(txtest PRIVATE ${SENPATCHER_BUILDFLAGS})
	target_include_directories(txtest
		PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_BINARY_DIR}/senpatcher_embeds"
	)
	target_link_libraries(txtest PUBLIC zlibstatic)
	add_dependencies(txtest GenerateGitRevisionHeader)
endif()

if (SENPATCHER_BUILD_TXHOOK)
	add_library(txhook SHARED)
	target_sources(txhook PRIVATE
		dinput8.def
		tx/main_tx.cpp
		modload/loaded_mods.cpp
		modload/loaded_mods.h
		modload/loaded_pka.cpp
		modload/loaded_pka.h
		x86/emitter.cpp
		x86/emitter.h
		x86/inject_jump_into.h
		x86/page_unprotect.cpp
		x86/page_unprotect.h

		tx/bra.cpp
		tx/bra.h
		tx/bra_extract.cpp
		tx/bra_extract.h
		tx/exe_patch.h
		tx/exe_patch_add_senpatcher_version_to_title.cpp
		tx/exe_patch_bgm_resume.cpp
		tx/exe_patch_custom_dlc_multiuse.cpp
		tx/exe_patch_disable_camera_auto_center.cpp
		tx/exe_patch_disable_mouse_camera.cpp
		tx/exe_patch_enable_background_input.cpp
		tx/exe_patch_enable_wav.cpp
		tx/exe_patch_force_mg04_uvs.cpp
		tx/exe_patch_increase_dlc_count.cpp
		tx/exe_patch_override_language.cpp
		tx/exe_patch_show_mouse_cursor.cpp
		tx/exe_patch_skip_movies.cpp
		tx/exe_patch_turbo_mode.cpp
		tx/inject_modloader.h
		tx/inject_modloader.cpp

		${SOURCES_BASE_FIXES}
		${SOURCES_TX_FIXES}

		${SOURCES_LZ4}
		${SOURCES_ZSTD_BASE}
		${SOURCES_ZSTD_COMPRESSION}
	)
	target_compile_definitions(txhook
		PUBLIC ${DEFINES_ZSTD} BUILD_FOR_WINDOWS
	)
	target_compile_options(txhook PRIVATE ${SENPATCHER_BUILDFLAGS})
	target_include_directories(txhook
		PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_BINARY_DIR}/senpatcher_embeds"
	)
	target_link_libraries(txhook PUBLIC zlibstatic)
	add_dependencies(txhook GenerateGitRevisionHeader)
endif()

if (SENPATCHER_BUILD_CS3TEST)
	add_executable(cs3test)
	target_sources(cs3test PRIVATE
		sen3/main_cs3_test.cpp

		modload/loaded_mods.cpp
		modload/loaded_mods.h

		${SOURCES_BASE_FIXES}
		${SOURCES_CS3_FIXES}

		${SOURCES_LZ4}
		${SOURCES_ZSTD_BASE}
		${SOURCES_ZSTD_COMPRESSION}
	)
	target_compile_definitions(cs3test
		PUBLIC ${DEFINES_ZSTD}
	)
	if (WIN32)
		target_compile_definitions(cs3test PUBLIC BUILD_FOR_WINDOWS)
	endif()
	target_compile_options(cs3test PRIVATE ${SENPATCHER_BUILDFLAGS})
	target_include_directories(cs3test
		PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_BINARY_DIR}/senpatcher_embeds"
	)
	add_dependencies(cs3test GenerateGitRevisionHeader)
endif()

if (SENPATCHER_BUILD_CS3HOOK)
	add_library(cs3hook SHARED)
	target_sources(cs3hook PRIVATE
		dinput8.def
		sen3/main_cs3.cpp
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
		sen3/exe_patch_custom_dlc_multiuse.cpp
		sen3/exe_patch_deglobalize_mutexes.cpp
		sen3/exe_patch_disable_crash_reporter.cpp
		sen3/exe_patch_disable_mouse_capture.cpp
		sen3/exe_patch_disable_pause_on_focus_loss.cpp
		sen3/exe_patch_dlc_costume_crash.cpp
		sen3/exe_patch_dlc_load_french_bounds.cpp
		sen3/exe_patch_fix_controller_mappings.cpp
		sen3/exe_patch_fix_in_game_button_mapping.cpp
		sen3/exe_patch_force_xinput.cpp
		sen3/exe_patch_increase_dlc_count.cpp
		sen3/exe_patch_patch_music_queueing.cpp
		sen3/exe_patch_show_mouse_cursor.cpp
		sen3/exe_patch_swap_broken_mq_values.cpp
		sen3/exe_patch_turbo_mode.cpp
		sen3/inject_modloader.h
		sen3/inject_modloader.cpp

		${SOURCES_BASE_FIXES}
		${SOURCES_CS3_FIXES}

		${SOURCES_LZ4}
		${SOURCES_ZSTD_BASE}
		${SOURCES_ZSTD_COMPRESSION}
	)
	target_compile_definitions(cs3hook
		PUBLIC ${DEFINES_ZSTD} BUILD_FOR_WINDOWS
	)
	target_compile_options(cs3hook PRIVATE ${SENPATCHER_BUILDFLAGS})
	target_include_directories(cs3hook
		PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_BINARY_DIR}/senpatcher_embeds"
	)
	add_dependencies(cs3hook GenerateGitRevisionHeader)
endif()

if (SENPATCHER_BUILD_CS4TEST)
	add_executable(cs4test)
	target_sources(cs4test PRIVATE
		sen4/main_cs4_test.cpp

		modload/loaded_mods.cpp
		modload/loaded_mods.h

		${SOURCES_BASE_FIXES}
		${SOURCES_CS4_FIXES}

		${SOURCES_LZ4}
		${SOURCES_ZSTD_BASE}
		${SOURCES_ZSTD_COMPRESSION}
	)
	target_compile_definitions(cs4test
		PUBLIC ${DEFINES_ZSTD}
	)
	if (WIN32)
		target_compile_definitions(cs4test PUBLIC BUILD_FOR_WINDOWS)
	endif()
	target_compile_options(cs4test PRIVATE ${SENPATCHER_BUILDFLAGS})
	target_include_directories(cs4test
		PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_BINARY_DIR}/senpatcher_embeds"
	)
	add_dependencies(cs4test GenerateGitRevisionHeader)
endif()

if (SENPATCHER_BUILD_CS4HOOK)
	add_library(cs4hook SHARED)
	target_sources(cs4hook PRIVATE
		dinput8.def
		sen4/main_cs4.cpp
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
		sen4/exe_patch_custom_dlc_multiuse.cpp
		sen4/exe_patch_deglobalize_mutexes.cpp
		sen4/exe_patch_disable_mouse_capture.cpp
		sen4/exe_patch_disable_pause_on_focus_loss.cpp
		sen4/exe_patch_dlc_costume_crash.cpp
		sen4/exe_patch_fix_pc_confirm_cancel_when_swapped.cpp
		sen4/exe_patch_force_swap_confirm_cancel.cpp
		sen4/exe_patch_increase_dlc_count.cpp
		sen4/exe_patch_patch_music_queueing.cpp
		sen4/exe_patch_show_mouse_cursor.cpp
		sen4/inject_modloader.cpp
		sen4/inject_modloader.h

		${SOURCES_BASE_FIXES}
		${SOURCES_CS4_FIXES}

		${SOURCES_LZ4}
		${SOURCES_ZSTD_BASE}
		${SOURCES_ZSTD_COMPRESSION}
	)
	target_compile_definitions(cs4hook
		PUBLIC ${DEFINES_ZSTD} BUILD_FOR_WINDOWS
	)
	target_compile_options(cs4hook PRIVATE ${SENPATCHER_BUILDFLAGS})
	target_include_directories(cs4hook
		PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_BINARY_DIR}/senpatcher_embeds"
	)
	add_dependencies(cs4hook GenerateGitRevisionHeader)
endif()

if (SENPATCHER_BUILD_REVTEST)
	add_executable(reverietest)
	target_sources(reverietest PRIVATE
		sen5/main_reverie_test.cpp

		modload/loaded_mods.cpp
		modload/loaded_mods.h

		${SOURCES_BASE_FIXES}
		${SOURCES_REVERIE_FIXES}

		${SOURCES_LZ4}
		${SOURCES_ZSTD_BASE}
		${SOURCES_ZSTD_COMPRESSION}
	)
	target_compile_definitions(reverietest
		PUBLIC ${DEFINES_ZSTD}
	)
	if (WIN32)
		target_compile_definitions(reverietest PUBLIC BUILD_FOR_WINDOWS)
	endif()
	target_compile_options(reverietest PRIVATE ${SENPATCHER_BUILDFLAGS})
	target_include_directories(reverietest
		PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_BINARY_DIR}/senpatcher_embeds"
	)
	add_dependencies(reverietest GenerateGitRevisionHeader)
endif()

if (SENPATCHER_BUILD_REVHOOK)
	add_library(reveriehook SHARED)
	target_sources(reveriehook PRIVATE
		dsound.def
		sen5/main_reverie.cpp
		modload/loaded_mods.cpp
		modload/loaded_mods.h
		x64/emitter.cpp
		x64/emitter.h
		x64/inject_jump_into.h
		x64/page_unprotect.cpp
		x64/page_unprotect.h

		sen5/exe_patch.h
		sen5/exe_patch_add_senpatcher_version_to_title.cpp
		sen5/exe_patch_custom_dlc_multiuse.cpp
		sen5/exe_patch_disable_fps_limit_on_focus_loss.cpp
		sen5/exe_patch_disable_mouse_capture.cpp
		sen5/exe_patch_dlc_costume_crash.cpp
		sen5/exe_patch_increase_dlc_count.cpp
		sen5/exe_patch_patch_music_queueing.cpp
		sen5/exe_patch_show_mouse_cursor.cpp
		sen5/inject_modloader.cpp
		sen5/inject_modloader.h

		${SOURCES_BASE_FIXES}
		${SOURCES_REVERIE_FIXES}

		${SOURCES_LZ4}
		${SOURCES_ZSTD_BASE}
		${SOURCES_ZSTD_COMPRESSION}
	)
	target_compile_definitions(reveriehook
		PUBLIC ${DEFINES_ZSTD} BUILD_FOR_WINDOWS
	)
	target_compile_options(reveriehook PRIVATE ${SENPATCHER_BUILDFLAGS})
	target_include_directories(reveriehook
		PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_BINARY_DIR}/senpatcher_embeds"
	)
	add_dependencies(reveriehook GenerateGitRevisionHeader)
endif()

if (SENPATCHER_BUILD_GTESTS)
	enable_testing()
	set(BUILD_GMOCK OFF CACHE BOOL "" FORCE)
	set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
	set(GTEST_HAS_ABSL OFF CACHE BOOL "" FORCE)
	add_subdirectory(gtest)

	add_executable(tests)
	target_sources(tests PRIVATE
		${SOURCES_BASE_FIXES}
		${SOURCES_LZ4}
		${SOURCES_ZSTD_BASE}
		${SOURCES_ZSTD_COMPRESSION}
		modload/loaded_mods.cpp
		modload/loaded_mods.h
		util/hash/sha256.h
		util/hash/sha256.cpp
		util/hash/util.h
		util/text.cpp
		util/text.h
		util/xorshift.h

		test/ini_test.cpp
		test/p3a_lookup_test.cpp
		test/sen_script_patcher_test.cpp
		test/sha1_test.cpp
		test/sha256_test.cpp
		test/text_case_test.cpp
		test/text_encoding_test.cpp
		test/text_glob_test.cpp
	)
	target_compile_definitions(tests
		PUBLIC ${DEFINES_ZSTD}
	)
	if (WIN32)
		target_compile_definitions(tests PUBLIC BUILD_FOR_WINDOWS)
	endif()
	target_include_directories(tests
		PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_BINARY_DIR}/senpatcher_embeds" "gtest/googletest/include"
	)
	add_dependencies(tests GenerateGitRevisionHeader)
	target_link_libraries(tests PUBLIC GTest::gtest_main)

	include(GoogleTest)
	gtest_discover_tests(tests)
endif()
