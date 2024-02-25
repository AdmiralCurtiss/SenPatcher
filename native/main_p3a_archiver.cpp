#include <filesystem>

#include "p3a/pack.h"
#include "p3a/structs.h"
#include "p3a/unpack.h"

int main(int argc, char** argv) {
    bool rv = true;
    rv &= SenPatcher::UnpackP3A(L"c:\\__ed8\\p3a\\script.p3a",
                                L"c:\\__ed8\\p3a\\script.p3a.ex",
                                L"c:\\__ed8\\p3a\\script.p3a.dict");
    rv &= SenPatcher::PackP3AFromDirectory(L"c:\\__ed8\\p3a\\script.p3a.ex",
                                           L"c:\\__ed8\\p3a\\script_new_none.p3a",
                                           SenPatcher::P3ACompressionType::None);
    rv &= SenPatcher::PackP3AFromDirectory(L"c:\\__ed8\\p3a\\script.p3a.ex",
                                           L"c:\\__ed8\\p3a\\script_new_lz4.p3a",
                                           SenPatcher::P3ACompressionType::LZ4);
    rv &= SenPatcher::PackP3AFromDirectory(L"c:\\__ed8\\p3a\\script.p3a.ex",
                                           L"c:\\__ed8\\p3a\\script_new_zstd.p3a",
                                           SenPatcher::P3ACompressionType::ZSTD);
    rv &= SenPatcher::PackP3AFromDirectory(L"c:\\__ed8\\p3a\\script.p3a.ex",
                                           L"c:\\__ed8\\p3a\\script_new_zstd_dict.p3a",
                                           SenPatcher::P3ACompressionType::ZSTD_DICT,
                                           L"c:\\__ed8\\p3a\\script.p3a.dict");
    rv &= SenPatcher::UnpackP3A(L"c:\\__ed8\\p3a\\script_new_none.p3a",
                                L"c:\\__ed8\\p3a\\script_new_none.p3a.ex",
                                L"c:\\__ed8\\p3a\\script_new_none.p3a.dict");
    rv &= SenPatcher::UnpackP3A(L"c:\\__ed8\\p3a\\script_new_lz4.p3a",
                                L"c:\\__ed8\\p3a\\script_new_lz4.p3a.ex",
                                L"c:\\__ed8\\p3a\\script_new_lz4.p3a.dict");
    rv &= SenPatcher::UnpackP3A(L"c:\\__ed8\\p3a\\script_new_zstd.p3a",
                                L"c:\\__ed8\\p3a\\script_new_zstd.p3a.ex",
                                L"c:\\__ed8\\p3a\\script_new_zstd.p3a.dict");
    rv &= SenPatcher::UnpackP3A(L"c:\\__ed8\\p3a\\script_new_zstd_dict.p3a",
                                L"c:\\__ed8\\p3a\\script_new_zstd_dict.p3a.ex",
                                L"c:\\__ed8\\p3a\\script_new_zstd_dict.p3a.dict");
    return rv ? 0 : -1;
}
