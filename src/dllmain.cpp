// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"
#include "hacks_version.h"
#include <libPPUI/Controls.h>
#include "hacks_com_impl.h"
#include <windows.h>
#include <string>

BOOL APIENTRY DllMain(HMODULE mod, DWORD reason, LPVOID /*reserved*/)
{
    switch (reason)
    {
    case DLL_PROCESS_ATTACH:
        DisableThreadLibraryCalls(mod);
        
        {
            char exePath[MAX_PATH] = {0};
            if (GetModuleFileNameA(NULL, exePath, MAX_PATH)) {
                std::string path(exePath);
                size_t pos = path.find_last_of('\\');
                if (pos != std::string::npos) {
                    std::string root = path.substr(0, pos);
                    SetEnvironmentVariableA("fb2k", root.c_str());
                    SetEnvironmentVariableA("foobar2000", root.c_str());
                }
            }
        }

        CSeparator::Register();
        // Initialize COM automation server
        if (SUCCEEDED(OleInitialize(nullptr)))
            OpenHacksCOM::Initialise(mod);
        break;

    case DLL_PROCESS_DETACH:
        // Finalize COM automation server
        OpenHacksCOM::Finalise();
        OleUninitialize();
        break;
        
    default:
        break;
    }
    return TRUE;
}

namespace
{
DECLARE_COMPONENT_VERSION_COPY("OpenHacksMod", HACKS_VERSION, GetOpenHacksAbout())
// VALIDATE_COMPONENT_FILENAME("foo_openhacks_mod.dll")
} // namespace
