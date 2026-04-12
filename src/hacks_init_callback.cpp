#include "pch.h"
#include "hacks_core.h"
#include "hacks_vars.h"

namespace
{

class open_hacks_init_stage_callback : public init_stage_callback
{
public:
    void on_init_stage(t_uint32 stage) override
    {
        if (stage == init_stages::after_config_read)
        {
            OpenHacksVars::InitialseOpenHacksVars();
        }
        else if (stage == init_stages::before_ui_init)
        {
            if (!OpenHacksCore::Get().CheckIncompatibleComponents())
                return;

            if (!OpenHacksCore::Get().InstallWindowHooks())
                return;
        }
    }
};

class open_hacks_initquit : public initquit
{
public:
    // on_init is called after the main window has been created.
    void on_init() override
    {
        OpenHacksCore::Get().Initialize();

        if (OpenHacksVars::AutoLoadFonts)
        {
            OpenHacksVars::LoadFontsAsync();
        }
    }
    // on_quit is called before the main window is destroyed.
    void on_quit() override
    {
        OpenHacksVars::UnloadCustomFonts();
        // safe to clean up
        OpenHacksCore::Get().Finalize();
    }
};

static initquit_factory_t<open_hacks_init_stage_callback> g_open_hacks_init_stage_callback;
static initquit_factory_t<open_hacks_initquit> g_open_hacks_initquit;
} // namespace
