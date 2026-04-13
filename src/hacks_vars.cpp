#include "pch.h"
#include "hacks_vars.h"
#include "win32_utils.h"
#include <windows.h>
#include <string>

namespace OpenHacksVars
{
    std::string g_fb2k_root;
    std::string g_fb2k_profile;

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
        const char* dllPath = core_api::get_my_full_path();
        bool isPortable = core_api::is_portable_mode_enabled();

        if (dllPath) {
            std::string path(dllPath);
            if (isPortable) {
                for (int i = 0; i < 4; i++) {
                    size_t pos = path.find_last_of('\\');
                    if (pos != std::string::npos) path = path.substr(0, pos);
                }
                g_fb2k_root = path;
            } else {
                char exePath[MAX_PATH] = {0};
                if (GetModuleFileNameA(NULL, exePath, MAX_PATH)) {
                    std::string exeStr(exePath);
                    size_t pos = exeStr.find_last_of('\\');
                    if (pos != std::string::npos) g_fb2k_root = exeStr.substr(0, pos);
                }
            }
        }

        const char* profilePath = core_api::get_profile_path();
        if (profilePath) {
            g_fb2k_profile = profilePath;
            if (g_fb2k_profile.length() >= 7 && g_fb2k_profile.substr(0, 7) == "file://") {
                g_fb2k_profile = g_fb2k_profile.substr(7);
            }
        }

        if (!g_fb2k_root.empty()) {
            SetEnvironmentVariableA("fb2k", g_fb2k_root.c_str());
            SetEnvironmentVariableA("foobar2000", g_fb2k_root.c_str());
            //console::printf("[OpenHacks] Env var 'fb2k' injected: %s", g_fb2k_root.c_str());
        }

        if (!g_fb2k_profile.empty()) {
            SetEnvironmentVariableA("fb2k_profile", g_fb2k_profile.c_str());
            //console::printf("[OpenHacks] Env var 'fb2k_profile' injected: %s", g_fb2k_profile.c_str());
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

} // namespace OpenHacksVars

// --- Custom Path Field Provider Implementation ---
static const struct {
    const char* name;
} kDisplayFields[] = {
    {"fb2k"},
    {"foobar2000"},
    {"fb2k_profile"}
};

uint32_t custom_path_field_provider::get_field_count() {
    return _countof(kDisplayFields);
}

void custom_path_field_provider::get_field_name(uint32_t index, pfc::string_base& out) {
    if (index < get_field_count()) {
        out.set_string(kDisplayFields[index].name);
    } else {
        out.reset();
    }
}

bool custom_path_field_provider::process_field(uint32_t index, metadb_handle* handle, titleformat_text_out* out) {
    if (index >= get_field_count()) return false;

    const char* path_str = nullptr;
    
    switch (index) {
    case 0: // fb2k
    case 1: // foobar2000 (Same as fb2k)
        if (!OpenHacksVars::g_fb2k_root.empty()) {
            path_str = OpenHacksVars::g_fb2k_root.c_str();
        }
        break;
    case 2: // fb2k_profile
        if (!OpenHacksVars::g_fb2k_profile.empty()) {
            path_str = OpenHacksVars::g_fb2k_profile.c_str();
        }
        break;
    }

    if (path_str) {
        //console::formatter() << "[OpenHacks Debug] %" << kDisplayFields[index].name << "% resolved to: " << path_str << "\n";
        out->write(titleformat_inputtypes::unknown, path_str, strlen(path_str));
        return true;
    }
    
    return false;
}

static service_factory_single_t<custom_path_field_provider> g_custom_path_field_provider;
