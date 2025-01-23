#include "gui_fonts.h"

#include "imgui.h"

#include "gui_state.h"
#include "sen/decompress_helper.h"

namespace {
static constexpr char CuprumFontData[] = {
#include "cuprum.h"
};
static constexpr size_t CuprumFontLength = sizeof(CuprumFontData);
static constexpr char NotoSansJpFontData[] = {
#include "noto_sans_jp.h"
};
static constexpr size_t NotoSansJpFontLength = sizeof(NotoSansJpFontData);
} // namespace

namespace SenTools {
void LoadFonts(ImGuiIO& io, GuiState& state) {
    static std::optional<std::vector<char>> cuprum =
        SenLib::DecompressFromBuffer(CuprumFontData, CuprumFontLength);
    static std::optional<std::vector<char>> noto =
        SenLib::DecompressFromBuffer(NotoSansJpFontData, NotoSansJpFontLength);

    ImFontConfig config;
    config.FontDataOwnedByAtlas = false;
    if (cuprum) {
        io.Fonts->AddFontFromMemoryTTF(cuprum->data(),
                                       static_cast<int>(cuprum->size()),
                                       18.0f,
                                       &config,
                                       io.Fonts->GetGlyphRangesDefault());
        config.MergeMode = true;
    }
    if (noto) {
        io.Fonts->AddFontFromMemoryTTF(noto->data(),
                                       static_cast<int>(noto->size()),
                                       20.0f,
                                       &config,
                                       io.Fonts->GetGlyphRangesJapanese());
    }
}
} // namespace SenTools
