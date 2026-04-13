#pragma once

class OpenHacksMenu
{
public:
    static OpenHacksMenu& Get();

    HMENU GenerateMenu();
    void ExecuteMenuCommand(int32_t cmd);

private:
    enum MainMemuCommands
    {
        CommandRootMenuMaxCount = 1000,
        CommandFileStart = 1,
        CommandFileEnd = CommandFileStart + CommandRootMenuMaxCount - 1,
        CommandViewStart,
        CommandViewEnd = CommandViewStart + CommandRootMenuMaxCount - 1,
        CommandEditStart,
        CommandEditEnd = CommandEditStart + CommandRootMenuMaxCount - 1,
        CommandPlaybackStart,
        CommandPlayBackEnd = CommandPlaybackStart + CommandRootMenuMaxCount - 1,
        CommandLibraryStart,
        CommandLibraryEnd = CommandLibraryStart + CommandRootMenuMaxCount - 1,
        CommandHelpStart,
        CommandHelpEnd = CommandHelpStart + CommandRootMenuMaxCount - 1,
    };

    void GenerateMenuUsingManager(HMENU menu, mainmenu_manager::ptr& manager, const GUID& guid, wstring_view_t caption, uint32_t baseId);

private:
    mainmenu_manager::ptr mFileMenuPtr;
    mainmenu_manager::ptr mViewMenuPtr;
    mainmenu_manager::ptr mEditMenuPtr;
    mainmenu_manager::ptr mPlaybackMenuPtr;
    mainmenu_manager::ptr mLibraryMenuPtr;
    mainmenu_manager::ptr mHelpMenuPtr;
};