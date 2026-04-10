#include "pch.h"
#include "hacks_vars.h"
#include "win32_utils.h"
#include <windows.h>
#include <string>
#include "initquit.h"

namespace OpenHacksVars
{
// {A1B2C3D4-E5F6-7890-ABCD-EF1234567890}
static const GUID cfg_guid_show_main_menu = {0xa1b2c3d4, 0xe5f6, 0x7890, {0xab, 0xcd, 0xef, 0x12, 0x34, 0x56, 0x78, 0x90}};
// {B2C3D4E5-F6A7-8901-BCDE-F12345678901}
static const GUID cfg_guid_show_status_bar = {0xb2c3d4e5, 0xf6a7, 0x8901, {0xbc, 0xde, 0xf1, 0x23, 0x45, 0x67, 0x89, 0x01}};
// {C3D4E5F6-A7B8-9012-CDEF-123456789012}
static const GUID cfg_guid_main_window_frame_style = {0xc3d4e5f6, 0xa7b8, 0x9012, {0xcd, 0xef, 0x12, 0x34, 0x56, 0x78, 0x90, 0x12}};
// {D4E5F6A7-B8C9-0123-DEF1-234567890123}
static const GUID cfg_guid_enable_win10_shadow = {0xd4e5f6a7, 0xb8c9, 0x0123, {0xde, 0xf1, 0x23, 0x45, 0x67, 0x89, 0x01, 0x23}};
// {E5F6A7B8-C9D0-1234-EF12-345678901234}
static const GUID cfg_guid_disable_resize_maximized = {0xe5f6a7b8, 0xc9d0, 0x1234, {0xef, 0x12, 0x34, 0x56, 0x78, 0x90, 0x12, 0x34}};
// {F6A7B8C9-D0E1-2345-F123-456789012345}
static const GUID cfg_guid_disable_resize_fullscreen = {0xf6a7b8c9, 0xd0e1, 0x2345, {0xf1, 0x23, 0x45, 0x67, 0x89, 0x01, 0x23, 0x45}};
// {A7B8C9D0-E1F2-3456-1234-567890123456}
static const GUID cfg_guid_pseudo_caption = {0xa7b8c9d0, 0xe1f2, 0x3456, {0x12, 0x34, 0x56, 0x78, 0x90, 0x12, 0x34, 0x56}};
// {B8C9D0E1-F2A3-4567-2345-678901234567}
static const GUID cfg_guid_saved_window_state = {0xb8c9d0e1, 0xf2a3, 0x4567, {0x23, 0x45, 0x67, 0x89, 0x01, 0x23, 0x45, 0x67}};

cfg_bool ShowMainMenu(cfg_guid_show_main_menu, true);
cfg_bool ShowStatusBar(cfg_guid_show_status_bar, true);
cfg_int MainWindowFrameStyle(cfg_guid_main_window_frame_style, 0);
cfg_bool EnableWin10Shadow(cfg_guid_enable_win10_shadow, true);
cfg_bool DisableResizeWhenMaximized(cfg_guid_disable_resize_maximized, true);
cfg_bool DisableResizeWhenFullscreen(cfg_guid_disable_resize_fullscreen, true);
cfg_struct_t<PseudoCaptionParam> PseudoCaptionSettings(cfg_guid_pseudo_caption);
cfg_struct_t<WindowStateData> SavedWindowState(cfg_guid_saved_window_state);

// runtime vars
uint32_t DPI = USER_DEFAULT_SCREEN_DPI;

void InitialseOpenHacksVars()
{
    try 
    {
        std::string fb2k_root;
        const char* dllPath = core_api::get_my_full_path();
        if (dllPath) {
            std::string path(dllPath);
            size_t slash = path.find_last_of('\\');
            if (slash != std::string::npos) {
                fb2k_root = path.substr(0, slash);
            }
        }

        std::string fb2k_profile;
        const char* profilePath = core_api::get_profile_path();
        if (profilePath) {
            fb2k_profile = profilePath;
            if (fb2k_profile.length() >= 7 && fb2k_profile.substr(0, 7) == "file://") {
                fb2k_profile = fb2k_profile.substr(7);
            }
        }

        if (!fb2k_root.empty()) {
            SetEnvironmentVariableA("fb2k", fb2k_root.c_str());
            SetEnvironmentVariableA("FB2K", fb2k_root.c_str());
        }
        
        if (!fb2k_profile.empty()) {
            SetEnvironmentVariableA("fb2k_profile", fb2k_profile.c_str());
            SetEnvironmentVariableA("FB2K_PROFILE", fb2k_profile.c_str());
        }

        auto& pseudoCaption = PseudoCaptionSettings.get_value();
        if (pseudoCaption.height == 0)
        {
            auto height = Utility::GetSystemMetricsForDpi(SM_CYCAPTION, Utility::GetDPI(HWND_DESKTOP));
            height += Utility::GetSystemMetricsForDpi(SM_CYFRAME, Utility::GetDPI(HWND_DESKTOP));
            height += Utility::GetSystemMetricsForDpi(SM_CXPADDEDBORDER, Utility::GetDPI(HWND_DESKTOP));
            pseudoCaption.height = height;
        }
    }
    catch (...) 
    {
        OutputDebugStringA("[OpenHacks] Initialization failed due to an exception.\n");
    }
}

} // namespace OpenHacksVars

class COpenHacksAutoInit : public init_callback_v2 {
public:
    void on_init() override {
        OpenHacksVars::InitialseOpenHacksVars();
    }
    void on_quit() override {}
};

static service_factory_single_t<COpenHacksAutoInit> g_auto_init_factory;
