#pragma once

#ifdef DEBUG
#include <libPPUI/DarkMode.h>
using CDarkModeHooksImpl = DarkMode::CHooks;
#else
#include <foobar2000/SDK/coreDarkMode.h>
using CDarkModeHooksImpl = fb2k::CCoreDarkModeHooks;
#endif

class CDarkModeHooks : public CDarkModeHooksImpl, private ui_config_callback_impl
{
public:
    CDarkModeHooks() : CDarkModeHooksImpl(IsDarkMode())
    {
    }

    static bool IsDarkMode();

private:
    void ui_fonts_changed() override
    {
    }

    void ui_colors_changed() override
    {
        this->SetDark(IsDarkMode());
    }
};