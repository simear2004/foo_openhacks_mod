#include "pch.h"
#include "hacks_vars.h"
#include "win32_utils.h"
#include <string>

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

// Path variables implementation
std::string g_fb2k_root;
std::string g_fb2k_profile;

std::string ResolvePathVariables(const char* input)
{
    if (!input) return "";
    std::string result(input);
    
    auto replaceAll = [](std::string& str, const std::string& from, const std::string& to) {
        size_t start_pos = 0;
        while((start_pos = str.find(from, start_pos)) != std::string::npos) {
            str.replace(start_pos, from.length(), to);
            start_pos += to.length(); 
        }
    };

    replaceAll(result, "%fb2k%", g_fb2k_root);
    replaceAll(result, "%fb2k_profile%", g_fb2k_profile);
    return result;
}

void InjectEnvironmentVariables()
{
    if (g_fb2k_root.empty() || g_fb2k_profile.empty())
    {
        console::warning("[OpenHacks] Environment variables not injected: paths are empty");
        return;
    }

    BOOL result1 = SetEnvironmentVariableA("fb2k", g_fb2k_root.c_str());
    BOOL result2 = SetEnvironmentVariableA("fb2k_profile", g_fb2k_profile.c_str());

    if (result1 && result2)
    {
        console::printf("[OpenHacks] ✓ Environment variables set successfully");
        console::printf("[OpenHacks]   %%fb2k%% = %s", g_fb2k_root.c_str());
        console::printf("[OpenHacks]   %%fb2k_profile%% = %s", g_fb2k_profile.c_str());
        
        // Verify by reading back
        char verifyFb2k[MAX_PATH] = {0};
        char verifyProfile[MAX_PATH] = {0};
        DWORD len1 = GetEnvironmentVariableA("fb2k", verifyFb2k, MAX_PATH);
        DWORD len2 = GetEnvironmentVariableA("fb2k_profile", verifyProfile, MAX_PATH);
        
        if (len1 > 0 && len2 > 0) {
            console::printf("[OpenHacks] ✓ Verification successful - variables are readable");
            console::printf("[OpenHacks]   Read back: fb2k=%s", verifyFb2k);
            console::printf("[OpenHacks]   Read back: fb2k_profile=%s", verifyProfile);
            
            // Test path resolution
            std::string testPath1 = "%fb2k%\\foobar2000.exe";
            std::string resolved1 = ResolvePathVariables(testPath1.c_str());
            console::printf("[OpenHacks]   Path test: %s -> %s", testPath1.c_str(), resolved1.c_str());
            
            std::string testPath2 = "%fb2k_profile%\\config.txt";
            std::string resolved2 = ResolvePathVariables(testPath2.c_str());
            console::printf("[OpenHacks]   Path test: %s -> %s", testPath2.c_str(), resolved2.c_str());
        } else {
            console::error("[OpenHacks] ✗ Verification failed - cannot read back variables");
        }
    }
    else
    {
        console::error("[OpenHacks] ✗ Failed to set environment variables");
        if (!result1) {
            DWORD err = GetLastError();
            console::error("[OpenHacks]   Failed to set fb2k (Error: %lu)", err);
        }
        if (!result2) {
            DWORD err = GetLastError();
            console::error("[OpenHacks]   Failed to set fb2k_profile (Error: %lu)", err);
        }
    }
}

void InitialseOpenHacksVars()
{
    // Initialize paths
    const char* dllPath = core_api::get_my_full_path();
    bool isPortable = core_api::is_portable_mode_enabled();
    
    console::printf("[OpenHacks] Installation mode: %s", isPortable ? "Portable" : "Standard");
    console::printf("[OpenHacks] DLL path: %s", dllPath ? dllPath : "(null)");
    
    if (dllPath) {
        std::string dllFullPath(dllPath);
        
        if (isPortable) {
            // Portable mode: profile is under foobar2000 root
            // DLL path: <fb2k_root>\profile\user-components-x64\foo_openhacks_mod\foo_openhacks_mod.dll
            // Need to go up 3 levels to reach fb2k root
            
            std::string currentPath = dllFullPath;
            for (int i = 0; i < 3; i++) {
                size_t pos = currentPath.find_last_of('\\');
                if (pos != std::string::npos) {
                    currentPath = currentPath.substr(0, pos);
                } else {
                    break;
                }
            }
            g_fb2k_root = currentPath;
            
            console::printf("[OpenHacks] Portable mode detected, calculated root: %s", g_fb2k_root.c_str());
        } else {
            // Standard mode: need to find foobar2000.exe location
            // DLL is in user's AppData, but exe is in Program Files or custom install location
            
            // Strategy: Check common locations where foobar2000.exe might be
            // 1. Same drive as DLL (unlikely for standard install)
            // 2. Use registry or known paths
            
            // For standard install, we can try to get it from the module handle
            HMODULE hModule = GetModuleHandleA(nullptr);
            if (hModule) {
                char exePath[MAX_PATH] = {0};
                if (GetModuleFileNameA(hModule, exePath, MAX_PATH)) {
                    std::string exeFullPath(exePath);
                    size_t pos = exeFullPath.find_last_of('\\');
                    if (pos != std::string::npos) {
                        g_fb2k_root = exeFullPath.substr(0, pos);
                        console::printf("[OpenHacks] Standard mode, found exe at: %s", g_fb2k_root.c_str());
                    }
                }
            }
            
            // Fallback: if we still don't have it, try to extract from DLL path
            if (g_fb2k_root.empty()) {
                // This shouldn't happen in standard mode, but just in case
                console::warning("[OpenHacks] Could not determine fb2k root in standard mode");
            }
        }
    }

    const char* profilePath = core_api::get_profile_path();
    if (profilePath) {
        g_fb2k_profile = profilePath;
        if (g_fb2k_profile.length() >= 7 && g_fb2k_profile.substr(0, 7) == "file://") {
            g_fb2k_profile = g_fb2k_profile.substr(7);
        }
        console::printf("[OpenHacks] Profile path from API: %s", g_fb2k_profile.c_str());
    }
    
    // Inject environment variables
    InjectEnvironmentVariables();
    
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
