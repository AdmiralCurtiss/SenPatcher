#pragma once

namespace SenTools {
struct GuiState;
}

namespace SenTools::GUI {
struct Window {
    virtual ~Window();

    // returns false if the window should be deleted
    virtual bool RenderFrame(GuiState& state) = 0;

    // called before the window is deleted
    virtual void Cleanup(GuiState& state) = 0;
};
} // namespace SenTools::GUI
