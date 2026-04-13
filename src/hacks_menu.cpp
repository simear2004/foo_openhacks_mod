#include "pch.h"
#include "hacks_menu.h"

OpenHacksMenu& OpenHacksMenu::Get()
{
    static OpenHacksMenu menu;
    return menu;
}

HMENU OpenHacksMenu::GenerateMenu()
{
    if (HMENU menu = CreatePopupMenu())
    {
        GenerateMenuUsingManager(menu, mFileMenuPtr, mainmenu_groups::file, L"文件", CommandFileStart);
        GenerateMenuUsingManager(menu, mEditMenuPtr, mainmenu_groups::edit, L"编辑", CommandEditStart);
        GenerateMenuUsingManager(menu, mViewMenuPtr, mainmenu_groups::view, L"视图", CommandViewStart);
        GenerateMenuUsingManager(menu, mPlaybackMenuPtr, mainmenu_groups::playback, L"播放", CommandPlaybackStart);
        GenerateMenuUsingManager(menu, mLibraryMenuPtr, mainmenu_groups::library, L"媒体库", CommandLibraryStart);
        GenerateMenuUsingManager(menu, mHelpMenuPtr, mainmenu_groups::help, L"帮助", CommandHelpStart);

        return menu;
    }

    return nullptr;
}

void OpenHacksMenu::ExecuteMenuCommand(int32_t cmd)
{
    auto InBounds = [cmd](const int32_t start) { return (cmd >= start) && (cmd < (start + CommandRootMenuMaxCount)); };

    if (InBounds(CommandFileStart))
    {
        mFileMenuPtr->execute_command(cmd - CommandFileStart);
    }
    else if (InBounds(CommandViewStart))
    {
        mViewMenuPtr->execute_command(cmd - CommandViewStart);
    }
    else if (InBounds(CommandEditStart))
    {
        mEditMenuPtr->execute_command(cmd - CommandEditStart);
    }
    else if (InBounds(CommandPlaybackStart))
    {
        mPlaybackMenuPtr->execute_command(cmd - CommandPlaybackStart);
    }
    else if (InBounds(CommandLibraryStart))
    {
        mLibraryMenuPtr->execute_command(cmd - CommandLibraryStart);
    }
    else if (InBounds(CommandHelpStart))
    {
        mHelpMenuPtr->execute_command(cmd - CommandHelpStart);
    }
}

void OpenHacksMenu::GenerateMenuUsingManager(HMENU menu, mainmenu_manager::ptr& manager, const GUID& guid, wstring_view_t caption, uint32_t baseId)
{
    if (manager.is_empty())
    {
        if (manager = mainmenu_manager::get(); manager.is_valid())
        {
            manager->instantiate(guid);
        }
    }

    if (manager.is_empty())
        return;

    if (HMENU subMenu = CreatePopupMenu())
    {
        const t_uint32 flags = mainmenu_manager::flag_show_shortcuts;
        manager->generate_menu_win32(subMenu, baseId, CommandRootMenuMaxCount, flags);
        AppendMenuW(menu, MF_POPUP, (UINT_PTR)subMenu, caption.data());
    }
}
