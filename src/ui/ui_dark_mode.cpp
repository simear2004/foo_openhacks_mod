#include "pch.h"
#include "ui_dark_mode.h"

bool CDarkModeHooks::IsDarkMode()
{
    auto api = ui_config_manager::tryGet();
    return api.is_valid() ? api->is_dark_mode() : false;
}
